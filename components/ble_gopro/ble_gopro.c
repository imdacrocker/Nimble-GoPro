#include "ble_gopro.h"
#include <stdio.h>
#include <assert.h>
#include "host/ble_store.h"
#include "esp_log.h"
#include "host/ble_uuid.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_nimble_hci.h"

static const char *TAG = "BLE_GOPRO";

void ble_store_config_init(void);

/*
 * Reset callback for the BLE host.
 */
static void ble_on_reset(int reason)
{
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

/*
 * Sync callback for the BLE host.
 */
static void ble_on_sync(void)
{
    ESP_LOGI(TAG, "ble_on_sync");
    int rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);
}

/*
 * The BLE host task.
 */
void ble_host_task(void *param)
{
    ESP_LOGI(TAG, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();
    nimble_port_freertos_deinit();
}

/*
 * Initiates the GAP discovery procedure.
 */
void ble_gopro_scan(void)
{
    ESP_LOGI(TAG, "Initializing scanning...");

    uint8_t own_addr_type;
    struct ble_gap_disc_params disc_params;
    int rc;

    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    ble_addr_t camera_address; // Variable to store the loaded camera address
    bool cameraFound = false;

    // Call the function to load the camera address from NVS
    esp_err_t err = load_camera_info_from_nvs(&camera_address);

    camera_address.type = BLE_ADDR_PUBLIC;
    if (err == ESP_OK)
    {
        // Successfully loaded the camera address
        char addr_str[18];
        ble_addr_to_str(&camera_address, addr_str);
        ESP_LOGI(TAG, "Camera loaded! addr:%s type:%d",addr_str, camera_address.type);
        cameraFound = true;
        reverse_addr(&camera_address);

        ble_addr_t wl_addrs[1] = {camera_address};
        int rc = ble_gap_wl_set(wl_addrs, 1);
        if (rc != ESP_OK)
        {
            ESP_LOGE(TAG,"Error setting whitelist! error=%d",rc);
        }
    }
    else
    {
        ESP_LOGE(TAG, "Failed to retrieve camera address from NVS");
    }

    disc_params.filter_duplicates = 1;
    disc_params.passive = 1;
    disc_params.itvl = 0x0010;
    disc_params.window = 0x0010;
    if (cameraFound)
    {
        disc_params.filter_policy = 0;
    }
    else
    {
    disc_params.filter_policy = 0;
    }
    disc_params.limited = 0;

    rc = ble_gap_disc(own_addr_type, 30 * 1000, &disc_params,
                      blecent_gap_event, NULL);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "Error initiating GAP discovery procedure; rc=%d\n", rc);
    }
}

/*
 * Initializes the BLE host.
 */
void ble_gopro_init(void)
{
    ;
    ESP_LOGI(TAG, "Initializing BLE storage...");
    ble_store_config_init();


    ESP_LOGI(TAG, "Initializing BLE...");
    esp_err_t ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init nimble %d ", ret);
        return;
    }

    ble_hs_cfg.reset_cb = ble_on_reset;
    ble_hs_cfg.sync_cb = ble_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr; // This needs to be replaced, it just removes old entries
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ble_hs_cfg.sm_bonding = 1; // Enable bonding
    ble_hs_cfg.sm_mitm = 0; // No Man in the Middle protection
    ble_hs_cfg.sm_sc = 1; // Enable secure connections
    ble_hs_cfg.sm_our_key_dist   = BLE_SM_PAIR_KEY_DIST_ENC   // share our LTK
                            | BLE_SM_PAIR_KEY_DIST_ID;   // share our IRK
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC   // request peer’s LTK
                            | BLE_SM_PAIR_KEY_DIST_ID;   // request peer’s IRK

    ESP_LOGI(TAG, "Initializing NimBLE port...");
    nimble_port_freertos_init(ble_host_task);

    // ESP_LOGI(TAG,"Clearing whitelist...");
    // ble_gap_wl_tx_clear();

    // uint8_t size;
    // ble_gap_wl_read_size(&size);

    // ESP_LOGI(TAG, "Whitelist size: %d", size);

}