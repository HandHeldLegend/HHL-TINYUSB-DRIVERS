#ifndef HHL_TUSB_XINPUT_H
#define HHL_TUSB_XINPUT_H

/**
 * XInput (Xbox 360 wired controller) TinyUSB class driver + descriptors.
 *
 * This module owns the USB device + configuration descriptors required to
 * enumerate as an Xbox 360 controller. Pull the descriptors from here instead
 * of hard coding them in the application.
 */

#include <stdint.h>
#include <stdbool.h>

#include "tusb.h"
#include "device/usbd_pvt.h"

#include "hhl_tusb_provider.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HHL_TUSB_DRIVER_XINPUT_ENABLE
#define HHL_TUSB_DRIVER_XINPUT_ENABLE 1
#endif

/* Identifiers used by the XInput descriptors. */
#define HHL_TUSB_XINPUT_VID  0x045E
#define HHL_TUSB_XINPUT_PID  0x028E
#define HHL_TUSB_XINPUT_NAME "XInput Gamepad"

/*------------- Descriptor accessors -------------*/

/** Raw USB device descriptor bytes (layout: tusb_desc_device_t). */
const uint8_t *hhl_tusb_xinput_device_descriptor(void);
uint16_t       hhl_tusb_xinput_device_descriptor_len(void);

/** Raw USB configuration descriptor bytes. */
const uint8_t *hhl_tusb_xinput_configuration_descriptor(void);
uint16_t       hhl_tusb_xinput_configuration_descriptor_len(void);

/*------------- Class driver API -------------*/

/** TinyUSB application class driver instance for XInput. */
const usbd_class_driver_t *hhl_tusb_xinput_driver(void);

/** Driver ops for the XInput driver. */
const hhl_tusb_driver_ops_s *hhl_tusb_xinput_ops(void);

/** @deprecated Use hhl_tusb_xinput_ops(). */
const hhl_tusb_driver_ops_s *hhl_tusb_xinput_provider(void);

/** True when the IN endpoint is ready to accept a new report. */
bool tud_xinput_ready(void);

/** Send an XInput report. report_id is prepended as the first byte. */
bool tud_xinput_report(uint8_t report_id, void const *report, uint16_t len);
bool tud_n_xinput_report(uint8_t report_id, void const *report, uint16_t len);

/** Queue an OUT transfer to receive host data (rumble / LED). */
void tud_xinput_getout(void);

#ifdef __cplusplus
}
#endif

#endif /* HHL_TUSB_XINPUT_H */
