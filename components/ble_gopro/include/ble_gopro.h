#ifndef BLE_GOPRO_H
#define BLE_GOPRO_H

#include "esp_log.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_hs_adv.h"
#include "esp_bt.h"

// BLE utility includes
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"

#define BLEGOPRO_QUERY_UUID     0xFEA6

#define MAX_DEVICES 1

#ifdef __cplusplus
extern "C" {
#endif

char *ble_addr_to_str(const ble_addr_t *addr, char *str);
void print_conn_desc(const struct ble_gap_conn_desc *desc);
void reverse_addr(ble_addr_t *addr);

ble_addr_t get_peer_addr(uint16_t conn_handle);

void save_camera_info_array_to_nvs(const ble_addr_t *addr);
esp_err_t load_camera_info_from_nvs(ble_addr_t *addr);
esp_err_t delete_camera_info_from_nvs();
void print_bonded_peers();

// Public API function prototypes
void ble_gopro_init(void);
void ble_gopro_scan(void);
void ble_host_task(void *param);

// GAP event handler used by the scan; defined in ble_gopro_gap.c
int blecent_gap_event(struct ble_gap_event *event, void *arg);

typedef struct {
    uint8_t mac[6];     // MAC address
    uint32_t ip;        // IP address (32-bit)
} CameraInfo;

#endif // BLE_GOPRO_H
