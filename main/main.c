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

#define btoa(x) ((x)?"true":"false") // for debugging purposes, remove if not needed.

#define number_of_buttons 32 //this many chess pieces have to be on the board before the "real" game starts. If just testing the code, you can make it like 4-6 or something
// so you dont have to put 32 chess pieces on the board every time you restart.

typedef struct { // holds info about every chess piece on the board. Maybe chesspiece.dead should be added.
    bool white;
    bool black;
    char letpos;
    int numpos;
} chesspiece;

#ifdef CONFIG_IDF_TARGET_ESP32
#define CHIP_NAME "ESP32"
#endif

#define PRINT_BOARD_INTERVAL_US 1000000*2 // how often the chess board is printed out.

#define SERVERIP "" // the public ip of the computer the server is located on.
#define BOARDID 1

uint8_t numb_of_chesspieces = 32; //for creating the chesspiece array, dont change this.

char sendFromLet = 'x'; // the values actually sent to the server. We don't use the values straight from the spi_config.c file like toLet and toNumb because
int sendFromNumb = 10; // when the chesspiece is taken, if we do it like this, it doesnt matter which chess piece is picked up first, this will always let us 
// send the correct command to the server.


void startGame() { // sends the command to the server to start a game. Format from server API
    char* starturl;
    asprintf(&starturl, "http://%s:24377/api/start/%d", SERVERIP, BOARDID); 
    sendHttpRequest(starturl, starturl, true);
}
char* buildMove(char fromLet, int fromNumb, char toLet, int toNumb) { // builds the move to be sent to the server. Format from server API
    char* move;
    asprintf(&move, "{\"boardId\":\"%d\",\"from\":\"%C%d\",\"to\":\"%C%d\"}", BOARDID, fromLet, fromNumb, toLet, toNumb);
    return move;
}
void sendMove(char* move) { // sends a move to the server.
    char* moveurl;
    asprintf(&moveurl, "http://%s:24377/api/move", SERVERIP);
    sendHttpRequest(moveurl, move, false);
}
void printboard(void) { // just for debugging purposes.
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

void fillChessPiecesArray(chesspiece *chesspiece_arr) { // fills the entire array of 32 chesspieces, manually too much work.
    int column = 0;
    for (int i = 0; i < numb_of_chesspieces; ++i) {
        if (i < 16) { // first 16 are white pieces.
            chesspiece_arr[i].white = true;
            chesspiece_arr[i].black = false;
            chesspiece_arr[i].letpos = letterFromColumn(column); // we keep the position so we can check when move is done if there was actually a piece there or a bug.
            if (i < 8) { // also we check to make sure when white turn is that white piece moves, even if black piece is moved as well.
                chesspiece_arr[i].numpos = 1; // white pieces on a1, b1, c1 etc.
            }
            else {
                chesspiece_arr[i].numpos = 2; // white pieces on a2, b2, c2 etc.
            }
        }
        else { // next 16 are black pieces
            chesspiece_arr[i].black = true;
            chesspiece_arr[i].white = false;
            chesspiece_arr[i].letpos = letterFromColumn(column);
            if (i < 24) {
                chesspiece_arr[i].numpos = 7; // black pieces on a7, b7, c7 etc.
            }
            else {
                chesspiece_arr[i].numpos = 8; // black pieces on a8, b8, c8 etc.
            }
        }
        column+=1;
        if (column == 8) { // we only got 8 columns, so everytime we reach that number, restart.
            column = 0;
        }
    }
}

/*Double checks if when a piece moves, there is a real chess piece there. 
Also if an opponents button is taken, it doesn't matter which of the 2 buttons move first.
this function makes sure that the piece whose turn it is, their position is sent to the server with the sendFromLet and sendFromNumb variables.
*/
bool checkFromPos(chesspiece *chesspiece_arr, char fromLet, char fromNumb, bool whiteturn) { 
    for (int i = 0; i < numb_of_chesspieces; ++i) {
        if (whiteturn) {
            if (chesspiece_arr[i].white) {
                if (fromLet == chesspiece_arr[i].letpos && fromNumb == chesspiece_arr[i].numpos) {
                    sendFromLet = fromLet;
                    sendFromNumb = fromNumb;
                    return true; // i used the true before for another check, but it is not needed right now anymore. Kept it so if it is needed again, it can be used.
                }
            }
        }
        else {
            if (chesspiece_arr[i].black) {
                if (fromLet == chesspiece_arr[i].letpos && fromNumb == chesspiece_arr[i].numpos) {
                    sendFromLet = fromLet;
                    sendFromNumb = fromNumb;
                    return true;
                }
            }
        }
    }
    return false;
}

/*
Function works the same way as the above one, but instead of making sure the server gets sent the right From position, this changes 
the chesspiece position to be kept up to date.
Could possibly be integrated with the above function, but this one is called out once the move is actually done, the above one is called out earlier.
*/
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
/*
This function counts how many chess pieces are actually on the board. This is there so when the game first begins, the chesspieces are placed on the board, 
basically the main loop is stuck counting the buttons until there are as many chess pieces on the board as there are defined in the number_of_buttons variable.

Mostly the point of this is that when this doesn't exist, putting pieces on the board messes with the fromLet and fromNumb variables that are used to send information
to the server. It might mess up the first move done on the board. 
Also putting chesspieces on the board before starting the code will still mess with those variables. So i found this is the best way to keep the beginning of the game 
clean and without bugs.
*/
int countButtons(void) {
    int check=0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) { 
            if (button_matrix[i][j] == 1) {
                check++;
            }
        }
    }
    return check;
}
void app_main(void)
{   

    configure_wifi(); // starts a connection to the defined WiFi network. The WiFi name and password are defined in the wifi_config.c file.
    // i wanted to define them in main.c and give them as function variables, but i managed to break the code with that so i skipped it.
    vTaskDelay(1000/ portTICK_PERIOD_MS);
    bool whiteturn = true; // whiteturn = true means it's white turn, false means it's black turn.
    char* move; // json message sent to the server is kept here.

    static const char *SPI_TAG = "MAIN";
    uint8_t numb_of_devices = 8; // 8 button microcontrollers.
    
    device device_arr[numb_of_devices]; // i used to do some checks in this code with the device array. Probably can be moved to spi_config file if necessary.
    
    
    chesspiece chesspiece_arr[numb_of_chesspieces];
    
    fillChessPiecesArray(chesspiece_arr);

    configure_spi(numb_of_devices, device_arr); // configures all SPI connections to the button microcontrollers etc.
    int64_t prev_time = 0;
    vTaskDelay(1000/ portTICK_PERIOD_MS);
    printf("START"); 
    printf("\n");
    startGame();
    int counter = 0;
    bool gamestarted = false;

    while (1) {
        if (gamestarted) {
            check_buttons(device_arr);
            vTaskDelay(100/ portTICK_PERIOD_MS); // can probably be removed if there are REALLY fast players that make moves faster than 100ms.
            if (fromdone && !todone) {
                if (!checkFromPos(chesspiece_arr, fromLet, fromNumb, whiteturn)) { // this function has to be called out, even if the true/false response isn't used.
                    printf("wrong");
                    printf("\n");
                }
            }
            if (fromdone && todone && (toLet != 'x' && toNumb != 10) && (fromLet != 'x' && fromNumb != 10) && (sendFromLet != 'x' && sendFromNumb != 10)) {
                printf("whiteorblack: ");
                printf(btoa(whiteturn));
                printf("\n");
                move = buildMove(sendFromLet, sendFromNumb, toLet, toNumb);
                sendMove(move);
                changeButtonPos(chesspiece_arr, sendFromLet, sendFromNumb, whiteturn, toLet, toNumb);
                ESP_LOGI("DEBUG","MOVE DONE - %s", move);
                fromdone = 0;
                todone = 0;
                toLet = 'x';
                toNumb = 10;
                fromLet = 'x';
                fromNumb = 10;
                sendFromLet = 'x';
                sendFromNumb = 10;
                whiteturn = whiteturn ^ 1; // makes the variable opposite of what it is. So if it's true, this changes it to false. If it is false, this changes it to true.
            }
            if (esp_timer_get_time()-prev_time >= PRINT_BOARD_INTERVAL_US) {
                prev_time = esp_timer_get_time();
                print_board();
                printf("\n");
                printf("\n");
                printf("\n");
            }
        }
        else {
            counter = countButtons();
            ESP_LOGI("DEBUG", "COUNTER: %d", counter);
            if (counter == number_of_buttons) {
                gamestarted = true;
                fromLet = 'x';
                fromNumb = 10;
            }
            check_buttons2(device_arr); // 2nd function, different than check_buttons(device_arr) by not changing the fromLet and fromNum variables.
        }
    }
}

