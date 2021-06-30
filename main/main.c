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

#define btoa(x) ((x)?"true":"false")

typedef struct {
    bool white;
    bool black;
    char letpos;
    int numpos;
    bool buttondead;
} chesspiece;

#ifdef CONFIG_IDF_TARGET_ESP32
#define CHIP_NAME "ESP32"
#endif

#define PRINT_BOARD_INTERVAL_US 1000000*2

#define SERVERIP "192.168.1.220"
#define BOARDID 1

int checkboard[8][8] = {};

uint8_t numb_of_chesspieces = 32;
//int fromnumb;
//int fromlet;
//int tonumb;
//int tolet;
int count = 0;
void startGame() {
    char* starturl;
    asprintf(&starturl, "http://%s:24377/api/start/%d", SERVERIP, BOARDID);
    sendHttpRequest(starturl, starturl, true);
}
char* buildMove(char fromLet, int fromNumb, char toLet, int toNumb) {
    char* move;
    asprintf(&move, "{\"boardId\":\"%d\",\"from\":\"%C%d\",\"to\":\"%C%d\"}", BOARDID, fromLet, fromNumb, toLet, toNumb);
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

void fillChessPiecesArray(chesspiece *chesspiece_arr) {
    int column = 0;
    for (int i = 0; i < numb_of_chesspieces; ++i) {
        if (i < 16) {
            chesspiece_arr[i].white = true;
            chesspiece_arr[i].black = false;
            chesspiece_arr[i].buttondead = false;
            chesspiece_arr[i].letpos = letterFromColumn(column);
            if (i < 8) {
                chesspiece_arr[i].numpos = 1;
            }
            else {
                chesspiece_arr[i].numpos = 2;
            }
        }
        else {
            chesspiece_arr[i].black = true;
            chesspiece_arr[i].white = false;
            chesspiece_arr[i].buttondead = false;
            chesspiece_arr[i].letpos = letterFromColumn(column);
            if (i < 24) {
                chesspiece_arr[i].numpos = 7;
            }
            else {
                chesspiece_arr[i].numpos = 8;
            }
        }
        column+=1;
        if (column == 8) {
            column = 0;
        }
    }
}

bool checkFromPos(chesspiece *chesspiece_arr, char fromLet, char fromNumb, bool whiteturn) {
    for (int i = 0; i < numb_of_chesspieces; ++i) {
        if (whiteturn) {
            if (chesspiece_arr[i].white) {
                if (fromLet == chesspiece_arr[i].letpos && fromNumb == chesspiece_arr[i].numpos) {
                    return true;
                }
            }
        }
        else {
            if (chesspiece_arr[i].black) {
                if (fromLet == chesspiece_arr[i].letpos && fromNumb == chesspiece_arr[i].numpos) {
                    return true;
                }
            }
        }
    }
    return false;
}
void changeButtonPos(chesspiece *chesspiece_arr, char fromLet, int fromNumb, bool whiteturn, char toLet, int toNumb) {
    for (int i = 0; i < numb_of_chesspieces; ++i) {
        if (whiteturn) {
            if (chesspiece_arr[i].white) {
                if (fromLet == chesspiece_arr[i].letpos && fromNumb == chesspiece_arr[i].numpos) {
                    chesspiece_arr[i].letpos = toLet;
                    chesspiece_arr[i].numpos = toNumb;
                }
            }
        }
        else {
            if (chesspiece_arr[i].black) {
                if (fromLet == chesspiece_arr[i].letpos && fromNumb == chesspiece_arr[i].numpos) {
                    chesspiece_arr[i].letpos = toLet;
                    chesspiece_arr[i].numpos = toNumb;
                }
            }
        }
    }
}
void app_main(void)
{   

    configure_wifi();
    vTaskDelay(1000/ portTICK_PERIOD_MS);
    bool whiteturn = true;
    char* move;

    static const char *SPI_TAG = "MAIN";
    uint8_t numb_of_devices = 8;
    
    device device_arr[numb_of_devices];
    
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) { 
            if (j == 0 || j == 1 || j == 6 || j == 7) {
                button_matrix[i][j] = 1;
            }
        }
    }
    
    chesspiece chesspiece_arr[numb_of_chesspieces];
    
    fillChessPiecesArray(chesspiece_arr);

    configure_spi(numb_of_devices, device_arr);
    int64_t prev_time = 0;
    vTaskDelay(1000/ portTICK_PERIOD_MS);
    printf("START"); 
    printf("\n");
    startGame();
    while (1) {

        check_buttons(device_arr);
        vTaskDelay(100/ portTICK_PERIOD_MS);
        if (fromdone && !todone) {
            if (!checkFromPos(chesspiece_arr, fromLet, fromNumb, whiteturn)) {
                fromdone = 0;
            }
        }
        if (fromdone && todone && (toLet != 'x' && toNumb != 10)) {
            printf("whiteorblack: ");
            printf(btoa(whiteturn));
            printf("\n");
            move = buildMove(fromLet, fromNumb, toLet, toNumb);
            sendMove(move);
            changeButtonPos(chesspiece_arr, fromLet, fromNumb, whiteturn, toLet, toNumb);
            ESP_LOGI("DEBUG","MOVE DONE - %s", move);
            fromdone = 0;
            todone = 0;
            toLet = 'x';
            toNumb = 10;
            whiteturn = whiteturn ^ 1;
        }
        if (esp_timer_get_time()-prev_time >= PRINT_BOARD_INTERVAL_US) {
            prev_time = esp_timer_get_time();
            print_board();
            printf("\n");
            printf("\n");
            printf("\n");
        }
    }
}

