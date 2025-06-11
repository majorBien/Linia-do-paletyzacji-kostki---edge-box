/*
 * io.c
 *
 *  Created on: 11 cze 2025
 *      Author: lenovo
 */


/* io.c */

#include "io.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "io"

void io_init(void)
{
    // Configure input pins
    gpio_config_t input_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << IO_INPUT_SENSOR_1) |
                        (1ULL << IO_INPUT_SENSOR_2) |
                        (1ULL << IO_INPUT_SENSOR_3) |
                        (1ULL << IO_INPUT_WRAP_DONE),
        .pull_down_en = 0,
        .pull_up_en = 1
    };
    gpio_config(&input_conf);

    // Configure output pins
    gpio_config_t output_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << IO_OUTPUT_EJECTOR) |
                        (1ULL << IO_OUTPUT_WRAPPER) |
                        (1ULL << IO_OUTPUT_LED_GREEN) |
                        (1ULL << IO_OUTPUT_LED_RED) |
                        (1ULL << IO_OUTPUT_LED_YELLOW),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&output_conf);

    ESP_LOGI(TAG, "GPIO initialized");
}

void io_task(void *pvParameters)
{
    while (1) {
        int sensor1 = gpio_get_level(IO_INPUT_SENSOR_1);
        int sensor2 = gpio_get_level(IO_INPUT_SENSOR_2);
        int sensor3 = gpio_get_level(IO_INPUT_SENSOR_3);
        int wrap_done = gpio_get_level(IO_INPUT_WRAP_DONE);

        // Debug log
        ESP_LOGI(TAG, "Sensors: T1=%d T2=%d T3=%d WrapDone=%d",
                 sensor1, sensor2, sensor3, wrap_done);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
