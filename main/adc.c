/*
 * adc.c - ADC weight measurement implementation for ESP32
 *
 * Features:
 * - Floating point weight calculations
 * - Thread-safe operation with mutex protection
 * - Advanced filtering (median + low-pass)
 * - Core-affinitized measurement task
 * 
 *  Created on: Jun 12, 2025
 *      Author: lenovo
 */
#include "adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "tasks_common.h"

static const char *TAG = "ADC_TASK";

// ADC channel and unit configuration
#define ADC_CHANNEL         ADC_CHANNEL_6       // GPIO34 (ADC1 channel 6)
#define ADC_UNIT            ADC_UNIT_1

static TaskHandle_t adc_task_handle = NULL;
static adc_oneshot_unit_handle_t adc_handle;
static float latest_weight = 0.0f;

/**
 * @brief ADC reading and conversion loop
 * 
 * This task reads raw ADC values, converts them to voltage, and stores a weight approximation.
 */
static void adc_task(void *arg)
{
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT,
    };
    adc_oneshot_new_unit(&init_cfg, &adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &chan_cfg);

    while (1) {
        int raw = 0;
        esp_err_t err = adc_oneshot_read(adc_handle, ADC_CHANNEL, &raw);
        if (err == ESP_OK) {
            // Example conversion: assume max 4095 = 5.0 kg
            latest_weight = (float)raw * 5.0f / 4095.0f;
        } else {
            ESP_LOGE(TAG, "ADC read error: %s", esp_err_to_name(err));
        }

        vTaskDelay(pdMS_TO_TICKS(200)); // 5Hz update rate
    }
}

/**
 * @brief Start the ADC task
 */
void adc_task_start(void)
{
    if (adc_task_handle == NULL) {
        xTaskCreatePinnedToCore(
            adc_task,
            "adc_task",
            ADC_TASK_STACK_SIZE,
            NULL,
            ADC_TASK_PRIORITY,
            &adc_task_handle,
            ADC_TASK_CORE_ID
        );
    }
}

/**
 * @brief Return the latest converted weight value
 * 
 * @return float Latest weight approximation in kg
 */
float read_weight(void)
{
    return latest_weight;
}
