#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "spi_config.h"
#include "wifi_config.h"


#ifdef CONFIG_IDF_TARGET_ESP32
#define CHIP_NAME "ESP32"
#endif

#define PRINT_BOARD_INTERVAL_US 1000000*2

#define SERVERIP "192.168.1.220"

void startGame(int boardId) {
    char* starturl;
    asprintf(&starturl, "http://%s/api/start/", SERVERIP);
    printf(starturl);
    sendHttpRequest(starturl, starturl, true);
}

void app_main(void)
{   
    configure_wifi();
    vTaskDelay(1000/ portTICK_PERIOD_MS);
    //char* starturl = "http://192.168.1.220:24377/api/start/7";
    char* move;// = "{\"boardId\":\"7\",\"from\":\"a2\",\"to\":\"a4\"}";
    char pos1 = 'a';
    char pos2 = '2';
    char pos3 = 'a';
    char pos4 = '4';
    asprintf(&move, "{\"boardId\":\"7\",\"from\":\"%C%C\",\"to\":\"%C%C\"}", pos1,pos2,pos3,pos4);
    printf(move);
    //sendHttpRequest(starturl, move, true);
    startGame(1);
    vTaskDelay(1000/ portTICK_PERIOD_MS);

    char* moveurl = "http://192.168.1.220:24377/api/move";
    //sendHttpRequest(moveurl, move, false);

    while (1) {
        printf("tere\n");
        vTaskDelay(1000/ portTICK_PERIOD_MS);    // Wait at least 100ms
    }
    /*static const char *SPI_TAG = "MAIN";
    uint8_t numb_of_devices = 8;
    device device_arr[numb_of_devices];
    configure_spi(numb_of_devices, device_arr);
    uint16_t print_counter = 0;
    int64_t prev_time = 0;
    while (1) {
        if (QT_MU_1_2_INT_FLAG || QT_MU_3_4_INT_FLAG || QT_SU_1_2_INT_FLAG || QT_SU_3_4_INT_FLAG || QT_INT_ERR_FLAG) {
            if (QT_MU_1_2_INT_FLAG == true) {
                QT_MU_1_2_INT_FLAG = false;
                QT_check_buttons_and_update_board(device_arr[0]);
                QT_check_buttons_and_update_board(device_arr[1]);
            }
            else if (QT_MU_3_4_INT_FLAG == true) {
                QT_MU_3_4_INT_FLAG = false;
                QT_check_buttons_and_update_board(device_arr[2]);
                QT_check_buttons_and_update_board(device_arr[3]); 
            }
            else if (QT_SU_1_2_INT_FLAG == true) {
                QT_SU_1_2_INT_FLAG = false;
                QT_check_buttons_and_update_board(device_arr[4]);
                QT_check_buttons_and_update_board(device_arr[5]);
            }
            else if (QT_SU_3_4_INT_FLAG == true) {
                QT_SU_3_4_INT_FLAG = false;
                QT_check_buttons_and_update_board(device_arr[6]);
                QT_check_buttons_and_update_board(device_arr[7]);
            }
            else if (QT_INT_ERR_FLAG == true) {
                QT_INT_ERR_FLAG = false; 
            }
        }
        vTaskDelay(100/ portTICK_PERIOD_MS);    // Wait at least 100ms
        if (esp_timer_get_time()-prev_time >= PRINT_BOARD_INTERVAL_US) {
            prev_time = esp_timer_get_time();
            printf("Print number: %d\n", print_counter++);
            print_board();
        }
    }*/
}

