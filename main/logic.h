/*
 * logic.h
 *
 *  Created on: 11 Jun 2025
 *      Author: majorBien
 */

#ifndef MAIN_LOGIC_H_
#define MAIN_LOGIC_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Maximum number of layers on the pallet (set from the control panel)
typedef enum {
    LAYERS_5 = 5,
    LAYERS_8 = 8,
    LAYERS_10 = 10
} pallet_layer_count_t;

// Possible states of the machine
typedef enum {
    STATE_IDLE = 0,
    STATE_MEASURING,
    STATE_EJECT_REJECTED,
    STATE_READY_FOR_ROBOT,
    STATE_WAIT_FOR_LAYER,
    STATE_WRAPPING,
    STATE_WAIT_WRAP_DONE
} system_state_t;


typedef enum {
    CMD_NONE = 0,
    CMD_ROBOT_PLACE,
    CMD_INVERTER_START
} tcp_command_type_t;

typedef struct {
    tcp_command_type_t type;
    float payload;  
} tcp_command_t;

extern QueueHandle_t tcp_command_queue;

// Initialization and start of the logic task
void logic_task(void *pvParameters);

// Set the number of layers on the pallet (e.g., from the operator panel)
void logic_set_max_layers(pallet_layer_count_t layers);

//start ogic task
void start_logic_task(void);
// Queue for passing data from io_task
extern QueueHandle_t logic_input_queue;

#endif /* MAIN_LOGIC_H_ */