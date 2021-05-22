# ChessMove


## main.c

### Important variables:

`SERVERIP` - The IP of the computer where the webserver runs.

`BOARDID` - ID for this board.

### Functions:

`startGame()` - Creates the message to send to the server to start a game and sends it. 

`buildMove()` - Builds a move based on the changes on the chess board.

`sendMove()` - Sends the previously built move to the server.

`app_main` - Calls configure_wifi() and configurespi() functions and then starts the game. Loops to read buttons and send the data to the server.



## wifi_config.c

### Important variables:

`ESP_WIFI_SSID` - WiFi name.

`ESP_WIFI_PASS` - WiFi password.

### Functions:

`configure_wifi()` - Opens a connection with the defined WiFi name and password to a network.

`sendHttpRequest(char* urlString, char* move, bool begin)` - Sends HTTP POST request to the server to either start a game or send a move.

1) urlString - Server API endpoint.
2) move - Message to send to the server endpoint.
3) begin - To indicate wheter to send the start game or move command.


## spi_config.c

### Important variables:

All the ESP32 pin connections and commands to send to the button microcontrollers are defined here.

`button_matrix` - 8x8 list that holds the values 0 or 1 to indicate whether a chess piece is on that square or not.

### Functions:

