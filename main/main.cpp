#include "freertos/FreeRTOS.h"
#include <stdio.h>
extern "C" void app_main();

void app_main()
{
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}