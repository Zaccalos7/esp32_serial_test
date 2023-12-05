#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include <ctype.h>

#define MAX_CMD_LENGTH 50
#define ECHO_UART_PORT_NUM (CONFIG_EXAMPLE_UART_PORT_NUM)
#define BUF_SIZE 1024
#define TRUE 1
#define FALSE 0
#define DEFAULT -1

int LED_COLOR = DEFAULT;
int led_index;

// functions
void turn_led_on();
void turn_led_off();
void write_string_output(char *text);
void check(char *string_verify);
void do_cmd();
void delect(char *test);
void reset_check_control();
int count_word_string(char *test);

// int backspace_single(char *test, int index);
bool check_command(char *element_n);
bool check_parameter(char *element_n);
bool check_color(char *element_n);
char lower_char(char ch);

// Command's List //
char *command_switch[] = {"led", "on", "off", "red", "blue", "green"};
const char space[] = " ";

// lenght input string read from serial
#define CMD_S_COUNT (sizeof(command_switch) / sizeof(char *))
int MAX_word = CMD_S_COUNT;

typedef struct struct_parameter{
    const char *parameter;
    void (*callback)();
} parameter_t;

typedef struct struct_command{
    const char *cmd;
    parameter_t *parameters;
} command_t;

parameter_t led_parameters[] = {
    {.parameter = "on",
    .callback = &turn_led_on},
    {.parameter = "off",
    .callback = &turn_led_off},
};

command_t command = {
    .cmd = "led",
    .parameters = led_parameters,
};

// GLOBAL FOR CHECK STRING
bool find_cmd = FALSE;
bool find_parameters = FALSE;
bool find_color = FALSE;

void delect(char *test){
    char null_string[MAX_CMD_LENGTH]="";
    strcpy(test, null_string);
}

void reset_check_control(){
    // flags
    find_cmd = FALSE;
    find_parameters = FALSE;
    find_color = FALSE;
    // LED RGB
    LED_COLOR = DEFAULT;
}

void check(char *string_verify){
    int len_string_input = count_word_string(string_verify);
    int increment = 0;
    char *token = strtok(string_verify, space);
    while (token != NULL){
    // SEARCH
        if (find_cmd == FALSE){
            find_cmd = check_command(token);
        }
        
        if (find_parameters == FALSE){
            find_parameters = check_parameter(token);
        }

        if (find_color == FALSE){
            find_color = check_color(token);
        }

        // EXIT
        if (increment == MAX_word || increment > len_string_input){
            token = NULL;
            write_string_output("ERROR 404:\tCOMMAND NOT FOUND");
        }
        
        // CHANGE TOKEN
        token = strtok(NULL, space);
        increment++;
    }
}

bool check_command(char *element_n){
    
    bool flag = FALSE;
    // find out and save it
    if (strcmp(element_n, command.cmd) == 0)
        flag = TRUE;

    return flag;
}
    
bool check_parameter(char *element_n){
    
    bool flag = FALSE;
    int cmp_result = 0;
    int index_parameter = 2; // in future calculate with sizeof
    
    for (int i = 0; i < index_parameter; i++){
        cmp_result = strcmp(element_n, led_parameters[i].parameter);
        
        if (cmp_result == 0){
        flag = TRUE;
        led_index = i;
        }
    }
    
    return flag;

}

bool check_color(char *element_n){
    
    bool flag = FALSE;
    char color[20];
    strcpy(color, element_n);
    
    switch (color[0]){
        case 'r':
            LED_COLOR = 0;
            flag = TRUE;
        break;
        
        case 'g':
            LED_COLOR = 2;
            flag = TRUE;
        break;

        case 'b':
            LED_COLOR = 4;
            flag = TRUE;
        break;

    }
    return flag;
}


void do_cmd(){
    
    if (find_cmd == TRUE && find_parameters == TRUE){
        led_parameters[led_index].callback();
    }
}

void turn_led_on(){
    
    if (LED_COLOR != DEFAULT){
    gpio_set_level(LED_COLOR, 1);
    }
    else if (LED_COLOR == DEFAULT){
        write_string_output("NO LED FOUND");
    }

}


void turn_led_off(){
    
    if (LED_COLOR != DEFAULT){
        gpio_set_level(LED_COLOR, 0);
    }
    else if (LED_COLOR == DEFAULT){
        write_string_output("NO LED FOUND");
    }

}


char serial_read_single(){
    
    char data;
    while (uart_read_bytes(ECHO_UART_PORT_NUM, &data, 1, 20 / portTICK_RATE_MS) <= 0){
    }

    uart_write_bytes(ECHO_UART_PORT_NUM, &data, 1);
    return data;

}


char lower_char(char ch){

    int ascii_dex;
    ascii_dex = (int)ch;
    
    if (ascii_dex >= 65 && ascii_dex <= 90){
        ch = tolower(ch);
        return ch;
    }

    return ch;

}


char backspace_single(char test){
    
    test = ' ';
    return test;

}


void write_string_output(char *text){
    
    if (text == NULL)
    return;
    uart_write_bytes(ECHO_UART_PORT_NUM, "\n", 1);
    uart_write_bytes(ECHO_UART_PORT_NUM, text, strlen(text));
    uart_write_bytes(ECHO_UART_PORT_NUM, "\n", 1);

}


int count_word_string(char *cmd){
    
    int len = strlen(cmd);
    int count_word = 0;
    
    for (int i = 0; i < len; i++){
        
        if (cmd[i] == ' '){
            count_word++;
        }
        else if (cmd[i + 1] == '\0'){
            count_word++;
        }
    }

    return count_word;

}


void led_manager(){
    
    char cmd[MAX_CMD_LENGTH];
    int cmd_length = 0;

    while (1){
    
    char ch = serial_read_single(); // read one byte at a time
    //delete one byte
    if (ch == '\b'){
        ch=backspace_single(ch);
        cmd_length--;
    //cmd[cmd_length]=ch;
    }

    // from uppercase or lower to lower
    ch = lower_char(ch);
    //save in buffer
    cmd[cmd_length] = ch;
    
    //end of trasmission
    if (cmd[cmd_length] == '\r' || cmd[cmd_length] == '\n'){
        cmd[cmd_length] = '\0';
        
        if (cmd_length == 0){
            continue;
        }
        // cicle of check command
        check(cmd);
        do_cmd();
        delect(cmd);
        reset_check_control();

        cmd_length = 0;
    }
    else{
        cmd_length++;
    }
    
}
}