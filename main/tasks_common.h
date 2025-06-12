/*
 * tasks_common.h
 *
 *  Created on: Sep 27, 2024
 *      Author: majorBien
 */

#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_



#define ETH_APP_TASK_STACK_SIZE				4096
#define ETH_APP_TASK_PRIORITY				6
#define ETH_APP_TASK_CORE_ID				0

#define TCP_CLIENT_TASK_STACK_SIZE			4096
#define TCP_CLIENT_TASK_PRIORITY			6
#define TCP_CLIENT_TASK_CORE_ID				0

#define LOGIC_TASK_STACK_SIZE				4096
#define LOGIC_TASK_PRIORITY					8
#define LOGIC_TASK_CORE_ID					0

#define ADC_TASK_STACK_SIZE					4096
#define ADC_TASK_PRIORITY					10
#define ADC_TASK_CORE_ID					0

// WiFi application task
#define WIFI_APP_TASK_STACK_SIZE			4096
#define WIFI_APP_TASK_PRIORITY				5
#define WIFI_APP_TASK_CORE_ID				1

// HTTP Server task
#define HTTP_SERVER_TASK_STACK_SIZE			8192
#define HTTP_SERVER_TASK_PRIORITY			4
#define HTTP_SERVER_TASK_CORE_ID			1

// HTTP Server Monitor task
#define HTTP_SERVER_MONITOR_STACK_SIZE		4096
#define HTTP_SERVER_MONITOR_PRIORITY		3
#define HTTP_SERVER_MONITOR_CORE_ID			1

#endif /* MAIN_TASKS_COMMON_H_ */

