#include "ble_gopro.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static const char *TAG = "BLE_GOPRO_GAP";

/*
 * If not already defined elsewhere, define a helper to convert a BLE address to a string.
 */
#ifndef ble_addr_to_str
static inline char *ble_addr_to_str(const ble_addr_t *addr, char *str)
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
#endif

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

/**
 * Connects to the advertiser if it appears to be a GoPro.
 */
void ble_connect(void *disc)
{
    uint8_t own_addr_type;
    int rc;
    ble_addr_t *addr;

    /* Scanning must be stopped before a connection can be initiated. */
    ESP_LOGI(TAG, "Cancelling scan");
    rc = ble_gap_disc_cancel();
    if (rc != 0)
    {
        MODLOG_DFLT(DEBUG, "Failed to cancel scan; rc=%d\n", rc);
        return;
    }

    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0)
    {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    addr = &((struct ble_gap_disc_desc *)disc)->addr;

    ESP_LOGI(TAG, "ble_gap_connect");
    rc = ble_gap_connect(own_addr_type, addr, 30000, NULL,
                         blecent_gap_event, NULL);
    if (rc == BLE_HS_EALREADY)
    {
        ESP_LOGI(TAG, "Connection already in progress!");
        return;
    }
    else if (rc != 0)
    {
        char addr_str_buf[18];
        MODLOG_DFLT(ERROR, "Error: Failed to connect to device; addr_type=%d addr=%s; rc=%d\n",
                    addr->type, ble_addr_to_str(addr, addr_str_buf), rc);
        return;
    }
    ESP_LOGI(TAG, "Connection complete!");
}

/**
 * GAP event handler.
 */
int blecent_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int rc;
    char addr_str_buf[18];

    switch (event->type)
    {
    case BLE_GAP_EVENT_DISC:
    {
        ESP_LOGI(TAG, "GAP EVENT: BLE_GAP_EVENT_DISC");
        struct ble_gap_disc_desc *disc = &event->disc;
        ESP_LOGI(TAG, "Discovered device: %s", ble_addr_to_str(&disc->addr, addr_str_buf));

        bool is_gopro = false;
        const uint8_t *adv_data = disc->data;
        uint8_t adv_length = disc->length_data;

        int pos = 0;
        while (pos < adv_length)
        {
            uint8_t field_len = adv_data[pos];
            if (field_len == 0)
                break;
            uint8_t field_type = adv_data[pos + 1];

            // Check for 16-bit Service UUIDs (0x02 = Incomplete, 0x03 = Complete)
            if (field_type == 0x02 || field_type == 0x03)
            {
                for (int i = 2; i < field_len + 1; i += 2)
                {
                    uint16_t svc_uuid = adv_data[pos + i] | (adv_data[pos + i + 1] << 8);
                    if (svc_uuid == 0xFEA6)
                    {
                        is_gopro = true;
                    }
                }
            }
            pos += field_len + 1;
        }

        if (is_gopro)
        {
            ESP_LOGI(TAG, "GoPro Discovered: %s (RSSI: %d dBm)",
                     ble_addr_to_str(&disc->addr, addr_str_buf), disc->rssi);

            ble_connect(disc);
        }
        return 0;
    }

    case BLE_GAP_EVENT_DISC_COMPLETE:
    {
        ESP_LOGI(TAG, "GAP EVENT: BLE_GAP_EVENT_DISC_COMPLETE");
        MODLOG_DFLT(INFO, "discovery complete; reason=%d\n", event->disc_complete.reason);
        return 0;
    }

    case BLE_GAP_EVENT_LINK_ESTAB:
    {
        ESP_LOGI(TAG, "GAP EVENT: BLE_GAP_EVENT_LINK_ESTAB");
        print_conn_desc(&desc);
        rc = ble_gap_conn_find(event->connect.conn_handle, &desc);

        if (rc == 0)
        {
            MODLOG_DFLT(INFO, "Initiating Security");
            rc = ble_gap_security_initiate(event->connect.conn_handle);
            if (rc != 0)
            {
                MODLOG_DFLT(INFO, "Security could not be initiated, rc = %d\n", rc);
                return ble_gap_terminate(event->connect.conn_handle,
                                         BLE_ERR_REM_USER_CONN_TERM);
            }
            else
            {
                MODLOG_DFLT(INFO, "Connection security initiated\n");
            }
        }
        else
        {
            MODLOG_DFLT(ERROR, "Error: Could not find connection handle!");
        }
        return 0;
    }

    case BLE_GAP_EVENT_ENC_CHANGE:
    {
        ESP_LOGI(TAG, "GAP EVENT: BLE_GAP_EVENT_ENC_CHANGE; handle=%u  status=%d",
            event->enc_change.conn_handle,
            event->enc_change.status);
        return 0;
    }

    case BLE_GAP_EVENT_CONNECT:
    {
        ESP_LOGI(TAG, "BLE_GAP_EVENT_CONNECT");

        if (event->connect.status == 0) {
            ESP_LOGI(TAG, "Connected! New handle=%u",
                     event->connect.conn_handle);
        } else {
            ESP_LOGI(TAG, "Connection failed; status=%d",
                     event->connect.status);
        }

        return 0;
    }

    case BLE_GAP_EVENT_PARING_COMPLETE:
    {
        ESP_LOGI(TAG, "BLE_GAP_EVENT_PARING_COMPLETE");
        return 0;
    }

    case BLE_GAP_EVENT_REATTEMPT_COUNT:
    {
        ESP_LOGI(TAG, "BLE_GAP_EVENT_REATTEMPT_COUNT");
        return 0;
    }

    case BLE_GAP_EVENT_DISCONNECT:
    {
        ESP_LOGI(TAG, "BLE_GAP_EVENT_DISCONNECT");
        return 0;
    }

    default:
        ESP_LOGI(TAG, "GAP: Unhandled GAP event received! Event Type: %d", event->type);
        return 0;
    }
}
