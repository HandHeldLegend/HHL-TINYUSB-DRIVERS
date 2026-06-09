#ifndef HHL_TUSB_TRANSPORT_H
#define HHL_TUSB_TRANSPORT_H

/**
 * Portable transport API — host firmware uses these instead of calling tud_* /
 * tusb_* directly for stack lifecycle and class report I/O.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Standard USB 2.0 device descriptor length in bytes. */
#define HHL_TUSB_STD_DEVICE_DESC_LEN 18

/** Run the TinyUSB device task (wraps tud_task). */
void hhl_tusb_task(void);

/** Initialize the TinyUSB device stack (call after hhl_tusb_init). */
bool hhl_tusb_start(void);

/** Tear down the TinyUSB device stack and clear the active driver. */
void hhl_tusb_stop(void);

/** True when the active class driver can accept an IN report. */
bool hhl_tusb_report_ready(void);

/** Send an IN report through the active class driver. */
bool hhl_tusb_report_send(uint8_t report_id, const void *report, uint16_t len);

/** Enable or disable the start-of-frame callback (used on USB mount). */
void hhl_tusb_sof_cb_enable(bool enable);

#ifdef __cplusplus
}
#endif

#endif /* HHL_TUSB_TRANSPORT_H */
