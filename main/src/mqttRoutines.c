#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"   
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_timer.h"
#include "esp_ota_ops.h"
#include "driver/uart.h"
#include "esp_netif.h"
#include "rom/ets_sys.h"
#include "esp_smartconfig.h"
#include <sys/socket.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "mqtt_client.h"

#include "externVars.h"
#include "calls.h"

static const char *TAG = "MQTT";

void InitMqtt (void);

int32_t MQTT_CONNEECTED = 0;

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */


 esp_mqtt_client_handle_t client = NULL;


void publish_sip_message(const char *message, esp_mqtt_client_handle_t client) {
    // Publish the provided message to the MQTT topic
    esp_mqtt_client_publish(client, "GVC/KP/ALL", message, strlen(message), 0, 0);

    // Indicate that a transaction is pending
    tx_event_pending = 1;

    // Log the published message for debugging
    ESP_LOGI(TAG, "Published SIP message: %s", message);
}

void Publisher_Task(void *params)
{
  while (true)
  {
    if(MQTT_CONNEECTED)
    {
        publish_sip_message("*HBT#", client);
        vTaskDelay(15000 / portTICK_PERIOD_MS);
    }
  }
}


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, (int)event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    // Declare variables outside the switch statement
    char topic[256]; // Assuming a max topic length
    char data[256];  // Assuming a max data length
    char payload[400];
    char rx_buffer[128];

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        MQTT_CONNEECTED = 1;  // Ensure MQTT_CONNECTED is defined

        msg_id = esp_mqtt_client_subscribe(client, "GVC/KP/00002", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "GVC/KP/00003", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        MQTT_CONNEECTED = 0;
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
        ESP_LOGI(TAG,"TOPIC=%.*s\r\n", event->topic_len, event->topic);
        ESP_LOGI(TAG,"DATA=%.*s\r\n", event->data_len, event->data);

        // Ensure topic and data are within bounds
        if (event->topic_len < sizeof(topic) && event->data_len < sizeof(data)) {
            strncpy(topic, event->topic, event->topic_len);
            topic[event->topic_len] = '\0';

            strncpy(data, event->data, event->data_len);
            data[event->data_len] = '\0';

            if (strcmp(topic, "GVC/KP/00002") == 0) {
                if (strcmp(data, "*HBT#") == 0) {
                    ESP_LOGI(TAG, "Heartbeat message received.");
                } else if (strcmp(data, "SS1:") == 0) {
                    ESP_LOGI(TAG, "Command 1 received.");
                    // Execute action for COMMAND1
                } else if (strcmp(data, "COMMAND2") == 0) {
                    ESP_LOGI(TAG, "Command 2 received.");
                    // Execute action for COMMAND2
                } 
                else if(strncmp(data, "*SIP:", 5) == 0){
                                  
                    sscanf(data, "*SIP:%[^:]:%d#",server_ip_addr,
                        &sp_port);
                    strcpy(SIPuserName,"MQTT_LOCAL");
                    strcpy(SIPdateTime,"00/00/00");
                    char buf[100];
                    sprintf(payload, "*SIP-OK,%s,%s#",SIPuserName,SIPdateTime);
                    sprintf(buf, "%s",server_ip_addr);
                    

                    utils_nvs_set_str(NVS_SERVER_IP_KEY, buf);
                    utils_nvs_set_int(NVS_SERVER_PORT_KEY, sp_port);
                    utils_nvs_set_str(NVS_SIP_USERNAME, SIPuserName);
                    utils_nvs_set_str(NVS_SIP_DATETIME, SIPdateTime);
                    
                     publish_sip_message(payload, client);
                    // send(sock, payload, strlen(payload), 0);
                    tx_event_pending = 1;
                }
                else if(strncmp(data, "*SIP?#", 6) == 0){
                    sprintf(payload, "*SIP,%s,%s,%s,%d#",SIPuserName,SIPdateTime,server_ip_addr,
                    sp_port ); //actual when in production
                     publish_sip_message(payload, client);
                    ESP_LOGI(TAG, "*SIP,%s,%s,%s,%d#",SIPuserName,SIPdateTime,server_ip_addr,
                    sp_port );
                }
                else if(strncmp(data, "*CC#", 4) == 0){
                  
                    ESP_LOGI(TAG, "*CC-OK#");
                    strcpy(CCuserName,"MQTT_LOCAL");
                    strcpy(CCdateTime,"00/00/00");
                   
                    utils_nvs_set_str(NVS_CC_USERNAME, CCuserName);
                    utils_nvs_set_str(NVS_CC_DATETIME, CCdateTime);
                    sprintf(payload, "*CC-OK,%s,%s#",CCuserName,CCdateTime);
                     publish_sip_message(payload, client);
                    for (int i = 0 ; i < 7 ; i++)
                    {
                        Totals[i] = 0;
                        CashTotals[i] = 0;
                    } 
                    utils_nvs_set_int(NVS_CASH1_KEY, CashTotals[0]);
                    utils_nvs_set_int(NVS_CASH2_KEY, CashTotals[1]);
                    utils_nvs_set_int(NVS_CASH3_KEY, CashTotals[2]);
                    utils_nvs_set_int(NVS_CASH4_KEY, CashTotals[3]);
                    utils_nvs_set_int(NVS_CASH5_KEY, CashTotals[4]);
                    utils_nvs_set_int(NVS_CASH6_KEY, CashTotals[5]);
                    utils_nvs_set_int(NVS_CASH7_KEY, CashTotals[6]);
                }
                else if(strncmp(data, "*RST#", 5) == 0){
                        ESP_LOGI(TAG, "**************Restarting after 3 second*******");
                        strcpy(RSTuserName,"MQTT_LOCAL");
                        strcpy(RSTdateTime,"00/00/00");
                        sprintf(payload, "*RST-OK,%s,%s#",RSTuserName,RSTdateTime);
                         publish_sip_message(payload, client);
                        ESP_LOGI(TAG, "*RST-OK#");
                        vTaskDelay(3000/portTICK_PERIOD_MS);
                        esp_restart();
                }
                 else if(strncmp(data, "*URL?#", 6) == 0){
                    ESP_LOGI(TAG,"URL RECEIVED,%s,%s,%s",URLuserName,URLdateTime,FOTA_URL);
                    char msg[600];
                    sprintf(msg,"*URL,%s,%s,%s#",URLuserName,URLdateTime,FOTA_URL); 
                     publish_sip_message(msg, client);
                    tx_event_pending = 1;
                }
                else {
                    ESP_LOGI(TAG, "Unknown message received.");
                }
            }
        } else {
            ESP_LOGE(TAG, "Received topic/data too large");
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        // Add more error handling here if needed
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}


void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "STARTING MQTT");
    esp_mqtt_client_config_t mqttConfig = {
         .broker.address.uri = "mqtt://zest-iot.in:1883",
        .session.protocol_ver = MQTT_PROTOCOL_V_3_1_1,
        .network.disable_auto_reconnect = true,
        .credentials.username = "123",
        .credentials.authentication.password = "456",
        .session.last_will.topic = "GVC/VM/00002",
        .session.last_will.msg = "i will leave",
        .session.last_will.msg_len = 12,
        .session.last_will.qos = 1,
        .session.last_will.retain = true,};
    
    client = esp_mqtt_client_init(&mqttConfig);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}





void InitMqtt (void)
{
     xTaskCreate(Publisher_Task, "Publisher_Task", 1024 * 5, NULL, 5, NULL);
    
     ESP_LOGI(TAG,"MQTT Initiated");
}