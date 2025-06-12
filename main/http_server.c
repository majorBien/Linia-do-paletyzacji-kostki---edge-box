/*
 * http_server.c
 *
 *  Created on: Oct 20, 2021
 *      Author: kjagu
 */

#include "esp_http_server.h"
#include "esp_log.h"
#include "sys/param.h"

#include "http_server.h"
#include "tasks_common.h"
#include "wifi_app.h"
#include "cJSON.h"
#include "stdio.h"
#include "string.h"

QueueHandle_t hmi_data_queue;

// Tag used for ESP serial console messages
static const char TAG[] = "http_server";

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// HTTP server monitor task handle
static TaskHandle_t task_http_server_monitor = NULL;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t http_server_monitor_queue_handle;

// Embedded files: JQuery, index.html, app.css, app.js and favicon.ico files
extern const uint8_t jquery_3_3_1_min_js_start[]    asm("_binary_jquery_3_3_1_min_js_start");
extern const uint8_t jquery_3_3_1_min_js_end[]      asm("_binary_jquery_3_3_1_min_js_end");
extern const uint8_t index_html_start[]              asm("_binary_index_html_start");
extern const uint8_t index_html_end[]                asm("_binary_index_html_end");
extern const uint8_t app_css_start[]                 asm("_binary_app_css_start");
extern const uint8_t app_css_end[]                   asm("_binary_app_css_end");
extern const uint8_t app_js_start[]                  asm("_binary_app_js_start");
extern const uint8_t app_js_end[]                    asm("_binary_app_js_end");
extern const uint8_t favicon_ico_start[]             asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[]               asm("_binary_favicon_ico_end");

// Forward declarations for URI handlers
static esp_err_t http_server_hmi_handler(httpd_req_t *req);
static esp_err_t http_server_status_handler(httpd_req_t *req);

/**
 * HTTP server monitor task used to track events of the HTTP server
 * @param pvParameters parameter which can be passed to the task.
 */
static void http_server_monitor(void *parameter)
{
    http_server_queue_message_t msg;

    for (;;)
    {
        if (xQueueReceive(http_server_monitor_queue_handle, &msg, portMAX_DELAY))
        {
            switch (msg.msgID)
            {
                case HTTP_MSG_WIFI_CONNECT_INIT:
                    ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_INIT");
                    break;

                case HTTP_MSG_WIFI_CONNECT_SUCCESS:
                    ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_SUCCESS");
                    break;

                case HTTP_MSG_WIFI_CONNECT_FAIL:
                    ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_FAIL");
                    break;

                default:
                    break;
            }
        }
    }
}

/**
 * Jquery get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_jquery_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Jquery requested");

    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char *)jquery_3_3_1_min_js_start, jquery_3_3_1_min_js_end - jquery_3_3_1_min_js_start);

    return ESP_OK;
}

/**
 * Sends the index.html page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_index_html_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "index.html requested");

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);

    return ESP_OK;
}

/**
 * app.css get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_css_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "app.css requested");

    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char *)app_css_start, app_css_end - app_css_start);

    return ESP_OK;
}

/**
 * app.js get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_js_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "app.js requested");

    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char *)app_js_start, app_js_end - app_js_start);

    return ESP_OK;
}

/**
 * Sends the .ico (icon) file when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "favicon.ico requested");

    httpd_resp_set_type(req, "image/x-icon");
    httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);

    return ESP_OK;
}

/**
 * Sets up the default httpd server configuration.
 * @return http server instance handle if successful, NULL otherwise.
 */
static httpd_handle_t http_server_configure(void)
{
    // Generate the default configuration
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Create the message queue FIRST
    http_server_monitor_queue_handle = xQueueCreate(3, sizeof(http_server_queue_message_t));
    if (!http_server_monitor_queue_handle) {
        ESP_LOGE(TAG, "Failed to create http_server_monitor_queue_handle");
        return NULL;
    }

    // Create HMI data queue
    hmi_data_queue = xQueueCreate(10, sizeof(hmi_data_t));
    if (!hmi_data_queue) {
        ESP_LOGE(TAG, "Failed to create hmi_data_queue");
        return NULL;
    }

    // Create HTTP server monitor task AFTER creating queues
    xTaskCreatePinnedToCore(&http_server_monitor, "http_server_monitor", HTTP_SERVER_MONITOR_STACK_SIZE, 
                            NULL, HTTP_SERVER_MONITOR_PRIORITY, &task_http_server_monitor, 
                            HTTP_SERVER_MONITOR_CORE_ID);

    // The core that the HTTP server will run on
    config.core_id = HTTP_SERVER_TASK_CORE_ID;

    // Adjust the default priority to 1 less than the wifi application task
    config.task_priority = HTTP_SERVER_TASK_PRIORITY;

    // Bump up the stack size (default is 4096)
    config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;

    // Increase uri handlers
    config.max_uri_handlers = 20;

    // Increase the timeout limits
    config.recv_wait_timeout = 10;
    config.send_wait_timeout = 10;

    ESP_LOGI(TAG,
            "http_server_configure: Starting server on port: '%d' with task priority: '%d'",
            config.server_port,
            config.task_priority);

    // Start the httpd server
    if (httpd_start(&http_server_handle, &config) == ESP_OK)
    {
        ESP_LOGI(TAG, "http_server_configure: Registering URI handlers");

        // register query handler
        httpd_uri_t jquery_js = {
                .uri = "/jquery-3.3.1.min.js",
                .method = HTTP_GET,
                .handler = http_server_jquery_handler,
                .user_ctx = NULL
        };
        httpd_register_uri_handler(http_server_handle, &jquery_js);

        // register index.html handler
        httpd_uri_t index_html = {
                .uri = "/",
                .method = HTTP_GET,
                .handler = http_server_index_html_handler,
                .user_ctx = NULL
        };
        httpd_register_uri_handler(http_server_handle, &index_html);

        // register app.css handler
        httpd_uri_t app_css = {
                .uri = "/app.css",
                .method = HTTP_GET,
                .handler = http_server_app_css_handler,
                .user_ctx = NULL
        };
        httpd_register_uri_handler(http_server_handle, &app_css);

        // register app.js handler
        httpd_uri_t app_js = {
                .uri = "/app.js",
                .method = HTTP_GET,
                .handler = http_server_app_js_handler,
                .user_ctx = NULL
        };
        httpd_register_uri_handler(http_server_handle, &app_js);

        // register favicon.ico handler
        httpd_uri_t favicon_ico = {
                .uri = "/favicon.ico",
                .method = HTTP_GET,
                .handler = http_server_favicon_ico_handler,
                .user_ctx = NULL
        };
        httpd_register_uri_handler(http_server_handle, &favicon_ico);
        
        // Register HMI API handler
        httpd_uri_t hmi_uri = {
            .uri      = "/api/hmi",
            .method   = HTTP_POST,
            .handler  = http_server_hmi_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(http_server_handle, &hmi_uri);
        
        // Register status API handler
        httpd_uri_t status_uri = {
            .uri      = "/api/status",
            .method   = HTTP_GET,
            .handler  = http_server_status_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(http_server_handle, &status_uri);

        return http_server_handle;
    }

    return NULL;
}

void http_server_start(void)
{
    if (http_server_handle == NULL)
    {
        http_server_handle = http_server_configure();
    }
}

void http_server_stop(void)
{
    if (http_server_handle)
    {
        httpd_stop(http_server_handle);
        ESP_LOGI(TAG, "http_server_stop: stopping HTTP server");
        http_server_handle = NULL;
    }
    if (task_http_server_monitor)
    {
        vTaskDelete(task_http_server_monitor);
        ESP_LOGI(TAG, "http_server_stop: stopping HTTP server monitor");
        task_http_server_monitor = NULL;
    }
    if (http_server_monitor_queue_handle) {
        vQueueDelete(http_server_monitor_queue_handle);
        http_server_monitor_queue_handle = NULL;
    }
    if (hmi_data_queue) {
        vQueueDelete(hmi_data_queue);
        hmi_data_queue = NULL;
    }
}

BaseType_t http_server_monitor_send_message(http_server_message_e msgID)
{
    http_server_queue_message_t msg;
    msg.msgID = msgID;
    return xQueueSend(http_server_monitor_queue_handle, &msg, portMAX_DELAY);
}
static esp_err_t http_server_hmi_handler(httpd_req_t *req)
{
    char buf[128]; // bufor na ciało żądania (dostosuj wielkość jeśli trzeba)
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        ESP_LOGE(TAG, "Failed to read request body");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request body");
        return ESP_FAIL;
    }
    buf[ret] = '\0'; // zakończ string

    // Parsowanie JSON
    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        ESP_LOGE(TAG, "Invalid JSON");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    const cJSON *type = cJSON_GetObjectItemCaseSensitive(root, "type");
    if (!cJSON_IsString(type) || type->valuestring == NULL) {
        ESP_LOGE(TAG, "Missing or invalid 'type'");
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or invalid 'type'");
        return ESP_FAIL;
    }

    const cJSON *data = cJSON_GetObjectItemCaseSensitive(root, "data");

    hmi_data_t hmi = {0};

    // Alokuj string na stercie
    hmi.type = strdup(type->valuestring);
    if (!hmi.type) {
        ESP_LOGE(TAG, "Failed to allocate memory for type");
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal error");
        return ESP_FAIL;
    }

    if (cJSON_IsNumber(data)) {
        hmi.value = data->valuedouble;
    } else {
        hmi.value = 0.0; // lub możesz użyć NAN
    }

    // Opcjonalnie: wpisz całe żądanie do cmd (np. do logów)
    snprintf(hmi.cmd, sizeof(hmi.cmd), "%s", buf);

    // Wrzuć do kolejki
    if (xQueueSend(hmi_data_queue, &hmi, portMAX_DELAY) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to enqueue HMI data");
        free(hmi.type);  // pamiętaj o zwolnieniu alokacji
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Queue full");
        return ESP_FAIL;
    }

    cJSON_Delete(root);
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}


static esp_err_t http_server_status_handler(httpd_req_t *req)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "weight", 12.34);
    cJSON_AddStringToObject(root, "weightStatus", "OK");
    cJSON_AddBoolToObject(root, "sensor1", true);
    cJSON_AddBoolToObject(root, "sensor3", false);
    cJSON_AddBoolToObject(root, "wrap_done", false);
    cJSON_AddBoolToObject(root, "robot", true);
    cJSON_AddBoolToObject(root, "inverter", true);
    cJSON_AddNumberToObject(root, "wrapProgress", 75);

    const char *json_str = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json_str);
    
    cJSON_free((void*)json_str);
    cJSON_Delete(root);

    return ESP_OK;
}