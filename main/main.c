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

void app_main(void)
{
    // Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

    // Initialize ADC on channel 0 (GPIO36), 12-bit width, 11dB attenuation
   // adc_weight_init(ADC1_CHANNEL_0, ADC_WIDTH_BIT_12, ADC_ATTEN_DB_11);
    
    // Start measurement task with priority 5 and 4096 bytes stack
    //adc_weight_start_task();
    start_logic_task();
	eth_app_task();

	start_tcp_client_task();    // uruchamia TCP

}

