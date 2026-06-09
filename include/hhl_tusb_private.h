#ifndef HHL_TUSB_PRIVATE_H
#define HHL_TUSB_PRIVATE_H

/**
 * HHL-TINYUSB-DRIVERS — library-internal API.
 *
 * Not for host firmware. Used by callback dispatch, transport, and driver modules.
 */

#include <stdint.h>
#include <stdbool.h>

#include "hhl_tusb.h"
#include "hhl_tusb_provider.h"

#ifdef __cplusplus
extern "C" {
#endif

void hhl_tusb_set_active_driver(hhl_tusb_driver_t driver);
const hhl_tusb_driver_ops_s *hhl_tusb_active_ops(void);

void (*hhl_tusb_hook_vendor_rx(void))(const uint8_t *data, uint16_t len);
void (*hhl_tusb_hook_vendor_rx_preamble(void))(void);
void (*hhl_tusb_hook_platform_sleep_ms(void))(uint32_t ms);
void (*hhl_tusb_hook_hid_output_report(void))(const uint8_t *buffer, uint16_t len);
void (*hhl_tusb_hook_on_mount(void))(void);
void (*hhl_tusb_hook_on_sof(void))(uint32_t frame_count);

#ifdef __cplusplus
}
#endif

#endif /* HHL_TUSB_PRIVATE_H */
