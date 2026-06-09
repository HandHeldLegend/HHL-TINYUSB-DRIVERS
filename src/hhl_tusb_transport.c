/*
 * HHL-TINYUSB-DRIVERS - Portable transport / report I/O layer.
 *
 * Copyright (c) 2026 Hand Held Legend, LLC
 */

#include "hhl_tusb_transport.h"

#include "hhl_tusb_private.h"

#include "tusb.h"

#if (HHL_TUSB_DRIVER_XINPUT_ENABLE)
#include "hhl_tusb_xinput.h"
#endif

#if (HHL_TUSB_DRIVER_SLIPPI_ENABLE)
#include "hhl_tusb_slippi.h"
#endif

void hhl_tusb_task(void)
{
    tud_task();
}

bool hhl_tusb_start(void)
{
    return tusb_init();
}

void hhl_tusb_stop(void)
{
    hhl_tusb_set_active_driver(HHL_TUSB_DRIVER_NONE);
    tud_deinit(0);
}

bool hhl_tusb_report_ready(void)
{
    switch (hhl_tusb_get_active_driver())
    {
    case HHL_TUSB_DRIVER_HID:
        return tud_hid_ready();

#if (HHL_TUSB_DRIVER_XINPUT_ENABLE)
    case HHL_TUSB_DRIVER_XINPUT:
        return tud_xinput_ready();
#endif

#if (HHL_TUSB_DRIVER_SLIPPI_ENABLE)
    case HHL_TUSB_DRIVER_SLIPPI:
        return tud_slippi_ready();
#endif

    default:
        return false;
    }
}

bool hhl_tusb_report_send(uint8_t report_id, const void *report, uint16_t len)
{
    switch (hhl_tusb_get_active_driver())
    {
    case HHL_TUSB_DRIVER_HID:
        return tud_hid_report(report_id, report, len);

#if (HHL_TUSB_DRIVER_XINPUT_ENABLE)
    case HHL_TUSB_DRIVER_XINPUT:
        return tud_xinput_report(report_id, report, len);
#endif

#if (HHL_TUSB_DRIVER_SLIPPI_ENABLE)
    case HHL_TUSB_DRIVER_SLIPPI:
        return tud_slippi_report(report_id, report, len);
#endif

    default:
        return false;
    }
}

void hhl_tusb_sof_cb_enable(bool enable)
{
    tud_sof_cb_enable(enable);
}
