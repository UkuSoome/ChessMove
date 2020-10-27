# ChessMove

spi_master.c file should replace the file with same name in ESP_IDF_LOCATION/components/driver/spi_master.c and then be deleted from the folder.

OR

on line 145 of the file, #define NO_DEV 5 can be added and spi_bus_add_device function should be replaced with the one in this file.
