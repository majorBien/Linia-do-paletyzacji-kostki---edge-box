/*
 * hmi.c
 *
 *  Created on: 12 cze 2025
 *      Author: majorBien
 */

/* hmi.c */

#include "hmi.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "string.h"

#define TAG "hmi"

QueueHandle_t hmi_data_queue;

void hmi_uart_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_param_config(UART_HMI_NUM, &uart_config);
    uart_set_pin(UART_HMI_NUM, UART_HMI_TXD, UART_HMI_RXD, UART_HMI_RTS, UART_HMI_CTS);
    uart_driver_install(UART_HMI_NUM, UART_HMI_BUF_SIZE * 2, 0, 0, NULL, 0);

    hmi_data_queue = xQueueCreate(5, sizeof(hmi_data_t));
    configASSERT(hmi_data_queue);

    ESP_LOGI(TAG, "UART HMI (RS485) initialized");
}

void hmi_task(void *pvParameters) {
    uint8_t rx_buffer[UART_HMI_BUF_SIZE];
    hmi_data_t hmi_data;

    while (1) {
        // Read from UART
        int len = uart_read_bytes(UART_HMI_NUM, rx_buffer, sizeof(rx_buffer), pdMS_TO_TICKS(100));
        if (len >= 1) {
            // Parse received HMI data (simple protocol: [0]=max_layers)
            hmi_data.max_layers = rx_buffer[0];
            xQueueSend(hmi_data_queue, &hmi_data, 0);
            ESP_LOGI(TAG, "Received max_layers from HMI: %d", hmi_data.max_layers);
        }

        // Send feedback to HMI (float + 4 bytes)
        if (xQueueReceive(hmi_data_queue, &hmi_data, 0)) {
            uint8_t tx_buf[8];
            memcpy(&tx_buf[0], &hmi_data.last_weight, sizeof(float)); // 4B float
            tx_buf[4] = hmi_data.sensor1;
            tx_buf[5] = hmi_data.sensor2;
            tx_buf[6] = hmi_data.sensor3;
            tx_buf[7] = hmi_data.wrap_done;
            uart_write_bytes(UART_HMI_NUM, (const char *)tx_buf, sizeof(tx_buf));
            ESP_LOGI(TAG, "Sent data to HMI: weight=%.2f, s1=%d, s2=%d, s3=%d, wrap=%d",
                     hmi_data.last_weight, hmi_data.sensor1, hmi_data.sensor2, hmi_data.sensor3, hmi_data.wrap_done);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


