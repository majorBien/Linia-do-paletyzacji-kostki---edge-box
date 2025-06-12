/*
 * adc.h
 *
 *  Created on: 12 cze 2025
 *      Author: lenovo
 */
#ifndef ADC_H
#define ADC_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"

// ADC task configuration macros
#define ADC_TASK_STACK_SIZE     4096
#define ADC_TASK_PRIORITY       10
#define ADC_TASK_CORE_ID        0

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start the ADC reading task.
 */
void adc_task_start(void);

/**
 * @brief Get the latest ADC weight reading.
 * 
 * @return float Latest weight value.
 */
float read_weight(void);

#ifdef __cplusplus
}
#endif

#endif // ADC_H
