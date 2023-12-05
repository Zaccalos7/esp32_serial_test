#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include <string.h>

#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_UART_PORT_NUM (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define ECHO_TASK_STACK_SIZE (CONFIG_EXAMPLE_TASK_STACK_SIZE)

#define LED_RED 0
#define LED_BLUE 4
#define LED_GREEN 2
#define BUF_SIZE 1024

void led_manager();

static void echo_task(void *arg){

    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(ECHO_UART_PORT_NUM, &uart_config);
    uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_write_bytes(ECHO_UART_PORT_NUM, "SYSTEM READY!!!\n", 17);
    uart_write_bytes(ECHO_UART_PORT_NUM, "\t\tWRITE\n", strlen("\t\tWRITE\n"));
    uart_write_bytes(ECHO_UART_PORT_NUM, "\tLED STATUS\tor\tLED ON OFF \n", strlen("\tLED STATUS\tor\tLED ON OFF \n"));
    // uart_write_bytes(ECHO_UART_PORT_NUM,"\t$:",strlen("\t$:"));
    led_manager();
}


void app_main(void){

    gpio_pad_select_gpio(LED_RED);
    gpio_set_direction(LED_RED, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_RED, 0);
    gpio_pad_select_gpio(LED_BLUE);
    gpio_set_direction(LED_BLUE, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_BLUE, 0);
    gpio_pad_select_gpio(LED_GREEN);
    gpio_set_direction(LED_GREEN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GREEN, 0);
    // esp_flash_get_chip_size()
    xTaskCreate(echo_task, "echo_task", ECHO_TASK_STACK_SIZE, NULL, 5, NULL);
    
}