// tcp_client.h

#ifndef MAIN_TCP_CLIENT_H_
#define MAIN_TCP_CLIENT_H_

#define TCP_CLIENT_TASK_STACK_SIZE 4096
#define TCP_CLIENT_TASK_PRIORITY   5
#define TCP_CLIENT_TASK_CORE_ID    tskNO_AFFINITY
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "logic.h"

void start_tcp_client_task(void);
extern QueueHandle_t tcp_command_queue;

#endif /* MAIN_TCP_CLIENT_H_ */
