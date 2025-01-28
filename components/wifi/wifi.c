#include "wifi.h"

#include <stdio.h>
#include <memory.h>
#include <inttypes.h>
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_netif.h"
#include "esp_wifi.h"


static const char *TAG = "wifi-con";

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define u_strncpy(to, from) memcpy(to, from, MIN(sizeof(to), strlen(from) + 1))


void wifi_init(char const *ssid, char const *passwd) {
    ESP_LOGI(TAG, "Initializing nvs");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    // init wifi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta(); //sets up necessary data structs for wifi station interface
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) ); 
    wifi_config_t wifi_configuration = { //struct wifi_config_t var wifi_configuration
        .sta= {
            .ssid = {0},
            .password= {0},
            .failure_retry_cnt = 10,
            .channel = 0,
        }
    };
    u_strncpy(wifi_configuration.sta.ssid, ssid);
    u_strncpy(wifi_configuration.sta.password, passwd);
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration));//setting up configs when event ESP_IF_WIFI_STA


    ESP_ERROR_CHECK( esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect()); //connect with saved ssid and pass

    ESP_ERROR_CHECK( esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N) ); 
}