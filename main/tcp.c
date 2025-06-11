/*
 * tcp.c - TCP client for communicating with Delta inverter and robot
 *
 * Features:
 * - Dedicated task for TCP communications
 * - Device-specific protocol implementations
 * - Automatic reconnection handling
 * - Command queuing system
 *
 *  Created on: 11 Jun 2025
 *  Author: majorBien
 */

#include "tcp.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <string.h>
#include <errno.h>

#define TAG "TCP_CLIENT"

// Device connection parameters
#define ROBOT_IP "192.168.1.100"   // Robot controller IP
#define ROBOT_PORT 5020             // Robot controller port
#define INVERTER_IP "192.168.1.101" // Delta inverter IP
#define INVERTER_PORT 502           // Modbus TCP port (standard)

// Delta inverter Modbus commands
#define DELTA_INVERTER_ADDRESS 0x01
#define DELTA_START_COMMAND 0x06    // Write single register
#define DELTA_START_REGISTER 0x2000 // Example start control register

static void tcp_client_task(void *pvParameters) {
    struct sockaddr_in robot_addr = {
        .sin_addr.s_addr = inet_addr(ROBOT_IP),
        .sin_family = AF_INET,
        .sin_port = htons(ROBOT_PORT)
    };
    
    struct sockaddr_in inverter_addr = {
        .sin_addr.s_addr = inet_addr(INVERTER_IP),
        .sin_family = AF_INET,
        .sin_port = htons(INVERTER_PORT)
    };

    while (1) {
        tcp_command_t cmd;
        if (xQueueReceive(tcp_command_queue, &cmd, pdMS_TO_TICKS(1000))) {
            int sock = -1;
            int addr_family = AF_INET;
            int ip_protocol = IPPROTO_IP;
            
            // Select appropriate device
            struct sockaddr_in *dest_addr = NULL;
            const char *device_type = "";
            
            if (cmd.type == CMD_ROBOT_PLACE) {
                dest_addr = &robot_addr;
                device_type = "robot";
            } else if (cmd.type == CMD_INVERTER_START) {
                dest_addr = &inverter_addr;
                device_type = "inverter";
            } else {
                continue;  // Unknown command type
            }
            
            // Create socket
            sock = socket(addr_family, SOCK_STREAM, ip_protocol);
            if (sock < 0) {
                ESP_LOGE(TAG, "Failed to create socket: errno %d", errno);
                continue;
            }
            
            // Connect to device
            if (connect(sock, (struct sockaddr *)dest_addr, sizeof(*dest_addr)) != 0) {
                ESP_LOGE(TAG, "Connection to %s failed: errno %d", device_type, errno);
                close(sock);
                vTaskDelay(pdMS_TO_TICKS(1000));  // Wait before retrying
                continue;
            }
            
            ESP_LOGI(TAG, "Connected to %s", device_type);
            
            // Device-specific command formatting
            uint8_t buffer[128];
            int len = 0;
            
            switch (cmd.type) {
                case CMD_ROBOT_PLACE:
                    // Format: "PLACE_CUBE\n" (simple text protocol)
                    len = snprintf((char *)buffer, sizeof(buffer), "PLACE_CUBE\n");
                    break;
                    
                case CMD_INVERTER_START:
                    // Format: Modbus TCP frame for Delta inverter
                    // Transaction ID (arbitrary)
                    buffer[0] = 0x00;
                    buffer[1] = 0x01;
                    
                    // Protocol ID (0 for Modbus)
                    buffer[2] = 0x00;
                    buffer[3] = 0x00;
                    
                    // Length (remaining bytes)
                    buffer[4] = 0x00;
                    buffer[5] = 0x06;
                    
                    // Unit ID
                    buffer[6] = DELTA_INVERTER_ADDRESS;
                    
                    // Function code (Write Single Register)
                    buffer[7] = DELTA_START_COMMAND;
                    
                    // Register address
                    buffer[8] = (DELTA_START_REGISTER >> 8) & 0xFF;
                    buffer[9] = DELTA_START_REGISTER & 0xFF;
                    
                    // Start command value (1 = start)
                    buffer[10] = 0x00;
                    buffer[11] = 0x01;
                    
                    len = 12;
                    break;
                    
                    case CMD_NONE:
                    ESP_LOGI(TAG, "Nothing to send");
                    break;
            }
            
            // Send command
            int err = send(sock, buffer, len, 0);
            if (err < 0) {
                ESP_LOGE(TAG, "Send to %s failed: errno %d", device_type, errno);
            } else {
                ESP_LOGI(TAG, "Command sent to %s (%d bytes)", device_type, err);
                
                // For Modbus, wait for response
                if (cmd.type == CMD_INVERTER_START) {
                    uint8_t response[256];
                    int received = recv(sock, response, sizeof(response) - 1, 0);
                    if (received > 0) {
                        ESP_LOGI(TAG, "Inverter response received (%d bytes)", received);
                    }
                }
            }
            
            close(sock);
        }
    }
}

void start_tcp_client_task(void) {
    xTaskCreatePinnedToCore(tcp_client_task,
                            "tcp_client_task",
                            TCP_CLIENT_TASK_STACK_SIZE,
                            NULL,
                            TCP_CLIENT_TASK_PRIORITY,
                            NULL,
                            TCP_CLIENT_TASK_CORE_ID);
}