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

// Public API function prototypes
void ble_gopro_init(void);
void ble_gopro_scan(void);
void ble_host_task(void *param);

// GAP event handler used by the scan; defined in ble_gopro_gap.c
int blecent_gap_event(struct ble_gap_event *event, void *arg);

#endif // BLE_GOPRO_H
