/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
//1 - 13
//无线信道也就是常说的无线的“频段（Channel）”，其是以无线信号作为传输媒体的数据信号传送通道。
#define EXAMPLE_ESP_WIFI_CHANNEL   11
//1 - 10 default 4
#define EXAMPLE_MAX_STA_CONN       4

static const char *TAG = "wifi AP";

/**
 * @description: 事件处理循环函数
 * @param 
 * void*            arg         表示传递给Handler函数的参数 在register函数里声明
 * esp_event_base_t event_base  表示事件基  参照esp_event_handler_instance_register函数
 * int32_t          event_id    表示事件ID
 * void*            event_data  表示传递给这个事件的数据
 * @return {*}
 */
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    //有设备通过WIFI连入
    if (event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                MAC2STR(event->mac), event->aid);
    }
    //有连入设备断开连接
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_ap(void)
{
    //init netif
    ESP_ERROR_CHECK(esp_netif_init());
    //event loop create 函数用于创建一个默认的事件循环（同一个esp32程序中可以有多个event_loop，这里使用默认的事件循环）
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //netif creat sta 创建wifi_sta模式的默认netif，返回一个指针
    esp_netif_create_default_wifi_sta();
    //这个宏WIFI_INIT_CONFIG_DEFAULT()可以初始化一个wifi_init_config结构体(初始化结构体)为默认值
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    /**
     * @description:esp_event_handler_instance_register 函数将事件处理程序的实例注册到默认循环。
     * @param 
     * event_base           表示事件基，代表事件的大类（如WiFi事件，IP事件等）
     * event_id             表示事件ID，即事件基下的一个具体事件（如WiFi连接丢失，IP成功获取）
     * event_handler        表示一个handler函数
     * *event_handler_arg   表示需要传递给handler函数的参数
     * *instance            **[输出]** 表示此函数注册的事件实例对象，用于生命周期管理（如删除unrigister这个事件handler）
     * @return {*}
     */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        NULL));

    //wifi配置
    wifi_config_t wifi_config = {
        .ap = {
            //your ID
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            //your PassWord
            .password = EXAMPLE_ESP_WIFI_PASS,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            //Channel of ESP32 soft-AP 1-13
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            //Max number of stations allowed to connect in, default 4, max 10
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                    .required = false,
            },
        }
    };
    //WIFI工作模式选择
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP) );
    //WIFI设置
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config) );
    //启动WIFI
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_ap();
}
