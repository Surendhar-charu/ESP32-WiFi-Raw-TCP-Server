#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

// ---------- USER SETTINGS ----------
#define WIFI_SSID "Lonelysoul"
#define WIFI_PASS "Charusri"

#define PORT 2323
#define BUFFER_SIZE 128

static const char *TAG = "APP";

// Event group to track WiFi connection
static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

// ---------- WIFI EVENT HANDLER ----------
static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }

    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "WiFi disconnected, retrying...");
        esp_wifi_connect();
    }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG, "CONNECTED!");
        ESP_LOGI(TAG, "IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "PORT: %d", PORT);

        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// ---------- WIFI INIT ----------
void wifi_init_sta(void)
{
    wifi_event_group = xEventGroupCreate();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Register event handlers
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        NULL);

    esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler,
                                        NULL,
                                        NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "Connecting to WiFi...");

    // Wait until connected
    xEventGroupWaitBits(wifi_event_group,
                        WIFI_CONNECTED_BIT,
                        pdFALSE,
                        pdFALSE,
                        portMAX_DELAY);
}

// ---------- TCP SERVER ----------
void start_server(void)
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 1);

    ESP_LOGI(TAG, "Server listening on port %d", PORT);

    while (1) {
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &addr_len);

        ESP_LOGI(TAG, "Client connected");

        while (1) {
            int len = recv(client_fd, buffer, BUFFER_SIZE, 0);

            if (len <= 0) {
                ESP_LOGI(TAG, "Client disconnected");
                break;
            }

            // Echo back (binary-safe)
            send(client_fd, buffer, len, 0);
        }

        close(client_fd);
    }
}

// ---------- MAIN ----------
void app_main(void)
{
    nvs_flash_init();

    wifi_init_sta();   // Connect + print IP
    start_server();    // Start TCP server
}