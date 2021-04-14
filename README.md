# ChessMove

ESP-IDF setup:

1) 
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 

2) 
sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10 && alias pip=pip3

3) 
mkdir -p ~/esp 
cd ~/esp 
git clone –recursive https://github.com/espressif/esp-idf.git 
cd /esp-idf 
./install.sh 

4)
mkdir /kaust/ 
cd /kaust/ 
cp -r ~/esp/esp-idf/examples/get-started/hello_world . 


In ESP_IDF_LOCATION/components/driver/spi_master.c file on line 145 "#define NO_DEV 5" should be added and "spi_bus_add_device" function should be replaced with the one in the spi_master.c file on this repository.

On the ChessMove virtual machine this is already done.

To run on the ChessMove virtual machine:

1) clone the repository to the machine if not already present.
2) delete the spi_master.c file
3) run ". /home/chessmove/esp_idf/export.sh" in the repository folder.
4) run "idf.py -p 'port' -b 115200 flash monitor" in the repository folder.
  
Replace 'port' with actual port, for an example "/dev/ttyUSB0".

To be able to use Virtual Studio Code for building, flashing and monitoring follow the guide: https://github.com/espressif/vscode-esp-idf-extension.
