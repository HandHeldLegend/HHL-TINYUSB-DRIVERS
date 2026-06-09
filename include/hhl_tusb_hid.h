#ifndef HHL_TUSB_HID_H
#define HHL_TUSB_HID_H

/**
 * Generic HID driver (Switch Pro, SInput, etc.) — library internal.
 */

#include <stdint.h>
#include <stdbool.h>

#include "hhl_tusb.h"
#include "hhl_tusb_provider.h"

#ifdef __cplusplus
extern "C" {
#endif

void hhl_tusb_hid_configure(const hhl_tusb_hid_config_s *hid, const hhl_tusb_webusb_config_s *webusb);
const hhl_tusb_driver_ops_s *hhl_tusb_hid_ops(void);
bool hhl_tusb_hid_webusb_active(void);

#ifdef __cplusplus
}
#endif

#endif /* HHL_TUSB_HID_H */
