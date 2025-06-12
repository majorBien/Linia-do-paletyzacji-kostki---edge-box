/*
 * logic.c
 *
 *  Created on: 11 Jun 2025
 *      Author: majorBien
 */


/* logic.c */

#include "logic.h"
#include "io.h"
//#include "tcp_devices.h"  // Interface to the robot and inverter
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "tcp.h"
#include "tasks_common.h"
#include "adc.h"


QueueHandle_t tcp_command_queue;


#define TAG "logic"

extern QueueHandle_t input_queue;

static system_state_t current_state = STATE_IDLE;
static uint8_t layer_count = 0;
static uint8_t max_layers = 5;  // Default value, can be changed via HMI
extern QueueHandle_t hmi_data_queue;


void logic_task(void *pvParameters) {
    inputs_t inputs;
    float weight = 0;
    tcp_command_queue = xQueueCreate(10, sizeof(tcp_command_t));
    configASSERT(tcp_command_queue != NULL);
    
    while (1) {
        if (xQueueReceive(input_queue, &inputs, portMAX_DELAY)) {
            switch (current_state) {
                case STATE_IDLE:
                    if (inputs.sensor1) {
                        ESP_LOGI(TAG, "Cube on T1, start");
                        current_state = STATE_MEASURING;
                    }
                    break;

                case STATE_MEASURING:
                    weight = read_weight();
                    ESP_LOGI(TAG, "Cube weight: %.2f kg", weight);

                    if (weight < 49.0f || weight > 51.0f) {
                        ESP_LOGW(TAG, "Incorrect weight, ejecting");
                        current_state = STATE_EJECT_REJECTED;
                    } else {
                        current_state = STATE_READY_FOR_ROBOT;
                    }
                    break;

                case STATE_READY_FOR_ROBOT:
                    if (inputs.sensor3) {
                        ESP_LOGI(TAG, "Cube ready for pickup by robot");
                        tcp_command_t cmd_r = { .type = CMD_ROBOT_PLACE };
                        xQueueSend(tcp_command_queue, &cmd_r, 0);
                        current_state = STATE_WAIT_FOR_LAYER;
                    }
                    break;

                case STATE_WAIT_FOR_LAYER:
                    layer_count++;
                    ESP_LOGI(TAG, "Layers placed: %d / %d", layer_count, max_layers);

                    if (layer_count >= max_layers) {
                        tcp_command_t cmd_i = { .type = CMD_INVERTER_START };
                        xQueueSend(tcp_command_queue, &cmd_i, 0);
                        current_state = STATE_WAIT_WRAP_DONE;
                    } else {
                        current_state = STATE_IDLE;
                    }
                    break;

                case STATE_WRAPPING:
                    if (inputs.wrap_done) {
                        ESP_LOGI(TAG, "Wrapping completed");
                        gpio_set_level(IO_OUTPUT_LED_GREEN, 1);
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        gpio_set_level(IO_OUTPUT_LED_GREEN, 0);
                        layer_count = 0;
                        current_state = STATE_IDLE;
                    }
                    break;

                case STATE_EJECT_REJECTED:
                    gpio_set_level(IO_OUTPUT_EJECTOR, 1);
                    vTaskDelay(pdMS_TO_TICKS(700));
                    gpio_set_level(IO_OUTPUT_EJECTOR, 0);
                    gpio_set_level(IO_OUTPUT_LED_RED, 1);
                    vTaskDelay(pdMS_TO_TICKS(500));
                    gpio_set_level(IO_OUTPUT_LED_RED, 0);
                    ESP_LOGI(TAG, "Rejected cube ejected");
                    current_state = STATE_IDLE;
                    break;

                case STATE_WAIT_WRAP_DONE:
                    gpio_set_level(IO_OUTPUT_WRAPPER, 1);
                    ESP_LOGI(TAG, "Wrapper started");

                    if (inputs.wrap_done) {
                        gpio_set_level(IO_OUTPUT_WRAPPER, 0);
                        ESP_LOGI(TAG, "Wrap sensor triggered");
                        current_state = STATE_WRAPPING;
                    } else {
                        // wait and recheck in next loop
                        vTaskDelay(pdMS_TO_TICKS(200));
                    }
                    break;

			case STATE_HMI_DATA_EXCHANGE: {

			    current_state = STATE_IDLE;
			    break;
			}

            }

        }
    }
}

void start_logic_task(void)
{
     xTaskCreatePinnedToCore(logic_task,
                            "logic",
                            LOGIC_TASK_STACK_SIZE,
                            NULL,
                            LOGIC_TASK_PRIORITY,
                            NULL,
                            LOGIC_TASK_CORE_ID);
}
