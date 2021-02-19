#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_system.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_http_client.h"

#include "roku.h"

static const char *TAG = "roku client";

/*
Scans the network for Roku Devices
*/
int roku_discover_devices(struct roku_device *devices, uint8_t max_devices)
{
    int num_devices = 0;

    // send the search query
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(ROKU_SEARCH_IP);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(ROKU_SEARCH_PORT);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    }
    ESP_LOGI(TAG, "Socket created, sending to %s:%d", ROKU_SEARCH_IP, ROKU_SEARCH_PORT);

    int err = sendto(sock, ROKU_SEARCH_PAYLOAD, strlen(ROKU_SEARCH_PAYLOAD), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
    ESP_LOGI(TAG, "Search message sent");

    // read the responses
    int recv_len = 0;
    struct sockaddr_in source_addr;
    socklen_t socklen = sizeof(source_addr);

    const struct timeval timeout = { 1, 0 }; // 1 second timeout
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    do {
        char rx_buffer[512];
        recv_len = recvfrom(sock, rx_buffer, (sizeof(char) * 512 )- 1, SO_RCVTIMEO, (struct sockaddr *)&source_addr, &socklen);

        // Error occurred during receiving
        if (recv_len <= 0) {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            break;
        } else {
            rx_buffer[recv_len] = 0; // null-terminate

            ESP_LOGI(TAG, "Received %d bytes from %s:", recv_len, ROKU_SEARCH_IP);

            // parse the IP address
            // TODO: there has to be a better way to do this...
            char *ip_address_start, *ip_address_end;
            ip_address_start = strstr(rx_buffer, "http://");
            ip_address_end = strstr(rx_buffer, ":8060/");
            
            int ip_len = ip_address_end - ip_address_start - 7;
            int offset = ip_address_start - rx_buffer + 7;

            char ip_address[ip_len+1];
            int i = 0;
            while (i < ip_len) {
                ip_address[i] = rx_buffer[offset+i];
                i++;
            }
            ip_address[ip_len+1] = 0; // null-terminate

            // store the IP address in the struct
            strcpy(devices[num_devices].ip_addr, ip_address);
            num_devices++;
        }
    } while (recv_len > 0 && num_devices < max_devices);
    shutdown(sock, 0);
    close(sock);

    // query each device IP for its name, storing it in the respective struct
    for (int i = 0; i < num_devices; i++) {
        // first, send the request to the device
        esp_http_client_config_t config = {
            .host = devices[i].ip_addr,
            .path = "/query/device-info",
            .port = 8060
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_err_t err;
        if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
            continue;
        }

        int content_length =  esp_http_client_fetch_headers(client);
        char *buffer = malloc(content_length + 1);
        if (buffer == NULL) {
            ESP_LOGI(TAG, "Cannot malloc http receive buffer");
            continue;
        }

        int read_len = esp_http_client_read(client, buffer, content_length);
        if (read_len <= 0) {
            ESP_LOGE(TAG, "Error read data");
            free(buffer);
            continue;
        }
        buffer[read_len] = 0; // null-terminate

        ESP_LOGI(TAG, "HTTP Stream reader Status = %d, content_length = %d",
                        esp_http_client_get_status_code(client),
                        esp_http_client_get_content_length(client));
        esp_http_client_close(client);
        esp_http_client_cleanup(client);

        // now, parse the response and save the name
        char *device_name_start, *device_name_end;
        device_name_start = strstr(buffer, "<user-device-name>");
        device_name_end = strstr(buffer, "</user-device-name>");

        int len = device_name_end - device_name_start - 18;
        int offset = device_name_start - buffer + 18;

        char device_name[len+1];
        int j = 0;
        while (j < len) {
            device_name[j] = buffer[offset+j];
            j++;
        }
        device_name[len] = 0; // null-terminate

        // store the name in the struct
        strcpy(devices[i].name, device_name);

        free(buffer);
    }

    return num_devices;
}

/*
Sends a keypress command to the specified device
*/
int roku_keypress(struct roku_device device, char *command, const char *keypress_type)
{
    // build the path string
    int path_len = strlen(keypress_type) + strlen(command) + 1;
    char request_path[path_len];
    sprintf(request_path, "%s%s", keypress_type, command);

    esp_http_client_config_t config = {
        .host = device.ip_addr,
        .port = 8060,
        .path = request_path
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    
    // send the HTTP request
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
    
    int status_code = esp_http_client_get_status_code(client);

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    return status_code;
}