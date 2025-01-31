#include "wifi.h"

#include <stdio.h>
#include <memory.h>
#include <inttypes.h>
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_netif.h"
#include "esp_wifi.h"


static const char *TAG = "wifi-con";

static int retry_num = 0;

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_STA_START)
    {
        printf("WIFI CONNECTING....\n");
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
        printf("WiFi CONNECTED\n");
        retry_num = 0;
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        printf("WiFi lost connection\n");
        if (retry_num < 5)
        {
            ESP_ERROR_CHECK(esp_wifi_connect());
            retry_num++;
            printf("Retrying to Connect...\n");
        }
    }
    else if (event_id == IP_EVENT_STA_GOT_IP)
    {
        printf("Wifi got IP...\n\n");
    }
    else if (event_id == WIFI_EVENT_STA_STOP) {
        printf("WIFI: sta stop\n");
    }
    else if (event_id == WIFI_EVENT_HOME_CHANNEL_CHANGE) {
        wifi_event_home_channel_change_t *dt =event_data;
        printf("Chang channel, from %d to %d\n", dt->old_chan, dt->new_chan);
    }
    else {
        printf("Got: %li: %p\n", event_id, event_data);
    }
}


#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define u_strncpy(to, from) memcpy(to, from, MIN(sizeof(to), strlen(from) + 1))


void wifi_start(char const *ssid, char const *passwd) {
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
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));
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