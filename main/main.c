/*
 * main.c
 *
 *  Created on: 28 sie 2024
 *      Author: majorBien
 */


/**
 * Application entry point.
 */

#include "nvs_flash.h"
#include "eth.h"
#include "logic.h"
#include "tcp.h"
#include "adc.h"
#include "wifi_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "io.h"

QueueHandle_t input_queue;

void app_main(void)
{
		input_queue = xQueueCreate(10, sizeof(inputs_t));
    // Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

    
    // Start tasks
    adc_task_start();
    start_logic_task();
	//eth_app_task();
	start_tcp_client_task();    // start TCP
	wifi_app_start();
}

