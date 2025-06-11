/*
 * adc.h
 *
 *  Created on: 12 cze 2025
 *      Author: lenovo
 */

#ifndef MAIN_ADC_H_
#define MAIN_ADC_H_

#ifndef ADC_H
#define ADC_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define ADC_SAMPLE_COUNT 64          // Number of samples for averaging
#define ADC_DEFAULT_VREF 1100        // Default reference voltage in mV
#define ADC_WEIGHT_CHANNEL ADC1_CHANNEL_0  // GPIO36 if ADC1, GPIO0 if ADC2

/**
 * @brief Initialize ADC for weight measurement
 * @param adc_channel ADC channel to use (ADC1_CHANNEL_x)
 * @param adc_width Bit width of ADC result (ADC_WIDTH_BIT_12 recommended)
 * @param adc_atten ADC attenuation (ADC_ATTEN_DB_11 for 0-3.1V range)
 */
void adc_weight_init(adc1_channel_t adc_channel, adc_bits_width_t adc_width, adc_atten_t adc_atten);

/**
 * @brief Start weight measurement task
 */
void adc_weight_start_task();

/**
 * @brief Get latest filtered weight value
 * @return Filtered weight in raw ADC units
 */
float adc_get_filtered_weight(void);

/**
 * @brief Convert raw ADC value to voltage in mV
 * @param adc_reading Raw ADC value
 * @return Voltage in millivolts
 */
float adc_raw_to_voltage(uint32_t adc_reading);

#endif // ADC_H
#endif /* MAIN_ADC_H_ */
