# ChessMove



---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



## File main.c

### Important variables:

`SERVERIP` - The IP of the computer where the webserver runs.

`BOARDID` - ID for this board.

### Functions:

`startGame()` - Creates the message to send to the server to start a game and sends it. 

`buildMove()` - Builds a move based on the changes on the chess board.

`sendMove()` - Sends the previously built move to the server.

`app_main` - Calls configure_wifi() and configurespi() functions and then starts the game. Loops to read buttons and send the data to the server.



---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



## File wifi_config.c

### Important variables:

`ESP_WIFI_SSID` - WiFi name.

`ESP_WIFI_PASS` - WiFi password.

### Functions:

`configure_wifi()` - Opens a connection with the defined WiFi name and password to a network.

`sendHttpRequest(char* urlString, char* move, bool begin)` - Sends HTTP POST request to the server to either start a game or send a move.

1) urlString - Server API endpoint.
2) move - Message to send to the server endpoint.
3) begin - To indicate wheter to send the start game or move command.



---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



## File spi_config.c

### Important variables:

All the ESP32 pin connections and commands to send to the button microcontrollers are defined here.

`button_matrix` - 8x8 list that holds the values 0 or 1 to indicate whether a chess piece is on that square or not.

`fromNumb` - Defined in the spi_config.h file. Holds the row number of where the move was made from. Used to build a message to be sent to the server.

`toNumb` - Defined in the spi_config.h file. Holds the row number of where the move is made to. Used to build a message to be sent to the server.

`fromLet` - Defined in the spi_config.h file. Holds the column letter of where the move was made from. Used to build a message to be sent to the server.

`toLet` - Defined in the spi_config.h file. Holds the column number of where the move is made to. Used to build a message to be sent to the server.

### Structs:

`device` - Defined in the spi_config.h file. Holds the information about button microcontrollers. It holds the name, CS pin, device handle, SPI bus host and row index. An array of devices is initialized in the main.c file and information is filled in spi_config.c file.

`MB_SPI_busConfigStruct` - MB and SB structs. They configure the SPI bus pin numbers.

`MB_QT1_devConfigStruct` - One for every button microcontroller to configure frequency and set the callback functions.

### Functions:

`QT_spi_pre_transfer_callback()` - Pre and post transfer callback funcitons to set the CS pin as 1 or 0 to start or stop the transfer.

`MU_1_2_isr_handler()` - 4 functions for every 2 rows of buttons. They set a flag to indicate that a change has happened in one of those 2 rows of buttons.

`initBothBoardsBus()` - Initializes both of the SPI buses the ESP32 has.

`addDeviceToBus(hostid, devconfig, devicehandle)` - Adds all 8 button microcontrollers to a SPI bus.

1) hostid - Which SPI bus to add the device handle to. HSPI_HOST and VSPI_HOST are the hosts defined by ESP-IDF.
2) devconfig - MB_QT0_devConfigStruct defined in ### Structs
3) devicehandle - MB_QT0_SPI for an example. Every button microcontroller has their own device handle.

`QT_handle_to_string()` - Mostly for debugging purposes. All of the button microcontrollers have a device handle and this function returns the device name as a string based on them.

`QT_reset(qt_device)` - Used to send the command to reset button microcontrollers.

`QT_setup_register(qt_device, QT_register, command))` - Used to send the commands to configure button microcontrollers.

1) qt_device -  One device from the device array.
2) qt_register - Which register to send the command to. Value defined in the button microcontroller datasheet.
3) command - Which command to send to that register. Value defined in the button microncontroller datasheet.

`QT_setup(qt_device)` - Calls out QT_setup_register() with different hex values to configure all the necessary values for button microcontrollers.

`QT_report_request(qt_device, command, rec_length)` - Used to read data from button microcontrollers about the status of all buttons.

1) qt_device - One device from the device array. 
2) command - Hex value to ask for data of all buttons.
3) rec_length - Since the button microcontrollers can have 11 keys attached the answer is sent in 2 bytes. This defines how many bytes to read. This hould always be 2 since the first byte only holds 3 bits and the second byte holds 8. We use the 2nd byte.

`QT_device_status()` - Not used, but can be used for more data about button microcontrollers. Data like if any of the keys are in detect, if any buttons are in error.

`letterFromColumn(column)` - Used to get the letter corresponding to a column number.

`QT_check_buttons_and_update_board(qt_device)` - Used to get data from button microcontrollers and update the button_matrix list and fill the values of fromLet, fromNumb, toLet and toNumb.

`print_board()` - Mostly for debugging purposes. Prints the chess board with column letters. X indicates a chess piece is on a button and O indicates there is none.

`check_buttons(device_arr)` - Used in the main.c file main loop. Calls the QT_check_buttons_and_update_board() function to read data of a certain button microcontroller based on the QT_MU_1_2_INT_FLAG flags.

`configure_spi(numb_of_devices, device_arr)` - Function called out in main.c file to configure SPI with one function. Calls the configurement functions defined above.
1) numb_of_devices - 8 button microcontrollers.
2) device_arr - Defined in the ##Structs section.
