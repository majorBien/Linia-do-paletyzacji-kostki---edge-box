/*
 * modbus_tcp_master.c
 *
 *  Created on: 11 cze 2025
 *      Author: majorBien
 */
// tcp_client.c

#include "tcp.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include <string.h>
#include <errno.h>

#define TAG "tcp_client"
#define HOST_IP "192.168.0.10"
#define PORT 2000
static const char *payload = "Message from ESP32 ";

static void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP;
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;

    struct sockaddr_in dest_addr = {
        .sin_addr.s_addr = inet_addr(host_ip),
        .sin_family = AF_INET,
        .sin_port = htons(PORT)
    };

    while (1) {
	int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    tcp_command_t cmd;
    if (xQueueReceive(tcp_command_queue, &cmd, pdMS_TO_TICKS(1000))) {
        char command_buffer[64] = {0};

        switch (cmd.type) {
            case CMD_ROBOT_PLACE:
                snprintf(command_buffer, sizeof(command_buffer), "ROBOT:PLACE\n");
                break;

            case CMD_INVERTER_START:
                snprintf(command_buffer, sizeof(command_buffer), "INVERTER:START\n");
                break;

            default:
                continue;
        }

        int err = send(sock, command_buffer, strlen(command_buffer), 0);
        if (err < 0) {
            ESP_LOGE(TAG, "Send failed: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Command sent: %s", command_buffer);
    }

}

}

void start_tcp_client_task(void)
{
    xTaskCreatePinnedToCore(tcp_client_task,
                            "tcp_client",
                            TCP_CLIENT_TASK_STACK_SIZE,
                            NULL,
                            TCP_CLIENT_TASK_PRIORITY,
                            NULL,
                            TCP_CLIENT_TASK_CORE_ID);
}
