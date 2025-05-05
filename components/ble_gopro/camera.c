#include "ble_gopro.h"
#include "nvs_flash.h"
#include "nvs.h"
 
static const char *TAG = "CAMERA";



void save_camera_info_array_to_nvs(const ble_addr_t *addr) {
    nvs_handle_t nvs_handle;
    char addr_str_buf[18];
    ESP_LOGI(TAG,"Saving camera: %s",ble_addr_to_str(addr, addr_str_buf));
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    ESP_ERROR_CHECK(err);

    // Save the IP-MAC pair array to NVS
    err = nvs_set_blob(nvs_handle, "camera_address", &addr, sizeof(addr));
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG,"Could not open NVS!");
        return;
    }

    // Commit the changes to NVS
    err = nvs_commit(nvs_handle);
    
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG,"Could not commit to NVS!");
        return;
    }

    // Close the NVS handle
    nvs_close(nvs_handle);

    ESP_LOGI(TAG, "Camera Saved");
}

esp_err_t load_camera_info_from_nvs(ble_addr_t *addr) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);

    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG,"Could not open NVS!");
        return err;
    }

    // Read the stored blob (camera_address) into the addr variable
    size_t addr_size = sizeof(ble_addr_t);  // Make sure this matches the size of ble_addr_t
    err = nvs_get_blob(nvs_handle, "camera_address", addr, &addr_size);
    
    if (err == ESP_OK) {
        char addr_str_buf[18];
        ESP_LOGI(TAG, "Camera info loaded successfully: %s",ble_addr_to_str(addr, addr_str_buf));
    } else {
        ESP_LOGE(TAG, "Failed to load camera info from NVS.");
    }

    // Close the NVS handle
    nvs_close(nvs_handle);

    return err;
}

esp_err_t delete_camera_info_from_nvs() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);

    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG,"Could not open NVS!");
        return err;
    }

    // Delete the camera_address key
    err = nvs_erase_key(nvs_handle, "camera_address");
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Camera info deleted successfully.");
    } else {
        ESP_LOGE(TAG, "Failed to delete camera info.");
    }

    // Commit the changes to NVS
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG,"Could not commit to NVS!");
        return err;
    }

    // Close the NVS handle
    nvs_close(nvs_handle);

    return err;
}