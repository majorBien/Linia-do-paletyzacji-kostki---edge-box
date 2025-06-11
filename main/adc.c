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
#include "esp_log.h"
#include "tasks_common.h"

static const char *TAG = "ADC_WEIGHT";

static esp_adc_cal_characteristics_t *adc_chars = NULL;
static adc1_channel_t adc_channel;
static float filtered_weight = 0.0f;          // Changed to float
static SemaphoreHandle_t adc_mutex = NULL;
static float scale_factor = 1.0f;             // Raw to weight conversion factor
static float zero_offset = 0.0f;              // Calibration offset

/**
 * @brief ADC measurement task for continuous weight monitoring
 * @param pvParameters Task parameters (unused)
 * 
 * Performs:
 * - Multi-sample acquisition
 * - Median filtering
 * - Low-pass filtering
 * - Thread-safe value updates
 */
static void adc_measurement_task(void *pvParameters)
{
    uint32_t raw_samples[ADC_SAMPLE_COUNT];
    
    while (1) {
        // Acquire multiple samples to reduce noise
        for (int i = 0; i < ADC_SAMPLE_COUNT; i++) {
            raw_samples[i] = adc1_get_raw(adc_channel);
            vTaskDelay(pdMS_TO_TICKS(1)); // Minimize ADC switching artifacts
        }
        
        // Sort samples for median calculation (bubble sort for small arrays)
        for (int i = 0; i < ADC_SAMPLE_COUNT - 1; i++) {
            for (int j = i + 1; j < ADC_SAMPLE_COUNT; j++) {
                if (raw_samples[j] < raw_samples[i]) {
                    uint32_t temp = raw_samples[i];
                    raw_samples[i] = raw_samples[j];
                    raw_samples[j] = temp;
                }
            }
        }
        
        // Calculate median (middle sample)
        float median = (float)raw_samples[ADC_SAMPLE_COUNT / 2];
        
        // Apply exponential low-pass filter (α=0.1)
        xSemaphoreTake(adc_mutex, portMAX_DELAY);
        filtered_weight = (filtered_weight * 0.9f) + (median * 0.1f);
        xSemaphoreGive(adc_mutex);
        
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz update rate
    }
}

/**
 * @brief Initialize ADC for weight measurement
 * @param channel ADC1 channel to use
 * @param width ADC bit width
 * @param atten ADC attenuation level
 */
void adc_weight_init(adc1_channel_t channel, adc_bits_width_t width, adc_atten_t atten)
{
    adc_channel = channel;
    
    // Configure ADC hardware
    ESP_ERROR_CHECK(adc1_config_width(width));
    ESP_ERROR_CHECK(adc1_config_channel_atten(channel, atten));
    
    // Characterize ADC for voltage conversion
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
        ADC_UNIT_1, 
        atten, 
        width, 
        ADC_DEFAULT_VREF, 
        adc_chars
    );
    
    // Log calibration method used
    switch (val_type) {
        case ESP_ADC_CAL_VAL_EFUSE_VREF:
            ESP_LOGI(TAG, "Using eFuse Vref: %umV", adc_chars->vref);
            break;
        case ESP_ADC_CAL_VAL_EFUSE_TP:
            ESP_LOGI(TAG, "Using Two Point coeffs: a=%umV b=%umV", 
                   adc_chars->coeff_a, adc_chars->coeff_b);
            break;
        default:
            ESP_LOGI(TAG, "Using default Vref: %umV", ADC_DEFAULT_VREF);
    }
    
    // Create mutex for thread-safe operations
    adc_mutex = xSemaphoreCreateMutex();
    assert(adc_mutex != NULL);
}

/**
 * @brief Start ADC measurement task on specified core
 */
void adc_weight_start_task(void)
{
    xTaskCreatePinnedToCore(
        adc_measurement_task,    // Task function
        "adc_weight_task",       // Task name
        ADC_TASK_STACK_SIZE,     // Stack size
        NULL,                   // Parameters
        ADC_TASK_PRIORITY,      // Priority
        NULL,                   // Task handle
        ADC_TASK_CORE_ID        // Core affinity
    );
}

/**
 * @brief Get current filtered weight value
 * @return Weight in calibrated units (float)
 */
float adc_get_filtered_weight(void)
{
    float weight;
    xSemaphoreTake(adc_mutex, portMAX_DELAY);
    weight = (filtered_weight * scale_factor) + zero_offset; // Apply calibration
    xSemaphoreGive(adc_mutex);
    return weight;
}

/**
 * @brief Convert raw ADC reading to voltage
 * @param adc_reading Raw ADC value
 * @return Voltage in millivolts (float)
 */
float adc_raw_to_voltage(uint32_t adc_reading)
{
    return (float)esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
}

/**
 * @brief Set calibration parameters
 * @param scale Multiplicative scale factor (raw to weight)
 * @param offset Additive zero offset
 */
void adc_set_calibration(float scale, float offset)
{
    xSemaphoreTake(adc_mutex, portMAX_DELAY);
    scale_factor = scale;
    zero_offset = offset;
    xSemaphoreGive(adc_mutex);
}