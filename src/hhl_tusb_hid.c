/*
 * HHL-TINYUSB-DRIVERS - Generic HID driver (Switch Pro, SInput, etc.)
 *
 * Copyright (c) 2026 Hand Held Legend, LLC
 */

#include "hhl_tusb_hid.h"

#include "hhl_tusb_strings.h"
#include "hhl_tusb_webusb.h"

#include "tusb.h"

static const uint8_t *_hid_device = NULL;
static const uint8_t *_hid_config = NULL;
static const uint8_t *_hid_report = NULL;
static bool _webusb_active = false;

static const uint8_t *_hid_device_descriptor(void)
{
    return _hid_device;
}

static const uint8_t *_hid_config_descriptor(uint8_t index)
{
    (void)index;
    return _hid_config;
}

static const uint8_t *_hid_report_descriptor(uint8_t instance)
{
    (void)instance;
    return _hid_report;
}

static const uint8_t *_hid_bos_descriptor(void)
{
    return _webusb_active ? hhl_tusb_webusb_bos_descriptor() : NULL;
}

static const uint8_t *_hid_ms_os_20_descriptor(uint16_t *len)
{
    if (_webusb_active)
    {
        if (len != NULL)
        {
            *len = hhl_tusb_webusb_ms_os_20_descriptor_len();
        }
        return hhl_tusb_webusb_ms_os_20_descriptor();
    }
    if (len != NULL)
    {
        *len = 0;
    }
    return NULL;
}

static uint16_t const *_hid_descriptor_string(uint8_t index, uint16_t langid)
{
    return hhl_tusb_strings_descriptor_cb(index, langid);
}

static bool _hid_vendor_control_xfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    if (!_webusb_active)
    {
        return false;
    }
    return hhl_tusb_webusb_vendor_control_xfer(rhport, stage, request);
}

static const hhl_tusb_driver_ops_s _hid_ops = {
    .device_descriptor        = _hid_device_descriptor,
    .configuration_descriptor = _hid_config_descriptor,
    .hid_report_descriptor    = _hid_report_descriptor,
    .bos_descriptor           = _hid_bos_descriptor,
    .ms_os_20_descriptor      = _hid_ms_os_20_descriptor,
    .descriptor_string        = _hid_descriptor_string,
    .vendor_control_xfer      = _hid_vendor_control_xfer,
    .class_driver             = NULL,
};

void hhl_tusb_hid_configure(const hhl_tusb_hid_config_s *hid, const hhl_tusb_webusb_config_s *webusb)
{
    if (hid == NULL || webusb == NULL)
    {
        _hid_device = NULL;
        _hid_config = NULL;
        _hid_report = NULL;
        _webusb_active = false;
        hhl_tusb_webusb_configure(NULL);
        return;
    }

    _hid_report = hid->report_descriptor;
    _webusb_active = webusb->enabled;

    if (webusb->enabled || hid->max_power_ma != 0)
    {
        uint16_t built_len = 0;
        const uint8_t *built = hhl_tusb_webusb_build_config(
            hid->config_descriptor, hid->config_descriptor_len, webusb->enabled, hid->max_power_ma, &built_len);
        _hid_config = (built != NULL) ? built : hid->config_descriptor;
    }
    else
    {
        _hid_config = hid->config_descriptor;
    }

    if (webusb->enabled)
    {
        const uint8_t *patched =
            hhl_tusb_webusb_patch_device_descriptor(hid->device_descriptor, hid->device_descriptor_len);
        _hid_device = (patched != NULL) ? patched : hid->device_descriptor;
        hhl_tusb_webusb_configure(webusb);
    }
    else
    {
        _hid_device = hid->device_descriptor;
        hhl_tusb_webusb_configure(NULL);
    }
}

const hhl_tusb_driver_ops_s *hhl_tusb_hid_ops(void)
{
    return &_hid_ops;
}

bool hhl_tusb_hid_webusb_active(void)
{
    return _webusb_active;
}
