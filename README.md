# HHL-TINYUSB-DRIVERS

Portable TinyUSB device-stack wrapper: custom class drivers (XInput, Slippi),
descriptor dispatch, WebUSB composition, and a single host-facing API.

Host firmware includes **`hhl_tusb.h` only** — no TinyUSB headers required in
application code.

## Drivers

| Mode | Description |
| ---- | ----------- |
| `HHL_TUSB_DRIVER_HID` | Generic HID (Switch Pro, SInput) with optional WebUSB vendor interface |
| `HHL_TUSB_DRIVER_XINPUT` | Xbox 360 wired controller |
| `HHL_TUSB_DRIVER_SLIPPI` | Nintendo GameCube adapter (WUP-028) |

Enable/disable built-in drivers at compile time (default: both on):

- `HHL_TUSB_DRIVER_XINPUT_ENABLE`
- `HHL_TUSB_DRIVER_SLIPPI_ENABLE`

## Host integration

1. Add the library to your build (`add_subdirectory` + link `hhl_tinyusb_drivers`).
2. Include `hhl_tusb_config.h` from your `tusb_config.h`.
3. Fill in `hhl_tusb_config_s` and call `hhl_tusb_init()` + `hhl_tusb_start()`.
4. Poll with `hhl_tusb_task()`; send gamepad reports via `hhl_tusb_report_send()`.
5. WebUSB configurator traffic uses `hhl_tusb_webusb_report_send()`.

```c
#include "hhl_tusb.h"

hhl_tusb_config_s cfg = {0};
cfg.driver = HHL_TUSB_DRIVER_HID;
cfg.strings.manufacturer = "HOJA";
cfg.strings.product = "Gamepad";
cfg.strings.serial_number = serial;

cfg.webusb.enabled = true;
cfg.webusb.url = "https://example.com";

cfg.hid.device_descriptor = device_desc;
cfg.hid.device_descriptor_len = HHL_TUSB_STD_DEVICE_DESC_LEN;
cfg.hid.config_descriptor = config_desc;
cfg.hid.config_descriptor_len = config_len;
cfg.hid.report_descriptor = report_desc;
cfg.hid.report_descriptor_len = report_len;
cfg.hid.max_power_ma = 250;

cfg.hooks.on_sof = my_sof_tick;
cfg.hooks.hid_output_report = my_rumble_handler;
cfg.hooks.vendor_rx = my_webusb_command_handler;

hhl_tusb_init(&cfg);
hhl_tusb_start();
```

## Internal layout

| Header | Audience |
| ------ | -------- |
| `hhl_tusb.h` | Host firmware (public) |
| `hhl_tusb_private.h` | Library internals only |
| `hhl_tusb_provider.h`, `hhl_tusb_webusb.h`, … | Driver implementation |

`hhl_tusb_drivers.h` is a deprecated alias for `hhl_tusb.h`.
