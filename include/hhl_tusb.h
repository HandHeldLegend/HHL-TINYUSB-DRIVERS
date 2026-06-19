#ifndef HHL_TUSB_H
#define HHL_TUSB_H

/**
 * HHL-TINYUSB-DRIVERS — public host firmware API.
 *
 * Include this header only. TinyUSB callbacks, descriptor dispatch, and class
 * drivers are owned by the library.
 */

#include <stdint.h>
#include <stdbool.h>

#include "hhl_tusb_strings.h"
#include "hhl_tusb_transport.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HHL_TUSB_DRIVER_XINPUT_ENABLE
#define HHL_TUSB_DRIVER_XINPUT_ENABLE 1
#endif

#ifndef HHL_TUSB_DRIVER_SLIPPI_ENABLE
#define HHL_TUSB_DRIVER_SLIPPI_ENABLE 1
#endif

typedef enum
{
    HHL_TUSB_DRIVER_NONE = 0,
    HHL_TUSB_DRIVER_HID,
    HHL_TUSB_DRIVER_XINPUT,
    HHL_TUSB_DRIVER_SLIPPI,
} hhl_tusb_driver_t;

typedef struct
{
    bool enabled;
    const char *url;
    const uint8_t *popup_enable_flag;
} hhl_tusb_webusb_config_s;

typedef struct
{
    const uint8_t *device_descriptor;
    uint16_t device_descriptor_len;
    const uint8_t *config_descriptor;
    uint16_t config_descriptor_len;
    const uint8_t *report_descriptor;
    uint16_t report_descriptor_len;
    uint16_t max_power_ma;
} hhl_tusb_hid_config_s;

typedef struct
{
    void (*vendor_rx)(const uint8_t *data, uint16_t len);
    void (*platform_sleep_ms)(uint32_t ms);
    void (*hid_output_report)(const uint8_t *buffer, uint16_t len);
    void (*on_mount)(void);
    void (*on_sof)(uint32_t frame_count);
} hhl_tusb_host_hooks_s;

typedef struct
{
    hhl_tusb_driver_t driver;
    hhl_tusb_strings_config_s strings;
    hhl_tusb_webusb_config_s webusb;
    hhl_tusb_hid_config_s hid;
    hhl_tusb_host_hooks_s hooks;
} hhl_tusb_config_s;

void hhl_tusb_init(const hhl_tusb_config_s *cfg);
hhl_tusb_driver_t hhl_tusb_get_active_driver(void);
bool hhl_tusb_webusb_active(void);

/** WebUSB configurator bulk I/O (vendor interface). */
bool hhl_tusb_webusb_report_ready_blocking(int timeout_ms);
bool hhl_tusb_webusb_report_send(const uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* HHL_TUSB_H */
