#ifndef HHL_TUSB_PROVIDER_H
#define HHL_TUSB_PROVIDER_H

/**
 * Driver operations interface.
 *
 * Each driver (HID, XInput, Slippi) exposes a table of function pointers that
 * handle every TinyUSB descriptor and vendor-control path for that mode. The
 * central callback dispatcher in hhl_tusb_callbacks.c routes all tud_* requests
 * through the active provider so the host firmware never needs per-driver logic.
 *
 * Any member may be NULL when a driver does not handle that request.
 */

#include <stdint.h>
#include <stdbool.h>

#include "tusb.h"
#include "device/usbd_pvt.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const uint8_t *(*device_descriptor)(void);
    const uint8_t *(*configuration_descriptor)(uint8_t index);
    const uint8_t *(*hid_report_descriptor)(uint8_t instance);
    const uint8_t *(*bos_descriptor)(void);
    const uint8_t *(*ms_os_20_descriptor)(uint16_t *len);
    uint16_t const *(*descriptor_string)(uint8_t index, uint16_t langid);
    bool (*vendor_control_xfer)(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);
    const usbd_class_driver_t *(*class_driver)(void);
} hhl_tusb_driver_ops_s;

#ifdef __cplusplus
}
#endif

#endif /* HHL_TUSB_PROVIDER_H */
