
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
#include "calls.h"
#include "vars.h"

// void resolve_hostname(const char *);
// uint32_t millis(void);

// extern bool extractSubstring(const char* , char* );


static const char *TAG = "main";



void app_main(void)
{
    //Initialize NVS
    //esp_log_level_set("*", ESP_LOG_NONE);
    // set totals to 0


   
    MQTTRequired = 0;
    for (int i = 0 ; i < 7 ; i++)
    {
        Totals[i] = 0;
        CashTotals[i] = 0;
    }   
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "*FW:%s#",FWVersion);
    ESP_LOGI(TAG, "================================");
    utils_nvs_init();
    status_leds_init();
    console_uart_init();
    uart_write_string(FWVersion);
    read_mac_address();
    xTaskCreate(tcpip_client_task, "tcpip_client_task", 8192, NULL, 7, NULL);
    load_settings_nvs();
    ESP_LOGI(TAG, "*Starting ICH#");
    ICH_init();
    ESP_LOGI(TAG, "*Starting S2P#");
    s2p_init();
    ESP_LOGI(TAG, "*Starting BT acceptor");
    // bt_init();
   
    Out4094(0x00);; // set all outputs inactive
    

    // for (int i = 0 ; i < 3 ; i++)
    // {
    //     led_set_level(LEDR, 1);
    //     led_set_level(LEDG, 0);
    //     vTaskDelay(500/portTICK_PERIOD_MS);   
    //     led_set_level(LEDR, 0);
    //     led_set_level(LEDG, 1);
    //     vTaskDelay(500/portTICK_PERIOD_MS);   
    //     led_set_level(LEDR, 0);
    //     led_set_level(LEDG, 0);
    //     vTaskDelay(500/portTICK_PERIOD_MS);   
    // }
    ESP_LOGI(TAG, "*Starting WiFi#");
    wifi_init_sta();
    ESP_LOGI(TAG, "*Testing RGB #");
    TestRGB();
    
    xTaskCreate(sendHBT, "sendHBT", 2048, NULL, 6, NULL);
    xTaskCreate(BlinkLED, "BlinkLED", 2048, NULL, 6, NULL);
   
    xTaskCreate(TestCoin, "TestCoin", 2048, NULL, 6, NULL);
    if (MQTTRequired)
        InitMqtt();

    for (;;) 
    {
        vTaskDelay(100);   
    }
}
