/*
 * io.h
 *
 *  Created on: 11 cze 2025
 *      Author: lenovo
 */

/* io.h */
#ifndef MAIN_IO_H_
#define MAIN_IO_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
extern QueueHandle_t input_queue;
// GPIO pin definitions
#define IO_INPUT_SENSOR_1     2   // Czujnik transporter 1
#define IO_INPUT_SENSOR_2     3   // Czujnik transporter 2
#define IO_INPUT_SENSOR_3     4   // Czujnik transporter 3
#define IO_INPUT_WRAP_DONE    5   // Owijarka koniec

#define IO_OUTPUT_EJECTOR     12  // Siłownik zrzutu
#define IO_OUTPUT_WRAPPER     13  // Start owijarki
#define IO_OUTPUT_LED_GREEN   14  // Wieża zielona
#define IO_OUTPUT_LED_RED     15  // Wieża czerwona
#define IO_OUTPUT_LED_YELLOW  16  // Wieża żółta


typedef struct inputs{
	bool sensor1;
	bool sensor2;
	bool sensor3;
	bool wrap_done;
	
}inputs_t;

void io_init(void);
void io_task(void *pvParameters);

#endif /* MAIN_IO_H_ */




