# ChessMove

### ESP-IDF setup:

1) 
```
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 
```
2)
```
sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10 && alias pip=pip3
```
3) 
```
mkdir -p ~/esp
cd ~/esp 
git clone –recursive https://github.com/espressif/esp-idf.git 
cd /esp-idf 
./install.sh 
```

### Starting the code:
1) 
```
git clone https://github.com/UkuSoome/ChessMove
cd ChessMove
```
2) Use inside the ChessMove directory. Has to be used so the idf.py program can be called out there.
```
. ~/{esp-idf-location}/export.sh
```
3) Use your USB port. Baudrate has to be 115200. Flash command builds the code and flashes it to the ESP. Monitor opens the code output monitor. ctrl + ] to exit. More useful shortcuts: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-monitor.html
```
idf.py -p ttyUSB0 -b 115200 flash monitor
```
4) Don't forget to configure the correct SERVERIP in main.c file and WiFi name and password in wifi_config.c file. 
NB! You might have to turn off the firewall on the computer the server runs on so the board can connect to the WiFi/server.



### spi_config.c file
NB! Doesn't have to be done in newer esp-idf versions. If the code gives errors about the last 2 button microcontrollers, then you might have to do this.

In ESP_IDF_LOCATION/components/driver/spi_master.c file on line 145 "#define NO_DEV 5" should be added and "spi_bus_add_device" function should be replaced with the one in the spi_master.c file on this repository.
