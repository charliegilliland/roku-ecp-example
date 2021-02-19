/* Roku External Control Protocol (ECP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "roku.h"

// Serial connection stuff
#define uart_buffer_size (1024)
const int uart_0 = UART_NUM_0;

// maximum number of devices
#define MAX_ROKU_DEVICES 5

// holds all devices found by roku_discover_devices
struct roku_device devices[MAX_ROKU_DEVICES];

// index for the device we want to use
int selected_device_index = -1;

static void roku_demo_task(void *pvParameters)
{
	// set up uart_0
    uart_config_t uart_0_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    ESP_ERROR_CHECK(uart_driver_install(uart_0, uart_buffer_size*2, uart_buffer_size*2, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_0, &uart_0_config));
    ESP_ERROR_CHECK(uart_set_pin(uart_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));


	printf("\nChoose a device by entering the device number\n");

    for (;;) {
		uint8_t data[128];
		int length = 0;
		ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_0, (size_t*)&length));
        length = uart_read_bytes(uart_0, data, length, (portTickType)portMAX_DELAY);
		if (length > 0) {
			uart_flush(uart_0);
			data[length] = 0; // null-terminate
			char *input = (char *)data;
			if (selected_device_index == -1) {
				selected_device_index = atoi(input);
				printf("Selected device %d, %s\n", selected_device_index, devices[selected_device_index].name);
				printf("Press 'h' to list key commands\n");
			} else {

				switch ((uint8_t) input[0]) {
				case 'e':
					printf("Volume Down\n");
					roku_keypress(devices[selected_device_index], ROKU_VOL_DOWN, ROKU_KEY_PRESS);
					break;
				case 'r':
					printf("Volume Up\n");
					roku_keypress(devices[selected_device_index], ROKU_VOL_UP, ROKU_KEY_PRESS);
					break;
				case 'q':
					printf("Back\n");
					roku_keypress(devices[selected_device_index], ROKU_BACK, ROKU_KEY_PRESS);
					break;
				case 'w':
					printf("Up\n");
					roku_keypress(devices[selected_device_index], ROKU_UP, ROKU_KEY_PRESS);
					break;
				case 'a':
					printf("Left\n");
					roku_keypress(devices[selected_device_index], ROKU_LEFT, ROKU_KEY_PRESS);
					break;
				case 's':
					printf("Select\n");
					roku_keypress(devices[selected_device_index], ROKU_SELECT, ROKU_KEY_PRESS);
					break;
				case 'd':
					printf("Right\n");
					roku_keypress(devices[selected_device_index], ROKU_RIGHT, ROKU_KEY_PRESS);
					break;
				case 'z':
					printf("Home\n");
					roku_keypress(devices[selected_device_index], ROKU_HOME, ROKU_KEY_PRESS);
					break;
				case 'x':
					printf("Down\n");
					roku_keypress(devices[selected_device_index], ROKU_DOWN, ROKU_KEY_PRESS);
					break;
				case '1':
					printf("HDMI1\n");
					roku_keypress(devices[selected_device_index], ROKU_INPUT_HDMI1, ROKU_KEY_PRESS);
					break;
				case '2':
					printf("HDMI2\n");
					roku_keypress(devices[selected_device_index], ROKU_INPUT_HDMI2, ROKU_KEY_PRESS);
					break;
				case '3':
					printf("HDMI3\n");
					roku_keypress(devices[selected_device_index], ROKU_INPUT_HDMI3, ROKU_KEY_PRESS);
					break;
				case 'h':
					printf("Commands:\n");
					printf("'q' = Back\n");
					printf("'w' = Up\n");
					printf("'e' = Volume Down\n");
					printf("'r' = Volume Up\n");
					printf("'a' = Left\n");
					printf("'s' = Select\n");
					printf("'d' = Right\n");
					printf("'z' = Home\n");
					printf("'x' = Down\n");
					printf("'1' = HDMI1\n");
					printf("'2' = HDMI2\n");
					printf("'3' = HDMI3\n");
					printf("'h' = Help Menu - Show this menu\n");
					break;
				default:
					printf("Unrecognized keypress, %s = %d\n", input, (uint8_t) input[0]);
					break;
				}

			}
		} else {
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}
    }
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

	// connect wifi
	wifi_init_sta();
	printf("\n\nWiFi Connected... Starting Roku Example\n\n");

	// discover devices on the network
	int num_devices = roku_discover_devices(devices, MAX_ROKU_DEVICES);
	printf("\nFound %d devices\n", num_devices);

	// print information for each device
	for (int i = 0; i < num_devices; i++) {
		printf("Device %d: %s\t%s\n", i, devices[i].name, devices[i].ip_addr);
	}

	// Start the roku demo task
	xTaskCreate(roku_demo_task, "roku_demo_task", 4096, NULL, 12, NULL);
}
