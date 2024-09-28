#ifndef ULTRASOUND_LIB__
#define ULTRASOUND_LIB__
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif
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
typedef void (*Ultrasound_Callback_t)(Ultrasound_Handle_t handle,
                                      void *user_data);

/**
 * @brief Ultrasound init structure
 *
 */
typedef struct {
  gpio_num_t gpio_trig_pin;     //< Trigger pin number
  gpio_num_t gpio_echo_pin;     //< Echo pin number
  Ultrasound_Callback_t onRead; //< Callback function when read value got
                                //updated (called from ISR context)
  void *user_data;              //< User data for callback argument
  uint32_t
      measurement_period_ms; //< Period of measurement in ms (at least 60 ms)
  uint32_t trig_signal_duration_us; //< Trig signal duration (should be 10us,
                                    //50us should be fine)
} Ultrasound_Init_t;

typedef uint32_t Ultrasound_Error_t;

typedef struct {
  int64_t timestamp_us;
  int32_t distance_mm;
} Ultrasound_Measurement_t;

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
Ultrasound_Measurement_t
Ultrasound_GetDistance(const Ultrasound_Handle_t handle);

/**
 * @brief Change ultrasound sensor period measurement
 *
 * @param handle Ultrasound Handle
 * @param measurement_period_ms New Period
 * @return Ultrasound_Error_t
 */
Ultrasound_Error_t Ultrasound_SetPeriodMs(Ultrasound_Handle_t handle,
                                          uint32_t measurement_period_ms);
#ifdef __cplusplus
}
#endif
#endif /*ULTRASOUND_LIB__*/