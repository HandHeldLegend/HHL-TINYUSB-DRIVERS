/*
 * HHL-TINYUSB-DRIVERS - TinyUSB callback dispatch.
 *
 * Copyright (c) 2026 Hand Held Legend, LLC
 */

#include "hhl_tusb_callbacks.h"

#include "hhl_tusb_private.h"
#include "hhl_tusb_transport.h"
#include "hhl_tusb_webusb.h"

#include "tusb.h"
#include "class/hid/hid.h"

static const hhl_tusb_driver_ops_s *_active_ops(void)
{
    return hhl_tusb_active_ops();
}

uint8_t const *hhl_tusb_cb_descriptor_device(void)
{
    const hhl_tusb_driver_ops_s *ops = _active_ops();
    if (ops != NULL && ops->device_descriptor != NULL)
    {
        return ops->device_descriptor();
    }
    return NULL;
}

uint8_t const *hhl_tusb_cb_descriptor_configuration(uint8_t index)
{
    const hhl_tusb_driver_ops_s *ops = _active_ops();
    if (ops != NULL && ops->configuration_descriptor != NULL)
    {
        return ops->configuration_descriptor(index);
    }
    return NULL;
}

uint8_t const *hhl_tusb_cb_descriptor_bos(void)
{
    const hhl_tusb_driver_ops_s *ops = _active_ops();
    if (ops != NULL && ops->bos_descriptor != NULL)
    {
        const uint8_t *bos = ops->bos_descriptor();
        if (bos != NULL)
        {
            return bos;
        }
    }
    return NULL;
}

uint16_t const *hhl_tusb_cb_descriptor_string(uint8_t index, uint16_t langid)
{
    const hhl_tusb_driver_ops_s *ops = _active_ops();
    if (ops != NULL && ops->descriptor_string != NULL)
    {
        return ops->descriptor_string(index, langid);
    }
    return NULL;
}

uint8_t const *hhl_tusb_cb_hid_descriptor_report(uint8_t instance)
{
    const hhl_tusb_driver_ops_s *ops = _active_ops();
    if (ops != NULL && ops->hid_report_descriptor != NULL)
    {
        return ops->hid_report_descriptor(instance);
    }
    return NULL;
}

bool hhl_tusb_cb_vendor_control_xfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    const hhl_tusb_driver_ops_s *ops = _active_ops();
    if (ops != NULL && ops->vendor_control_xfer != NULL)
    {
        return ops->vendor_control_xfer(rhport, stage, request);
    }
    return false;
}

//--------------------------------------------------------------------+
// TinyUSB application callbacks (owned by this library)
//--------------------------------------------------------------------+

uint8_t const *tud_descriptor_device_cb(void)
{
    return hhl_tusb_cb_descriptor_device();
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    return hhl_tusb_cb_descriptor_configuration(index);
}

uint8_t const *tud_descriptor_bos_cb(void)
{
    return hhl_tusb_cb_descriptor_bos();
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    return hhl_tusb_cb_descriptor_string(index, langid);
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    return hhl_tusb_cb_hid_descriptor_report(instance);
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    return hhl_tusb_cb_vendor_control_xfer(rhport, stage, request);
}

void tud_vendor_rx_cb(uint8_t itf, uint8_t const *buffer, uint16_t bufsize)
{
    (void)itf;
    (void)buffer;
    (void)bufsize;

    uint8_t rx_buf[HHL_TUSB_WEBUSB_REPORT_SIZE];
    uint32_t size = tud_vendor_n_read(HHL_TUSB_WEBUSB_VENDOR_INSTANCE, rx_buf, sizeof(rx_buf));
    tud_vendor_n_read_flush(HHL_TUSB_WEBUSB_VENDOR_INSTANCE);

    void (*handler)(const uint8_t *, uint16_t) = hhl_tusb_hook_vendor_rx();
    if (handler != NULL && size > 0)
    {
        handler(rx_buf, (uint16_t)size);
    }
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type,
                               uint8_t *buffer, uint16_t reqlen)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len)
{
    (void)instance;
    (void)report;
    (void)len;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type,
                           uint8_t const *buffer, uint16_t bufsize)
{
    (void)instance;
    if (report_id == 0 && report_type == HID_REPORT_TYPE_OUTPUT)
    {
        void (*handler)(const uint8_t *, uint16_t) = hhl_tusb_hook_hid_output_report();
        if (handler != NULL)
        {
            handler(buffer, bufsize);
        }
    }
}

void tud_mount_cb(void)
{
    hhl_tusb_sof_cb_enable(false);
    hhl_tusb_sof_cb_enable(true);

    void (*on_mount)(void) = hhl_tusb_hook_on_mount();
    if (on_mount != NULL)
    {
        on_mount();
    }
}

void tud_sof_cb(uint32_t frame_count)
{
    void (*on_sof)(uint32_t) = hhl_tusb_hook_on_sof();
    if (on_sof != NULL)
    {
        on_sof(frame_count);
    }
}
