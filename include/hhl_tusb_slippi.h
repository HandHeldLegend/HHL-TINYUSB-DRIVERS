#ifndef HHL_TUSB_SLIPPI_H
#define HHL_TUSB_SLIPPI_H

/**
 * Slippi / Nintendo GameCube Adapter TinyUSB class driver + descriptors.
 *
 * Enumerates as an official Nintendo GameCube controller adapter (WUP-028).
 * This module owns every descriptor required for that: device, configuration,
 * HID report, BOS and the Microsoft OS 2.0 descriptor set.
 */

#include <stdint.h>
#include <stdbool.h>

#include "tusb.h"
#include "device/usbd_pvt.h"

#include "hhl_tusb_provider.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HHL_TUSB_DRIVER_SLIPPI_ENABLE
#define HHL_TUSB_DRIVER_SLIPPI_ENABLE 1
#endif

/* Identifiers used by the GameCube adapter descriptors. */
#define HHL_TUSB_SLIPPI_VID 0x057E
#define HHL_TUSB_SLIPPI_PID 0x0337

/*------------- Descriptor accessors -------------*/

/** Raw USB device descriptor bytes (layout: tusb_desc_device_t). */
const uint8_t *hhl_tusb_slippi_device_descriptor(void);
uint16_t       hhl_tusb_slippi_device_descriptor_len(void);

/** Raw USB configuration descriptor bytes. */
const uint8_t *hhl_tusb_slippi_configuration_descriptor(void);
uint16_t       hhl_tusb_slippi_configuration_descriptor_len(void);

/** HID report descriptor bytes. */
const uint8_t *hhl_tusb_slippi_hid_report_descriptor(void);
uint16_t       hhl_tusb_slippi_hid_report_descriptor_len(void);

/** BOS descriptor bytes (advertises the MS OS 2.0 capability). */
const uint8_t *hhl_tusb_slippi_bos_descriptor(void);
uint16_t       hhl_tusb_slippi_bos_descriptor_len(void);

/** Microsoft OS 2.0 descriptor set bytes. */
const uint8_t *hhl_tusb_slippi_ms_os_20_descriptor(void);
uint16_t       hhl_tusb_slippi_ms_os_20_descriptor_len(void);

/*------------- Class driver API -------------*/

/** TinyUSB application class driver instance for the GameCube adapter. */
const usbd_class_driver_t *hhl_tusb_slippi_driver(void);

/** Driver ops for the Slippi/GameCube driver. */
const hhl_tusb_driver_ops_s *hhl_tusb_slippi_ops(void);

/** @deprecated Use hhl_tusb_slippi_ops(). */
const hhl_tusb_driver_ops_s *hhl_tusb_slippi_provider(void);

/** True when the IN endpoint is ready to accept a new report. */
bool tud_slippi_ready(void);
bool tud_slippi_n_ready(uint8_t instance);

/** Send a GameCube adapter report. report_id is prepended as the first byte. */
bool tud_slippi_report(uint8_t report_id, void const *report, uint16_t len);
bool tud_slippi_n_report(uint8_t instance, uint8_t report_id, void const *report, uint16_t len);

uint8_t tud_slippi_n_interface_protocol(uint8_t instance);
uint8_t tud_slippi_n_get_protocol(uint8_t instance);

#ifdef __cplusplus
}
#endif

#endif /* HHL_TUSB_SLIPPI_H */
