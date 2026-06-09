#ifndef HHL_TUSB_CALLBACKS_H
#define HHL_TUSB_CALLBACKS_H

/**
 * TinyUSB callback dispatch layer.
 *
 * All standard TinyUSB device callbacks are implemented in hhl_tusb_callbacks.c.
 * Host firmware includes hhl_tusb.h only; hooks are registered via
 * hhl_tusb_config_s.hooks at init time.
 */

#include <stdint.h>
#include <stdbool.h>

#include "tusb.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t const *hhl_tusb_cb_descriptor_device(void);
uint8_t const *hhl_tusb_cb_descriptor_configuration(uint8_t index);
uint8_t const *hhl_tusb_cb_descriptor_bos(void);
uint16_t const *hhl_tusb_cb_descriptor_string(uint8_t index, uint16_t langid);
uint8_t const *hhl_tusb_cb_hid_descriptor_report(uint8_t instance);
bool hhl_tusb_cb_vendor_control_xfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);

#ifdef __cplusplus
}
#endif

#endif /* HHL_TUSB_CALLBACKS_H */
