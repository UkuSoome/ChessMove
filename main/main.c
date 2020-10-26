#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "spi_config.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#define CHIP_NAME "ESP32"
#endif


void app_main(void)
{   
    static const char *SPI_TAG = "MAIN";
    configure_spi();
    while (1) {
        ESP_LOGI(SPI_TAG, "main \n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}

