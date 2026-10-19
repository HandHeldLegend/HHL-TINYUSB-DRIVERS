#ifndef HHL_TUSB_NS2_H
#define HHL_TUSB_NS2_H

/**
 * Switch 2 controller over USB: a HID interface (input reports in, output reports out) beside a
 * vendor interface whose bulk endpoints carry the console's commands and the replies. The
 * descriptors and every reply come from the host firmware (NS2-LIB-HID): this driver only moves
 * them.
 */

#include <stdint.h>
#include <stdbool.h>

#include "tusb.h"
#include "device/usbd_pvt.h"

#include "hhl_tusb.h"
#include "hhl_tusb_provider.h"

#ifdef __cplusplus
extern "C" {
#endif

void hhl_tusb_ns2_configure(const hhl_tusb_ns2_config_s *cfg);

const usbd_class_driver_t *hhl_tusb_ns2_driver(void);
const hhl_tusb_driver_ops_s *hhl_tusb_ns2_ops(void);

bool tud_ns2_ready(void);
bool tud_ns2_report(uint8_t report_id, void const *report, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* HHL_TUSB_NS2_H */
