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
#define BOARDID 1

int checkboard[8][8] = {};

int fromnumb;
int fromlet;
int tonumb;
int tolet;
bool movedone;
void startGame() {
    char* starturl;
    asprintf(&starturl, "http://%s:24377/api/start/%d", SERVERIP, BOARDID);
    sendHttpRequest(starturl, starturl, true);
}
char* buildMove(char pos1, int pos2, char pos3, int pos4) {
    char* move;
    asprintf(&move, "{\"boardId\":\"%d\",\"from\":\"%C%d\",\"to\":\"%C%d\"}", BOARDID, pos1,pos2,pos3,pos4);
    return move;
}
void sendMove(char* move) {
    char* moveurl;
    asprintf(&moveurl, "http://%s:24377/api/move", SERVERIP);
    sendHttpRequest(moveurl, move, false);
}
void printboard(void) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {   
            if (checkboard[i][j]) {
               printf(" X");
            }
            else {
                printf(" O");
            }
        }
        printf("\n");
    }
}
void compareBoards(void) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {   
            if (checkboard[i][j] == 1 && button_matrix[i][j] == 0) {
                checkboard[i][j] = 0;
                fromnumb = j+1;
                fromlet = i+1;
                movedone = true;
            }
            else if(checkboard[i][j] == 0 && button_matrix[i][j] == 1) {
                checkboard[i][j] = 1;
                tonumb = j+1;
                tolet = i+1;
                movedone = true;
            }
        }
    }
}

char letterFromRow(int row) {
    if (row==1) return 'a';
    if (row==2) return 'b';
    if (row==3) return 'c';
    if (row==4) return 'd';
    if (row==5) return 'e';
    if (row==6) return 'f';
    if (row==7) return 'g';
    if (row==8) return 'h';
    return 'a';
}
void app_main(void)
{   
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (i==0||i==1||i==6||i==7) {
                checkboard[i][j] = 1;
            }
            else {
                checkboard[i][j] = 0;
            }
        }
    }
    configure_wifi();
    vTaskDelay(1000/ portTICK_PERIOD_MS);

    startGame();
    vTaskDelay(1000/ portTICK_PERIOD_MS);
    char* move;

    static const char *SPI_TAG = "MAIN";
    uint8_t numb_of_devices = 8;
    device device_arr[numb_of_devices];
    configure_spi(numb_of_devices, device_arr);
    uint16_t print_counter = 0;
    int64_t prev_time = 0;
    char pos1;
    char pos2;
    //vTaskDelay(40000/ portTICK_PERIOD_MS);
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
        compareBoards();
        if (movedone) {
            pos1 = letterFromRow(fromlet);
            pos2 = letterFromRow(tolet);
            move = buildMove(pos1, fromnumb, pos2, tonumb);
            printf(move);
            printf("\n");
            sendMove(move);
            movedone = false;
        }
        /*if (esp_timer_get_time()-prev_time >= PRINT_BOARD_INTERVAL_US) {
            prev_time = esp_timer_get_time();
            //printf("Print number: %d\n", print_counter++);
            //print_board();
            //buttons = getButtonMatrix();
            printboard();
        }*/
    }
}

