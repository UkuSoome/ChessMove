# ChessMove

On line 145 of ESP_IDF_LOCATION/components/driver/spi_master.c "#define NO_DEV 5" should be added and "spi_bus_add_device" function should be replaced with the one in the spi_master.c file on this repositry.

On the chessMove virtual machine this is already done.

To run on the chessMove virtual machine:

1) clone the repositry to the machine if not already present.
2) remove/move the spi_master.c file
3) run ". /home/chessmove/esp_idf/export.sh" command in the repositry folder.
4) run "idf.py -p <port> -b 115200 flash monitor 
  
Replace <port> with actual port, for an example "/dev/ttyUSB0
