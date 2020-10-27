
typedef struct {
    const char* name;
    int cs_pin;
    spi_device_handle_t handle;
    uint8_t host;
    int row_index;
} device;
spi_device_handle_t MB_QT0_SPI;
spi_device_handle_t MB_QT1_SPI;
spi_device_handle_t MB_QT2_SPI;
spi_device_handle_t MB_QT3_SPI;
spi_device_handle_t SB_QT0_SPI;
spi_device_handle_t SB_QT1_SPI;
spi_device_handle_t SB_QT2_SPI;
spi_device_handle_t SB_QT3_SPI;
bool QT_MU_1_2_INT_FLAG;
bool QT_MU_3_4_INT_FLAG;
bool QT_SU_1_2_INT_FLAG;
bool QT_SU_3_4_INT_FLAG;
bool QT_INT_ERR_FLAG;


void configure_spi(uint8_t, device*);
const char* QT_handle_to_string(device);
void QT_check_buttons_and_update_board(device);
void print_board(void);