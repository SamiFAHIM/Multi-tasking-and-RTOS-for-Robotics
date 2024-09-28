#ifndef ULTRASOUND_LIB__
#define ULTRASOUND_LIB__
#include <tuple>
#include <time.h>
#include <functional>
#include "driver/gpio.h"
/**
 * @brief Handle definition: Ultrasound_t is a private type
 *
 */
struct Ultrasound_t;
typedef struct Ultrasound_t *Ultrasound_Handle_t;

/**
 * @brief Ultrasound successful measurement callback
 *
 */
typedef void (*Ultrasound_Callback_t)(Ultrasound_Handle_t handle, void *user_data);

/**
 * @brief Ultrasound init structure
 *
 */
typedef struct
{
    gpio_num_t gpio_trig_pin;         //< Trigger pin number
    gpio_num_t gpio_echo_pin;         //< Echo pin number
    Ultrasound_Callback_t onRead;     //< Callback function when read value got updated (called from ISR context)
    void *user_data;                  //< User data for callback argument
    uint32_t measurement_period_ms;   //< Period of measurement in ms (at least 60 ms)
    uint32_t trig_signal_duration_us; //< Trig signal duration (should be 10us, 50us should be fine)
} Ultrasound_Init_t;

typedef uint32_t Ultrasound_Error_t;

typedef struct
{
    int64_t timestamp_us;
    int32_t distance_mm;
} Ultrasound_Measurement_t;

class ultrasound
{
private:
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
public:
    ultrasound(const Ultrasound_Init_t &ultrasound_config);
    ~ultrasound();
    Ultrasound_Error_t start(void);
    Ultrasound_Error_t Ultrasound_SetPeriodMs(uint32_t ultrasound_period_ms);
    std::pair<time_t, int32_t> getDistance(void);
};

/**
 * @brief Create Ultrasound instance
 *
 * @param ultrasound_init Initialisation structure
 * @return Ultrasound_Handle_t NULL if initialization failed
 */
Ultrasound_Handle_t Ultrasound_Init(const Ultrasound_Init_t *ultrasound_init);

/**
 * @brief Start automatic measurement
 *
 * @param handle Ultrasound handle
 * @return Ultrasound_Error_t
 */
Ultrasound_Error_t Ultrasound_Start(Ultrasound_Handle_t handle);

/**
 * @brief Get last measured distance
 *
 * @param handle Ultrasound Handle
 * @return float
 */
Ultrasound_Measurement_t Ultrasound_GetDistance(const Ultrasound_Handle_t handle);

/**
 * @brief Change ultrasound sensor period measurement
 *
 * @param handle Ultrasound Handle
 * @param measurement_period_ms New Period
 * @return Ultrasound_Error_t
 */
Ultrasound_Error_t Ultrasound_SetPeriodMs(Ultrasound_Handle_t handle, uint32_t measurement_period_ms);

#endif /*ULTRASOUND_LIB__*/