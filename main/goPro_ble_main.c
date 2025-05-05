#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"

#include "ble_gopro.h"

static const char *TAG = "GoPro ESP32";

void app_main(void)
{
    /* NVS flash initialization */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init(); // Retry initialization
    }
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize NVS flash, error code: %d", ret);
        return;
    }
    else
    {
        ESP_LOGI(TAG, "NVS flash initialized successfully");
    }

    ble_gopro_init();

    ble_gopro_scan();
}