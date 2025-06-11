/*
 * modbus_tcp_master.c
 *
 *  Created on: 11 cze 2025
 *      Author: lenovo
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
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            close(sock);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "Successfully connected");

        while (1) {
            int err = send(sock, payload, strlen(payload), 0);
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            } else {
                ESP_LOGI(TAG, "Message sent");
            }

            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0) {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                break;
            } else {
                rx_buffer[len] = 0;
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
            }

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }

    vTaskDelete(NULL);
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
