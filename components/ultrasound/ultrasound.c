#include "ultrasound.h"
#include "freertos/FreeRTOS.h"
#include "semaphore.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <string.h>

#define VELOCITY_SOUND_MM_PER_MS 343u //< 343 mm per milliseconds (sound velocity)

/**
 * @brief Ultrasound sensor state machine
 *
 */
typedef enum
{
    ULTRASOUND_STATE_MIN = 0,
    ULTRASOUND_STATE_INIT = ULTRASOUND_STATE_MIN, // Init state
    ULTRASOUND_STATE_WAIT_TRIG_START,             // Idle state
    ULTRASOUND_STATE_WAIT_TRIG_END,               // Waiting for timer to terminate trigger period
    ULTRASOUND_STATE_WAIT_ECHO_START,             // Waiting for echo back (raising edge)
    ULTRASOUND_STATE_WAIT_ECHO_END,               // Waiting for echo back (falling edge)
    ULTRASOUND_STATE_MAX,
} Ultrasound_StateMachine_t;

/**
 * @brief Private UltraSound structure
 *
 */
typedef struct Ultrasound_t
{
    gpio_num_t gpio_trig_pin;                 //< GPIO Trigger pin number
    gpio_num_t gpio_echo_pin;                 //< GPIO Echo pin number
    esp_timer_handle_t timer;                 // Timer handle
    uint64_t measurement_period_us;           //< Measurement period us
    uint64_t trig_signal_duration_us;         //< Trigger signal duration
    int64_t time_trig_start;                  //< Time of trig rising edge
    int64_t time_trig_end;                    //< Time of trig falling edge
    int64_t time_echo_start;                  //< Time of echo rising edge
    int64_t time_echo_end;                    //< Time of echo falling edge
    uint8_t error_count;                      //< Measure failed counter
    Ultrasound_Measurement_t last_measure;    //< Last updated measurement
    volatile Ultrasound_StateMachine_t state; // Current state machine state
    Ultrasound_Callback_t callback;           //< Onread callback function (called from ISR)
    void *user_data;                          //< User context
} Ultrasound_struct_t;

static const char *LOG_TAG = "ULTRA";
static void ultrasound_gpio_isr_echo(void *args);
static void ultrasound_periodic_job(void *args);

Ultrasound_Handle_t Ultrasound_Init(const Ultrasound_Init_t *ultrasound_init)
{
    esp_err_t err = ESP_OK;
    Ultrasound_Handle_t handle = (Ultrasound_Handle_t)heap_caps_calloc(1, sizeof(Ultrasound_struct_t), MALLOC_CAP_DEFAULT);
    if (NULL == handle)
    {
        ESP_LOGE(LOG_TAG, "Failed to init Ultrasound");
        return NULL;
    }
    memset(handle, 0, sizeof(Ultrasound_struct_t)); // Sanity
    handle->callback = ultrasound_init->onRead;
    handle->user_data = ultrasound_init->user_data;
    // GPIO initialisation
    handle->gpio_echo_pin = ultrasound_init->gpio_echo_pin;
    handle->gpio_trig_pin = ultrasound_init->gpio_trig_pin;
    gpio_config_t gpio_echo = (gpio_config_t){
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE, // Disabled during initialization
        .pin_bit_mask = 1ULL << (handle->gpio_echo_pin),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };

    err = gpio_config(&gpio_echo);
    if (ESP_OK != err)
    {
        ESP_LOGE(LOG_TAG, "Failed to init Echo GPIO (err =%u)", err);
        vPortFree(handle);
        return NULL;
    }
    gpio_config_t gpio_trig = (gpio_config_t){
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = 1ULL << (handle->gpio_trig_pin),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    err = gpio_config(&gpio_trig);
    if (ESP_OK != err)
    {
        ESP_LOGE(LOG_TAG, "Failed to init Trig GPIO (err =%u)", err);
        vPortFree(handle);
        return NULL;
    }

    // Interruption initialisation
    gpio_intr_disable(handle->gpio_echo_pin); // Disable interrupt for now
    err = gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    if (ESP_ERR_INVALID_STATE == err)
    {
        ESP_LOGW(LOG_TAG, "GPIO ISR service already installed");
    }
    else if (ESP_OK != err)
    {
        ESP_LOGE(LOG_TAG, "Failed to init GPIO ISR service (err =%u)", err);
        vPortFree(handle);
        return NULL;
    }
    err = gpio_isr_handler_add(ultrasound_init->gpio_echo_pin, ultrasound_gpio_isr_echo, handle);
    if (ESP_OK != err)
    {
        ESP_LOGE(LOG_TAG, "Failed to add callback to GPIO ISR service (err =%u)", err);
        // TODO add interrupt uninstall
        vPortFree(handle);
        return NULL;
    }
    // Timer initialisation
    handle->measurement_period_us = ultrasound_init->measurement_period_ms * 1000;
    handle->trig_signal_duration_us = ultrasound_init->trig_signal_duration_us;
    err = esp_timer_init();
    if (ESP_ERR_INVALID_STATE == err)
    {
        ESP_LOGW(LOG_TAG, "ESP Timer service already installed");
    }
    else if (ESP_OK != err)
    {
        ESP_LOGE(LOG_TAG, "Failed to init ESP timer service (err =%u)", err);
        // TODO add interruption uninstall
        vPortFree(handle);
        return NULL;
    }
    esp_timer_create_args_t timer_create = {
        .name = "UltraSound",
        .skip_unhandled_events = true,
        .dispatch_method = ESP_TIMER_TASK,
        .callback = ultrasound_periodic_job,
        .arg = handle,
    };
    err = esp_timer_create(&timer_create, &handle->timer);
    if (ESP_OK != err)
    {
        ESP_LOGE(LOG_TAG, "Failed to create timer (err =%u)", err);
        // TODO uninstall stuff
        vPortFree(handle);
        return NULL;
    }
    handle->last_measure.timestamp_us = esp_timer_get_time();
    handle->last_measure.distance_mm = INT32_MAX;
    return handle;
}

Ultrasound_Error_t Ultrasound_Start(Ultrasound_Handle_t handle)
{
    handle->state = ULTRASOUND_STATE_WAIT_TRIG_START;
    esp_timer_start_once(handle->timer, 50); // Immediate start
    return ESP_OK;
}

Ultrasound_Measurement_t Ultrasound_GetDistance(const Ultrasound_Handle_t handle)
{
    return handle->last_measure;
}

static void ultrasound_periodic_job(void *args)
{
    Ultrasound_Handle_t handle = (Ultrasound_Handle_t)args;
    if (NULL == args)
    {
        return;
    }
    switch (handle->state)
    {
    case ULTRASOUND_STATE_WAIT_TRIG_START:
        gpio_intr_disable(handle->gpio_echo_pin);
        gpio_set_level(handle->gpio_trig_pin, 1);
        handle->time_trig_start = esp_timer_get_time();
        esp_timer_start_once(handle->timer, handle->trig_signal_duration_us);
        handle->state = ULTRASOUND_STATE_WAIT_TRIG_END;
        break;
    case ULTRASOUND_STATE_WAIT_TRIG_END:
        gpio_set_level(handle->gpio_trig_pin, 0);
        esp_timer_start_once(handle->timer, handle->measurement_period_us - handle->trig_signal_duration_us);
        handle->state = ULTRASOUND_STATE_WAIT_ECHO_START;
        gpio_set_intr_type(handle->gpio_echo_pin, GPIO_INTR_POSEDGE);
        gpio_intr_enable(handle->gpio_echo_pin);
        break;
    case ULTRASOUND_STATE_WAIT_ECHO_START:
        handle->state = ULTRASOUND_STATE_WAIT_TRIG_START;
        ++handle->error_count;
        ultrasound_periodic_job(args); // Call itself to restart measurement
        break;
    case ULTRASOUND_STATE_WAIT_ECHO_END:
        handle->state = ULTRASOUND_STATE_WAIT_TRIG_START;
        ++handle->error_count;
        ultrasound_periodic_job(args); // Call itself to restart measurement
        break;
    default:
        break;
    }
}

static void ultrasound_gpio_isr_echo(void *args)
{
    Ultrasound_Handle_t handle = (Ultrasound_Handle_t)args;
    if (NULL == args)
    {
        return;
    }
    switch (handle->state)
    {
    case ULTRASOUND_STATE_WAIT_ECHO_START:
        handle->time_echo_start = esp_timer_get_time();
        handle->state = ULTRASOUND_STATE_WAIT_ECHO_END;
        gpio_set_intr_type(handle->gpio_echo_pin, GPIO_INTR_NEGEDGE);
        break;
    case ULTRASOUND_STATE_WAIT_ECHO_END:
        handle->time_echo_end = esp_timer_get_time();
        // Compute distance
        int64_t duration = (handle->time_echo_end - handle->time_echo_start);
        handle->last_measure.timestamp_us = handle->time_trig_start;
        handle->last_measure.distance_mm = (duration * VELOCITY_SOUND_MM_PER_MS) / (1000 * 2);
        gpio_intr_disable(handle->gpio_echo_pin);
        if (NULL != handle->callback)
        {
            handle->callback(handle, handle->user_data);
        }
        handle->state = ULTRASOUND_STATE_WAIT_TRIG_START;
    default:
        break;
    }
}