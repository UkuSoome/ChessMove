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

void fillChessPiecesArray() {
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
void app_main(void)
{   

    configure_wifi();
    vTaskDelay(1000/ portTICK_PERIOD_MS);

    char* move;

    static const char *SPI_TAG = "MAIN";
    uint8_t numb_of_devices = 8;
    
    device device_arr[numb_of_devices];

    uint8_t numb_of_chesspieces = 32;
    chesspiece chesspiece_arr[numb_of_chesspieces];
    
    fillChessPiecesArray();

    for (int i = 0; i < numb_of_chesspieces; ++i) {
        char* move;
        asprintf(&move, "whitepiece: $d, blackpiece: $d, position $C$d\n", chesspiece_arr[i].white, chesspiece_arr[i].black, chesspiece_arr[i].letpos, chesspiece_arr[i].numpos);
        printf(move);
    }

    configure_spi(numb_of_devices, device_arr);
    uint16_t print_counter = 0;
    int64_t prev_time = 0;
    char pos1;
    char pos2;
    int check;
    vTaskDelay(1000/ portTICK_PERIOD_MS);
    printf("START"); 
    printf("\n");
    startGame();
    while (1) {

        check_buttons(device_arr);
        vTaskDelay(100/ portTICK_PERIOD_MS);    // Wait at least 100ms
        //compareBoards();
        if (checkTo == 2) {
            move = buildMove(fromLet, fromNumb, toLet, toNumb);
            sendMove(move);
            ESP_LOGI("DEBUG","MOVE DONE - %s", move);
            checkTo = 0;
            //fromLet = 'x';
            //toLet = 'x';
            //fromNumb = 0;
            //toNumb = 0;
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

