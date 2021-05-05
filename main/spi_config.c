#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <stdlib.h>
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "spi_config.h"


#define SPI_MU_CS0_PIN 4
#define SPI_MU_CS1_PIN 5
#define SPI_MU_CS2_PIN 15
#define SPI_MU_CS3_PIN 27
#define SPI_MU_SCK 14
#define SPI_MU_MISO 12
#define SPI_MU_MOSI 13
#define SPI_SU_CS0_PIN 21
#define SPI_SU_CS1_PIN 22
#define SPI_SU_CS2_PIN 25
#define SPI_SU_CS3_PIN 26
#define SPI_SU_SCK 18
#define SPI_SU_MISO 19
#define SPI_SU_MOSI 23
#define SPI_MU_1_2_nCHANGE 34
#define SPI_MU_3_4_nCHANGE 0
#define SPI_SU_1_2_nCHANGE 35
#define SPI_SU_3_4_nCHANGE 32

#define QT_SPI_FREQ 400000
#define QT_SPI_MODE 3

#define BUTTON_MATRIX_COL_SIZE 8
#define BUTTON_MATRIX_ROW_SIZE 8

#define CONST_QT_ID 0x57
#define CMD_QT_RESET 0x04
#define CMD_DISABLE_KEY 0x00
#define CONST_QT_ANS 0x55
#define QT_11KEY_MODE_COMMAND 0xF2 // Selects 11 key mode
#define MAX_TRANS_DATA_SIZE 42

#define nCHNG_INT_PIN_SEL  ((1ULL<<SPI_MU_1_2_nCHANGE) | (1ULL<<SPI_MU_3_4_nCHANGE)| (1ULL<<SPI_SU_1_2_nCHANGE)| (1ULL<<SPI_SU_3_4_nCHANGE))
#define ESP_INTR_FLAG_DEFAULT 0

#define REG_DEVICE_ID 0xC9
#define REG_DEVICE_STATUS 0xC2
#define REG_ALL_KEYS 0xC1
#define REG_FIRST_KEY 0xC0
#define REG_KEY_SIGNAl 0x20
#define REG_KEY_REF 0x40
#define REG_KEY_STAT 0x80
#define REG_DEVICE_MODE 0x90
#define REG_KEY8_NTHR 0xAB
#define REG_KEY9_NTHR 0xAC
#define REG_KEY10_NTHR 0xAD
#define REG_RETURN_SETUPS 0xC8
#define REG_LAST_COMMAND 0xC7
#define REG_ERROR_KEYS 0xC5


esp_err_t ret;
static const char *SPI_TAG = "SPICONF";
/*spi_device_handle_t MB_QT0_SPI;
spi_device_handle_t MB_QT1_SPI;
spi_device_handle_t MB_QT2_SPI;
spi_device_handle_t MB_QT3_SPI;
spi_device_handle_t SB_QT0_SPI;
spi_device_handle_t SB_QT1_SPI;
spi_device_handle_t SB_QT2_SPI;
spi_device_handle_t SB_QT3_SPI;*/

/*typedef struct {
    const char* name;
    int cs_pin;
    spi_device_handle_t handle;
    uint8_t host;
    int row_index;
} device;*/

const char* QT_handle_to_string(device);

int current_cs_pin;
int button_matrix[BUTTON_MATRIX_ROW_SIZE][BUTTON_MATRIX_COL_SIZE] = {};
uint8_t global_rx_buffer[MAX_TRANS_DATA_SIZE] = {0};


void QT_spi_pre_transfer_callback(spi_transaction_t *t) {
    //int SPI_CS=(int)t->user;s
    //gpio_set_level(SPI_CS, 0);
    gpio_set_level(current_cs_pin,0);
}

void QT_spi_post_transfer_callback(spi_transaction_t *t) {
    //int SPI_CS=(int)t->user;
    //gpio_set_level(SPI_CS, 1);
    gpio_set_level(current_cs_pin,1);
}
static void IRAM_ATTR MU_1_2_isr_handler(void* arg)
{
    // QT MU 1 and 2 interrupt service routine
    // Double check if IO matches and raise flags
    uint32_t gpio_num = (uint32_t) arg;
    if (gpio_num == SPI_MU_1_2_nCHANGE)
        QT_MU_1_2_INT_FLAG = true;
    else
        QT_INT_ERR_FLAG = true; 
}

static void IRAM_ATTR MU_3_4_isr_handler(void* arg)
{
    // QT MU 3 and 4 interrupt service routine
    // Double check if IO matches and raise flags
    uint32_t gpio_num = (uint32_t) arg;
    if (gpio_num == SPI_MU_3_4_nCHANGE)
        QT_MU_3_4_INT_FLAG = true;
    else
        QT_INT_ERR_FLAG = true; 
}

static void IRAM_ATTR SU_1_2_isr_handler(void* arg)
{
    // QT SU 1 and 2 interrupt service routine
    // Double check if IO matches and raise flags
    uint32_t gpio_num = (uint32_t) arg;
    if (gpio_num == SPI_SU_1_2_nCHANGE)
        QT_SU_1_2_INT_FLAG = true;
    else
        QT_INT_ERR_FLAG = true; 
}
static void IRAM_ATTR SU_3_4_isr_handler(void* arg)
{
    // QT SU 3 and 4 interrupt service routine
    // Double check if IO matches and raise flags
    uint32_t gpio_num = (uint32_t) arg;
    if (gpio_num == SPI_SU_3_4_nCHANGE)
        QT_SU_3_4_INT_FLAG = true;
    else
        QT_INT_ERR_FLAG = true; 
}
/*begin structs
Structs for configuring SPI bus ports, slave devices.
MB = main board (with ESP32)
SB = secondary Board (without ESP32)
*/
spi_bus_config_t MB_SPI_busConfigStruct={
    .miso_io_num=SPI_MU_MISO,
    .mosi_io_num=SPI_MU_MOSI,
    .sclk_io_num=SPI_MU_SCK,
    .quadwp_io_num=-1,
    .quadhd_io_num=-1,
};
spi_bus_config_t SB_SPI_busConfigStruct={
    .miso_io_num=SPI_SU_MISO,
    .mosi_io_num=SPI_SU_MOSI,
    .sclk_io_num=SPI_SU_SCK,
    .quadwp_io_num=-1,
    .quadhd_io_num=-1,
};
spi_device_interface_config_t MB_QT0_devConfigStruct={
    .clock_speed_hz=QT_SPI_FREQ,
    .mode=QT_SPI_MODE,
    .spics_io_num= -1, 
    .queue_size=7,
    .pre_cb=QT_spi_pre_transfer_callback,
    .post_cb=QT_spi_post_transfer_callback,
};
spi_device_interface_config_t MB_QT1_devConfigStruct={
    .clock_speed_hz=QT_SPI_FREQ,
    .mode=QT_SPI_MODE,
    .spics_io_num= -1, 
    .queue_size=7,
    .pre_cb=QT_spi_pre_transfer_callback,
    .post_cb=QT_spi_post_transfer_callback,
};
spi_device_interface_config_t MB_QT2_devConfigStruct={
    .clock_speed_hz=QT_SPI_FREQ,
    .mode=QT_SPI_MODE,
    .spics_io_num= -1, 
    .queue_size=7,
    .pre_cb=QT_spi_pre_transfer_callback,
    .post_cb=QT_spi_post_transfer_callback,
};
spi_device_interface_config_t MB_QT3_devConfigStruct={
    .clock_speed_hz=QT_SPI_FREQ,
    .mode=QT_SPI_MODE,
    .spics_io_num= -1, 
    .queue_size=7,
    .pre_cb=QT_spi_pre_transfer_callback,
    .post_cb=QT_spi_post_transfer_callback,
};

spi_device_interface_config_t SB_QT0_devConfigStruct={
    .clock_speed_hz=QT_SPI_FREQ,
    .mode=QT_SPI_MODE,
    .spics_io_num= -1, 
    .queue_size=7,
    .pre_cb=QT_spi_pre_transfer_callback,
    .post_cb=QT_spi_post_transfer_callback,
};
spi_device_interface_config_t SB_QT1_devConfigStruct={
    .clock_speed_hz=QT_SPI_FREQ,
    .mode=QT_SPI_MODE,
    .spics_io_num= -1, 
    .queue_size=7,
    .pre_cb=QT_spi_pre_transfer_callback,
    .post_cb=QT_spi_post_transfer_callback,
};
spi_device_interface_config_t SB_QT2_devConfigStruct={
    .clock_speed_hz=QT_SPI_FREQ,
    .mode=QT_SPI_MODE,
    .spics_io_num= -1, 
    .queue_size=7,
    .pre_cb=QT_spi_pre_transfer_callback,
    .post_cb=QT_spi_post_transfer_callback,
};
spi_device_interface_config_t SB_QT3_devConfigStruct={
    .clock_speed_hz=QT_SPI_FREQ,
    .mode=QT_SPI_MODE,
    .spics_io_num= -1, 
    .queue_size=7,
    .pre_cb=QT_spi_pre_transfer_callback,
    .post_cb=QT_spi_post_transfer_callback,
};

/*end structs
Structs for configuring SPI bus ports, slave devices.
*/

void initBothBoardsBus(void) {
    // main board with ESP
    ret=spi_bus_initialize(HSPI_HOST, &MB_SPI_busConfigStruct, 1);
    ESP_LOGI(SPI_TAG, "Main board SPI initialization return: %d", ret);
    ESP_ERROR_CHECK(ret);

    // 2nd board without ESP
    ret=spi_bus_initialize(VSPI_HOST, &SB_SPI_busConfigStruct, 2);
    ESP_LOGI(SPI_TAG, "Secondary board SPI initialization return: %d", ret);
    ESP_ERROR_CHECK(ret);
}
void addDeviceToBus(spi_host_device_t hostid, spi_device_interface_config_t *devconfig, spi_device_handle_t *devicehandle) {
    ret=spi_bus_add_device(hostid, devconfig, devicehandle);
    ESP_LOGI(SPI_TAG, "Main Board QT device add return: %d", ret);
    ESP_ERROR_CHECK(ret);
}
const char* QT_handle_to_string(device dev)
{
    //static char* QT_devices[] = {"QT_MU_1","QT_MU_2","QT_MU_3","QT_MU_4","QT_SU_1","QT_SU_2","QT_SU_3","QT_SU_4"};
    if (dev.handle == MB_QT0_SPI)
    {
        return("MB_QT0_SPI");
    }
    else if (dev.handle == MB_QT1_SPI)
    {
        return("MB_QT1_SPI");
    }
    else if (dev.handle == MB_QT2_SPI)
    {
        return("MB_QT2_SPI");
    }
    else if (dev.handle == MB_QT3_SPI)
    {
        return("MB_QT3_SPI");
    }
    else if (dev.handle == SB_QT0_SPI)
    {
        return("SB_QT0_SPI");
    }
    else if (dev.handle == SB_QT1_SPI)
    {
        return("SB_QT1_SPI");
    }
    else if (dev.handle == SB_QT2_SPI)
    {
        return("SB_QT2_SPI");
    }
    else if (dev.handle == SB_QT3_SPI)
    {
        return("SB_QT3_SPI");
    }
    else
    {
        return("ERROR - NON QT");
    }  
}

void QT_control_command(device qt_device, uint8_t command)
{
    ESP_LOGI(SPI_TAG, "Control_command func, command: 0x%x, devic: %s", command, qt_device.name);
    esp_err_t ret;                          // Variable for error handling
    static const char *SPI_TAG = "QT_CONTROL_COMMAND";
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));       // Zero out the transaction
    //trans.user=dev.cs_pin;
    current_cs_pin = qt_device.cs_pin;
    //trans.user=(void*)qt_device.cs_pin;

    trans.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
    trans.tx_data[0] = command;
    trans.length=8;                         // Command is 8 bits  
    ret=spi_device_polling_transmit(qt_device.handle, &trans);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
    vTaskDelay(2/ portTICK_PERIOD_MS);
    if(trans.rx_data[0] != CONST_QT_ANS){
        ESP_LOGE(SPI_TAG,"Oops! Something went wrong - command response: 0x%x, device: %s", trans.rx_data[0], qt_device.name);//QT_handle_to_string(device_handle));
    }
    vTaskDelay(200/ portTICK_PERIOD_MS);
}
void QT_reset(device qt_device){
    QT_control_command(qt_device, CMD_QT_RESET);
}
void QT_setup_register(device qt_device, uint8_t QT_register, uint8_t command)
{
    esp_err_t ret;                          // Variable for error handling
    static const char *SPI_TAG = "QT_CONTROL_COMMAND";
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));       // Zero out the transaction
    // Store specific QT device CS pin number as user variable for pre/post CB
    //trans.user=dev.cs_pin;
    current_cs_pin = qt_device.cs_pin;
    //trans.user=(void*)qt_device.cs_pin;

    trans.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
    trans.tx_data[0] = QT_register;
    trans.length=8;                         // Command is 8 bits  
    ret=spi_device_polling_transmit(qt_device.handle, &trans);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
    vTaskDelay(2/ portTICK_PERIOD_MS);
    if(trans.rx_data[0] != CONST_QT_ANS){
        ESP_LOGE(SPI_TAG,"Oops! 1 Something went wrong - command response: 0x%x - command %x \n", trans.rx_data[0], command);
    }
    trans.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
    trans.tx_data[0] = command;
    trans.length=8;                         // Command is 8 bits  
    ret=spi_device_polling_transmit(qt_device.handle, &trans);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
    vTaskDelay(2/ portTICK_PERIOD_MS);
    if(trans.rx_data[0] != QT_register){
        ESP_LOGE(SPI_TAG,"Oops! 2 Something went wrong - command response: 0x%x - register %x \n", trans.rx_data[0], QT_register);
    }
    vTaskDelay(200/ portTICK_PERIOD_MS);
}
void QT_setup(device qt_device){
    static const char *SPI_TAG = "QT_SETUP";
    ESP_LOGI(SPI_TAG, "Setup of %s", qt_device.name);
    QT_setup_register(qt_device, REG_DEVICE_MODE, QT_11KEY_MODE_COMMAND);
    QT_setup_register(qt_device, REG_KEY8_NTHR, CMD_DISABLE_KEY);
    QT_setup_register(qt_device, REG_KEY9_NTHR, CMD_DISABLE_KEY);
    QT_setup_register(qt_device, REG_KEY10_NTHR, CMD_DISABLE_KEY);
    QT_setup_register(qt_device, 0x96, 0x32);
    QT_control_command(qt_device, 0x03);
    //QT_control_command(qt_device, 0x0A);
}
void QT_report_request(device qt_device, uint8_t command, uint8_t rec_length)
{
    esp_err_t ret;                          // Variable for error handling
    static const char *SPI_TAG = "QT_REPORT_REQUEST";
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));       // Zero out the transactio

    //trans.user=dev.cs_pin;
    current_cs_pin = qt_device.cs_pin;
    //trans.user=(void*)qt_device.cs_pin;
    // Control - send single byte, 
    trans.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
    trans.tx_data[0] = command;
    trans.length=8;                         // Command is 8 bits  
    ret=spi_device_polling_transmit(qt_device.handle, &trans);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
    vTaskDelay(2/ portTICK_PERIOD_MS);
    if(trans.rx_data[0] != CONST_QT_ANS){
        ESP_LOGE(SPI_TAG,"Oops! Something went wrong - command response: 0x%x, device: %s", trans.rx_data[0], qt_device.name);
    }

    // Receiving data from QT register
    for (int i = 0; i < rec_length; ++i){
        trans.length=8;            
        trans.tx_data[0] = 0x00;     //The data is the cmd itself
        ret=spi_device_polling_transmit(qt_device.handle, &trans);  //Transmit!
        assert(ret==ESP_OK);            //Should have had no issues.
        global_rx_buffer[i]=trans.rx_data[0];
        ESP_LOGI(SPI_TAG, "data recieved: %x with command %x", trans.rx_data[0], command);
       // ESP_LOGI(SPI_TAG, "Data packet #%i: %x ", i+1,trans.rx_data[0]);
        vTaskDelay(2/ portTICK_PERIOD_MS);
    }
}
void QT_device_status(device qt_device)
{
    static const char *SPI_TAG = "QT_STATUS_CHECK";
    ESP_LOGI(SPI_TAG, "Getting status data for device: %s", qt_device.name);
    QT_report_request(qt_device, REG_DEVICE_STATUS, 1);
    ESP_LOGI(SPI_TAG, "Status data: %x \n" 
        "Key detect: %u \n" 
        "Cycle time: %u \n" 
        "Error: %u \n" 
        "Change: %u \n" 
        "EEPROM: %u \n" 
        "RESET: %u \n" 
        "Guard: %u \n",
        ((global_rx_buffer[0])),
        ((global_rx_buffer[0] & 0x40)>>6),
        ((global_rx_buffer[0] & 0x20)>>5),
        ((global_rx_buffer[0] & 0x10)>>4),
        ((global_rx_buffer[0] & 0x08)>>3),
        ((global_rx_buffer[0] & 0x04)>>2),
        ((global_rx_buffer[0] & 0x02)>>1),
        (global_rx_buffer[0] & 0x01));
    if (((global_rx_buffer[0] & 0x10)>>4)==1)
    {
        ESP_LOGE(SPI_TAG, "KEY ERROR DETECTED!");
        QT_report_request(qt_device, REG_ERROR_KEYS, 2);
        ESP_LOGI(SPI_TAG, "Keys in error: \n"
            "Key 11: %x \n"
            "Key 10: %x \n"
            "Key 09: %x \n"
            "Key 08: %x \n"
            "Key 07: %x \n"
            "Key 06: %x \n"
            "Key 05: %x \n"
            "Key 04: %x \n"
            "Key 03: %x \n"
            "Key 02: %x \n"
            "Key 01: %x \n", 
            ((global_rx_buffer[0] & 0x04)>>2),
            ((global_rx_buffer[0] & 0x02)>>1),
            (global_rx_buffer[0] & 0x01),
            ((global_rx_buffer[1] & 0x80)>>7),
            ((global_rx_buffer[1] & 0x40)>>6),
            ((global_rx_buffer[1] & 0x20)>>5),
            ((global_rx_buffer[1] & 0x10)>>4),
            ((global_rx_buffer[1] & 0x08)>>3),
            ((global_rx_buffer[1] & 0x04)>>2),
            ((global_rx_buffer[1] & 0x02)>>1),
            (global_rx_buffer[1] & 0x01));
    }
}
char letterFromRow(int row) {
    if (row==0) return 'a';
    if (row==1) return 'b';
    if (row==2) return 'c';
    if (row==3) return 'd';
    if (row==4) return 'e';
    if (row==5) return 'f';
    if (row==6) return 'g';
    if (row==7) return 'h';
    return 'a';
}
void QT_check_buttons_and_update_board(device qt_device) {
    static const char *SPI_TAG = "QT_BUTTON_CHECK";
    uint8_t button_row_data = 0; 
    QT_report_request(qt_device, 0x81, 1);
    bool a = (global_rx_buffer[1] & (1 << 7);
    ESP_LOGI(SPI_TAG, "esimene bit on : %d", a);
    QT_report_request(qt_device, 0x82, 1);
    a = (global_rx_buffer[1] & (1 << 7);
    ESP_LOGI(SPI_TAG, "esimene bit on : %d", a);
    QT_report_request(qt_device, 0x83, 1);
    a = (global_rx_buffer[1] & (1 << 7);
    ESP_LOGI(SPI_TAG, "esimene bit on : %d", a);
   /* QT_report_request(qt_device, REG_ALL_KEYS, 2);
    button_row_data = global_rx_buffer[1];
    ESP_LOGI(SPI_TAG, "Button row data: %x", button_row_data);
    for (int i = 0; i < BUTTON_MATRIX_COL_SIZE; ++i) {
        if ((button_row_data & (0x01<<i))>>i) {
            if (button_matrix[qt_device.row_index][i] == 0) {
                toLet = letterFromRow(qt_device.row_index);
                toNumb = i+1;
                ESP_LOGI(SPI_TAG, "SIIA TEHTI KÄIK: %C%X", toLet,toNumb);
            }
            button_matrix[qt_device.row_index][i] = 1;
            //ESP_LOGI(SPI_TAG, "siin real %x on nupp %x staatuses UKS", qt_device.row_index+1, i+1);
        }
        else {
            if (button_matrix[qt_device.row_index][i] == 1) {
                fromLet = letterFromRow(qt_device.row_index);
                fromNumb = i+1;
                ESP_LOGI(SPI_TAG, "SIIT TEHTI KÄIK: %C%X", fromLet,fromNumb);
            }
            button_matrix[qt_device.row_index][i] = 0;
            //ESP_LOGI(SPI_TAG,"siin real %x on nupp %x staatuses NULL", qt_device.row_index+1, i+1);
        }    
    }*/
}

void print_board(void) {
    printf("Printing board: \n");
    printf("\t    A B C D E F G H \n");                     // Print column legend
    for (int i = 0; i < BUTTON_MATRIX_ROW_SIZE;  ++i)
    {
        printf("\t %2d", i+1);                              // Print row legend
        for (int j = 0; j < BUTTON_MATRIX_COL_SIZE; ++j)
        {   
            if (button_matrix[i][j])
            {
               printf(" X");
            }
            else{
                printf(" O");
            }
        }
        printf("\n");
    }
}
void configure_spi(uint8_t numb_of_devices, device* device_arr) {
    ESP_LOGI(SPI_TAG, "spi conf \n");

    QT_MU_1_2_INT_FLAG = false;
    QT_MU_3_4_INT_FLAG = false;
    QT_SU_1_2_INT_FLAG = false;
    QT_SU_3_4_INT_FLAG = false;
    QT_INT_ERR_FLAG = false;
    device_arr[0].name = "MB_QT0_SPI";
    device_arr[0].cs_pin = SPI_MU_CS0_PIN;
    device_arr[1].name = "MB_QT1_SPI";
    device_arr[1].cs_pin = SPI_MU_CS1_PIN;
    device_arr[2].name = "MB_QT2_SPI";
    device_arr[2].cs_pin = SPI_MU_CS2_PIN;
    device_arr[3].name = "MB_QT3_SPI";
    device_arr[3].cs_pin = SPI_MU_CS3_PIN;

    device_arr[4].name = "SB_QT0_SPI";
    device_arr[4].cs_pin = SPI_SU_CS0_PIN;
    device_arr[5].name = "SB_QT1_SPI";
    device_arr[5].cs_pin = SPI_SU_CS1_PIN;
    device_arr[6].name = "SB_QT2_SPI";
    device_arr[6].cs_pin = SPI_SU_CS2_PIN;
    device_arr[7].name = "SB_QT3_SPI";
    device_arr[7].cs_pin = SPI_SU_CS3_PIN;


    gpio_config_t QT_CS_IOCONF;
    QT_CS_IOCONF.mode = GPIO_MODE_OUTPUT;
    QT_CS_IOCONF.pin_bit_mask = ((1ULL<<SPI_MU_CS0_PIN) | (1ULL<<SPI_MU_CS1_PIN) | (1ULL<<SPI_MU_CS2_PIN) | (1ULL<<SPI_MU_CS3_PIN) | (1ULL<<SPI_SU_CS0_PIN) | (1ULL<<SPI_SU_CS1_PIN) | (1ULL<<SPI_SU_CS2_PIN) | (1ULL<<SPI_SU_CS3_PIN));
    QT_CS_IOCONF.pull_down_en = 0;
    QT_CS_IOCONF.pull_up_en = 0;
    QT_CS_IOCONF.intr_type = GPIO_PIN_INTR_DISABLE;
    gpio_config(&QT_CS_IOCONF);
    for (int i = 0; i < numb_of_devices; i++) {
        gpio_set_level(device_arr[i].cs_pin, 1);
        device_arr[i].row_index = i;
    }
    
    initBothBoardsBus();

    addDeviceToBus(HSPI_HOST, &MB_QT0_devConfigStruct, &MB_QT0_SPI);
    addDeviceToBus(HSPI_HOST, &MB_QT1_devConfigStruct, &MB_QT1_SPI);
    addDeviceToBus(HSPI_HOST, &MB_QT2_devConfigStruct, &MB_QT2_SPI);
    addDeviceToBus(HSPI_HOST, &MB_QT3_devConfigStruct, &MB_QT3_SPI);

    addDeviceToBus(VSPI_HOST, &SB_QT0_devConfigStruct, &SB_QT0_SPI);
    addDeviceToBus(VSPI_HOST, &SB_QT1_devConfigStruct, &SB_QT1_SPI);
    addDeviceToBus(VSPI_HOST, &SB_QT2_devConfigStruct, &SB_QT2_SPI);
    addDeviceToBus(VSPI_HOST, &SB_QT3_devConfigStruct, &SB_QT3_SPI);

    printf("devices added\n");

    vTaskDelay(200/ portTICK_PERIOD_MS);
    device_arr[0].handle = MB_QT0_SPI;
    device_arr[1].handle = MB_QT1_SPI;
    device_arr[2].handle = MB_QT2_SPI;
    device_arr[3].handle = MB_QT3_SPI;

    device_arr[4].handle = SB_QT0_SPI;
    device_arr[5].handle = SB_QT1_SPI;
    device_arr[6].handle = SB_QT2_SPI;
    device_arr[7].handle = SB_QT3_SPI;

    for (int i = 0; i < numb_of_devices; i++) {
        QT_reset(device_arr[i]);
    }

    printf("devices reset done\n");

    vTaskDelay(200/ portTICK_PERIOD_MS);

    gpio_config_t interrupt_conf;
    interrupt_conf.pin_bit_mask = nCHNG_INT_PIN_SEL;
    interrupt_conf.mode = GPIO_MODE_INPUT;
    interrupt_conf.intr_type = GPIO_INTR_NEGEDGE;
    gpio_config(&interrupt_conf);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(SPI_MU_1_2_nCHANGE, MU_1_2_isr_handler, (void*) SPI_MU_1_2_nCHANGE);
    gpio_isr_handler_add(SPI_MU_3_4_nCHANGE, MU_3_4_isr_handler, (void*) SPI_MU_3_4_nCHANGE);
    gpio_isr_handler_add(SPI_SU_1_2_nCHANGE, SU_1_2_isr_handler, (void*) SPI_SU_1_2_nCHANGE);
    gpio_isr_handler_add(SPI_SU_3_4_nCHANGE, SU_3_4_isr_handler, (void*) SPI_SU_3_4_nCHANGE); 

    for (int i = 0; i < numb_of_devices; i++) {
        QT_setup(device_arr[i]);
    }

    printf("devices setup done\n"); 

    vTaskDelay(200/ portTICK_PERIOD_MS);
    for (int i = 0; i < numb_of_devices; i++) {
        printf("device name: %s\n", device_arr[i].name);
        QT_device_status(device_arr[i]);
    }
}

