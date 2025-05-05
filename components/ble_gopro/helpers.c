#include "ble_gopro.h"

static const char *TAG = "BLE_GOPRO_HELPERS";


char *ble_addr_to_str(const ble_addr_t *addr, char *str)
{
    sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X",
            addr->val[0],
            addr->val[1],
            addr->val[2],
            addr->val[3],
            addr->val[4],
            addr->val[5]);
    return str;
}

ble_addr_t get_peer_addr(uint16_t conn_handle) {
    struct ble_gap_conn_desc desc;
    int rc = ble_gap_conn_find(conn_handle, &desc);

    if (rc != 0) {
        // Return an invalid address (all zeros and type 0xFF)
        ble_addr_t invalid_addr = {
            .type = 0xFF,
            .val = {0}
        };
        return invalid_addr;
    }

    return desc.peer_ota_addr;
}

// Get bonded peers
void print_bonded_peers()
{
    ble_addr_t addrs[5];
    int num_addrs;
    int rc;
    rc = ble_store_util_bonded_peers(addrs, &num_addrs, 5);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Failed to get bonded peers, rc=%d", rc);
        return;
    }

    if (num_addrs == 0)
    {
        ESP_LOGI(TAG, "No bonded peers found, starting fresh scan");
        return;
    }

    ESP_LOGI(TAG, "Number of bonded peers: %d", num_addrs);

    // Log each bonded peer address
    for (int i = 0; i < num_addrs; i++)
    {
        char addr_str[18];
        ble_addr_to_str(&addrs[i], addr_str);
        ESP_LOGI(TAG, "Bonded peer %d: %s (type: %d)", i, addr_str, addrs[i].type);
    }
}

void print_conn_desc(const struct ble_gap_conn_desc *desc)
{
    MODLOG_DFLT(INFO, "conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                      "encrypted=%d authenticated=%d bonded=%d conn id =%u" ,
                desc->conn_itvl, desc->conn_latency,
                desc->supervision_timeout,
                desc->sec_state.encrypted,
                desc->sec_state.authenticated,
                desc->sec_state.bonded,
                desc->conn_handle);
}

void reverse_addr(ble_addr_t *addr)
{
    for (int i = 0; i < 3; i++) {
        uint8_t tmp = addr->val[i];
        addr->val[i] = addr->val[5 - i];
        addr->val[5 - i] = tmp;
    }
}
