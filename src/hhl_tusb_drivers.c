#include <string.h>

#include "hhl_tusb_private.h"

#include "hhl_tusb_callbacks.h"
#include "hhl_tusb_hid.h"
#include "hhl_tusb_strings.h"

#include "tusb.h"
#include "device/usbd_pvt.h"

#if (HHL_TUSB_DRIVER_XINPUT_ENABLE)
#include "hhl_tusb_xinput.h"
#endif

#if (HHL_TUSB_DRIVER_SLIPPI_ENABLE)
#include "hhl_tusb_slippi.h"
#endif

static hhl_tusb_driver_t _active_driver = HHL_TUSB_DRIVER_NONE;
static const hhl_tusb_driver_ops_s *_active_ops = NULL;
static hhl_tusb_host_hooks_s _hooks = {0};

static void _hhl_tusb_clear_hooks(void)
{
    memset(&_hooks, 0, sizeof(_hooks));
}

void hhl_tusb_set_active_driver(hhl_tusb_driver_t driver)
{
    _active_driver = driver;

    switch (driver)
    {
    case HHL_TUSB_DRIVER_HID:
        _active_ops = hhl_tusb_hid_ops();
        break;

#if (HHL_TUSB_DRIVER_XINPUT_ENABLE)
    case HHL_TUSB_DRIVER_XINPUT:
        _active_ops = hhl_tusb_xinput_ops();
        break;
#endif

#if (HHL_TUSB_DRIVER_SLIPPI_ENABLE)
    case HHL_TUSB_DRIVER_SLIPPI:
        _active_ops = hhl_tusb_slippi_ops();
        break;
#endif

    default:
        _active_ops = NULL;
        break;
    }
}

void hhl_tusb_init(const hhl_tusb_config_s *cfg)
{
    if (cfg == NULL)
    {
        hhl_tusb_strings_configure(NULL);
        _hhl_tusb_clear_hooks();
        hhl_tusb_hid_configure(NULL, NULL);
        hhl_tusb_set_active_driver(HHL_TUSB_DRIVER_NONE);
        return;
    }

    hhl_tusb_strings_configure(&cfg->strings);
    _hooks = cfg->hooks;

    if (cfg->driver == HHL_TUSB_DRIVER_HID)
    {
        hhl_tusb_hid_configure(&cfg->hid, &cfg->webusb);
    }
    else
    {
        hhl_tusb_hid_configure(NULL, NULL);
    }

    hhl_tusb_set_active_driver(cfg->driver);
}

hhl_tusb_driver_t hhl_tusb_get_active_driver(void)
{
    return _active_driver;
}

const hhl_tusb_driver_ops_s *hhl_tusb_active_ops(void)
{
    return _active_ops;
}

bool hhl_tusb_webusb_active(void)
{
    return hhl_tusb_hid_webusb_active();
}

void (*hhl_tusb_hook_vendor_rx(void))(const uint8_t *data, uint16_t len)
{
    return _hooks.vendor_rx;
}

void (*hhl_tusb_hook_vendor_rx_preamble(void))(void)
{
    return _hooks.vendor_rx_preamble;
}

void (*hhl_tusb_hook_platform_sleep_ms(void))(uint32_t ms)
{
    return _hooks.platform_sleep_ms;
}

void (*hhl_tusb_hook_hid_output_report(void))(const uint8_t *buffer, uint16_t len)
{
    return _hooks.hid_output_report;
}

void (*hhl_tusb_hook_on_mount(void))(void)
{
    return _hooks.on_mount;
}

void (*hhl_tusb_hook_on_sof(void))(uint32_t frame_count)
{
    return _hooks.on_sof;
}

usbd_class_driver_t const *usbd_app_driver_get_cb(uint8_t *driver_count)
{
    if (_active_ops != NULL && _active_ops->class_driver != NULL)
    {
        const usbd_class_driver_t *driver = _active_ops->class_driver();
        if (driver != NULL)
        {
            *driver_count += 1;
            return driver;
        }
    }
    return NULL;
}
