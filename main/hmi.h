/*
 * hmi.h
 *
 *  Created on: 12 cze 2025
 *      Author: majorBien
 */



/* hmi.h */

#ifndef HMI_H_
#define HMI_H_

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/queue.h"

#define UART_HMI_NUM UART_NUM_1
#define UART_HMI_TXD  17
#define UART_HMI_RXD  16
#define UART_HMI_RTS  (UART_PIN_NO_CHANGE)
#define UART_HMI_CTS  (UART_PIN_NO_CHANGE)
#define UART_HMI_BUF_SIZE (1024)

typedef struct {
    uint8_t max_layers;     // from HMI to ESP32
    float last_weight;      // from ESP32 to HMI
    uint8_t sensor1;        // from ESP32 to HMI
    uint8_t sensor2;        // from ESP32 to HMI
    uint8_t sensor3;        // from ESP32 to HMI
    uint8_t wrap_done;      // from ESP32 to HMI
} hmi_data_t;

extern QueueHandle_t hmi_data_queue;

void hmi_uart_init(void);
void hmi_task(void *pvParameters);

#endif /* HMI_H_ */


