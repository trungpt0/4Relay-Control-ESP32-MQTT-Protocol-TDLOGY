/**
 * @file app_main.c
 * @author Trung Tran
 * @brief 
 * @version 0.1
 * @date 2024-05-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

/* Define GPIO */
#define RELAY_GPIO_1     ((gpio_num_t)13)
#define RELAY_GPIO_2     ((gpio_num_t)14)
#define RELAY_GPIO_3     ((gpio_num_t)18)
#define RELAY_GPIO_4     ((gpio_num_t)19)

/* Message control 4 relay */
#define RELAY1_ON_TOPIC    "RELAY1:ON"
#define RELAY2_ON_TOPIC    "RELAY2:ON"
#define RELAY3_ON_TOPIC    "RELAY3:ON"
#define RELAY4_ON_TOPIC    "RELAY4:ON"
#define RELAY1_OFF_TOPIC   "RELAY1:OFF"
#define RELAY2_OFF_TOPIC   "RELAY2:OFF"
#define RELAY3_OFF_TOPIC   "RELAY3:OFF"
#define RELAY4_OFF_TOPIC   "RELAY4:OFF"

static const char *TAG = "MQTT_ESP32_4RELAY:";

/**
 * @brief Configure a relay GPIO pin.
 * 
 * @param GPIO_Num The GPIO pin number to configure.
 */
static void config_relay(gpio_num_t GPIO_Num)
{
    ESP_LOGI(TAG, "Configured to blink GPIO LED%d!", GPIO_Num);
    gpio_reset_pin(GPIO_Num);
    gpio_set_direction(GPIO_Num, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_Num, 0);
}

/**
 * @brief Log an error message if the error code is non-zero.
 * 
 * @param message The error message.
 * @param error_code The error code to check.
 */
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/**
 * @brief MQTT event handler function.
 * 
 * @param handler_args Handler arguments.
 * @param base Base event type.
 * @param event_id Event ID.
 * @param event_data Event data.
 */
esp_mqtt_event_handle_t event;
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "data", "Publish from ESP32_4RELAY", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "data", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        /* Control 4 relay */
        if (strncmp(event->data, RELAY1_ON_TOPIC, event->data_len) == 0) {
            gpio_set_level(RELAY_GPIO_1, 1);
        } else if (strncmp(event->data, RELAY2_ON_TOPIC, event->data_len) == 0) {
            gpio_set_level(RELAY_GPIO_2, 1);
        } else if (strncmp(event->data, RELAY3_ON_TOPIC, event->data_len) == 0) {
            gpio_set_level(RELAY_GPIO_3, 1);
        } else if (strncmp(event->data, RELAY4_ON_TOPIC, event->data_len) == 0) {
            gpio_set_level(RELAY_GPIO_4, 1);
        } else if (strncmp(event->data, RELAY1_OFF_TOPIC, event->data_len) == 0) {
            gpio_set_level(RELAY_GPIO_1, 0);
        } else if (strncmp(event->data, RELAY2_OFF_TOPIC, event->data_len) == 0) {
            gpio_set_level(RELAY_GPIO_2, 0);
        } else if (strncmp(event->data, RELAY3_OFF_TOPIC, event->data_len) == 0) {
            gpio_set_level(RELAY_GPIO_3, 0);
        } else if (strncmp(event->data, RELAY4_OFF_TOPIC, event->data_len) == 0) {
            gpio_set_level(RELAY_GPIO_4, 0);
        } else {
            printf("ERROR! %.*s\r\n", event->data_len, event->data);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}


/**
 * @brief Start the MQTT application.
 * 
 */
static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = "mqtt://mqtt.eclipseprojects.io",
        },
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    mqtt_app_start();

    /* Config relay */
    config_relay(RELAY_GPIO_1);
    config_relay(RELAY_GPIO_2);
    config_relay(RELAY_GPIO_3);
    config_relay(RELAY_GPIO_4);
}