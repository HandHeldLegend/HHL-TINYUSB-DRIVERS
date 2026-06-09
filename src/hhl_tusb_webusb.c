/*
 * HHL-TINYUSB-DRIVERS - WebUSB / Microsoft OS 2.0 descriptors and dynamic
 * configuration composition.
 *
 * Copyright (c) 2026 Hand Held Legend, LLC
 * Author: Mitchell Cairns
 */

#include <string.h>

#include "hhl_tusb_private.h"
#include "hhl_tusb_transport.h"
#include "hhl_tusb_webusb.h"

#include "tusb.h"

//--------------------------------------------------------------------+
// BOS descriptor (required for WebUSB)
//--------------------------------------------------------------------+

#define MS_OS_20_DESC_LEN 0xB2

// The vendor (WebUSB) interface always follows the single HID interface, so it
// is interface number 1 in the composed configuration descriptor.
#define WEBUSB_ITF_NUM_VENDOR 1

#define BOS_TOTAL_LEN (TUD_BOS_DESC_LEN + TUD_BOS_WEBUSB_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN)

static const uint8_t _webusb_desc_bos[] = {
    // total length, number of device caps
    TUD_BOS_DESCRIPTOR(BOS_TOTAL_LEN, 2),

    // Vendor Code, iLandingPage
    TUD_BOS_WEBUSB_DESCRIPTOR(HHL_TUSB_VENDOR_REQUEST_WEBUSB, 1),

    // Microsoft OS 2.0 descriptor
    TUD_BOS_MS_OS_20_DESCRIPTOR(MS_OS_20_DESC_LEN, HHL_TUSB_VENDOR_REQUEST_MICROSOFT)};

//--------------------------------------------------------------------+
// Microsoft OS 2.0 descriptor set (binds the vendor interface to WinUSB)
//--------------------------------------------------------------------+

static const uint8_t _webusb_desc_ms_os_20[] = {
    // Set header: length, type, windows version, total length
    U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR), U32_TO_U8S_LE(0x06030000),
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN),

    // Configuration subset header: length, type, configuration index, reserved, configuration total length
    U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION), 0, 0,
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A),

    // Function Subset header: length, type, first interface, reserved, subset length
    U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION), WEBUSB_ITF_NUM_VENDOR, 0,
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A - 0x08),

    // MS OS 2.0 Compatible ID descriptor: length, type, compatible ID, sub compatible ID
    U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sub-compatible

    // MS OS 2.0 Registry property descriptor: length, type
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A - 0x08 - 0x08 - 0x14), U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY),
    U16_TO_U8S_LE(0x0007), U16_TO_U8S_LE(0x002A), // wPropertyDataType, wPropertyNameLength and PropertyName "DeviceInterfaceGUIDs\0" in UTF-16
    'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
    'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00,
    0x00,
    U16_TO_U8S_LE(0x0050), // wPropertyDataLength
                           // bPropertyData: "{8B3E9D2E-7EEC-4994-AAE7-0C40DE84D36D}".
    '{', 0x00, '8', 0x00, 'B', 0x00, '3', 0x00, 'E', 0x00, '9', 0x00, 'D', 0x00, '2', 0x00, 'E', 0x00, '-', 0x00,
    '7', 0x00, 'E', 0x00, 'E', 0x00, 'C', 0x00, '-', 0x00, '4', 0x00, '9', 0x00, '9', 0x00, '4', 0x00, '-', 0x00,
    'A', 0x00, 'A', 0x00, 'E', 0x00, '7', 0x00, '-', 0x00, '0', 0x00, 'C', 0x00, '4', 0x00, '0', 0x00, 'D', 0x00,
    'E', 0x00, '8', 0x00, '4', 0x00, 'D', 0x00, '3', 0x00, '6', 0x00, 'D', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00};

TU_VERIFY_STATIC(sizeof(_webusb_desc_ms_os_20) == MS_OS_20_DESC_LEN, "Incorrect size");

//--------------------------------------------------------------------+
// Accessors
//--------------------------------------------------------------------+

const uint8_t *hhl_tusb_webusb_bos_descriptor(void)
{
    return _webusb_desc_bos;
}

uint16_t hhl_tusb_webusb_bos_descriptor_len(void)
{
    return (uint16_t)sizeof(_webusb_desc_bos);
}

const uint8_t *hhl_tusb_webusb_ms_os_20_descriptor(void)
{
    return _webusb_desc_ms_os_20;
}

uint16_t hhl_tusb_webusb_ms_os_20_descriptor_len(void)
{
    return (uint16_t)sizeof(_webusb_desc_ms_os_20);
}

//--------------------------------------------------------------------+
// Dynamic composition
//--------------------------------------------------------------------+

// Big enough for a single-HID-interface config (<=64 bytes) plus the vendor
// interface block, with headroom.
static uint8_t _composed_config[128];
static uint8_t _patched_device[32];

const uint8_t *hhl_tusb_webusb_build_config(const uint8_t *hid_config, uint16_t hid_config_len,
                                            bool append_vendor_interface, uint16_t max_power_ma, uint16_t *out_len)
{
    if (hid_config == NULL || hid_config_len < 9)
    {
        if (out_len != NULL)
        {
            *out_len = 0;
        }
        return NULL;
    }

    uint16_t total = (uint16_t)(hid_config_len + (append_vendor_interface ? HHL_TUSB_WEBUSB_ITF_DESC_LEN : 0));
    if (total > sizeof(_composed_config))
    {
        if (out_len != NULL)
        {
            *out_len = 0;
        }
        return NULL;
    }

    memcpy(_composed_config, hid_config, hid_config_len);

    if (append_vendor_interface)
    {
        // The new vendor interface number follows the existing interface(s).
        const uint8_t vendor_itf_num = _composed_config[4]; // current bNumInterfaces

        static const uint8_t vendor_block[HHL_TUSB_WEBUSB_ITF_DESC_LEN] = {
            // Vendor-specific interface descriptor (bInterfaceNumber patched below)
            9, TUSB_DESC_INTERFACE, 0x00, 0x00, 0x02, TUSB_CLASS_VENDOR_SPECIFIC, 0x00, 0x00, 0x00,
            // Bulk IN endpoint 0x82
            7, TUSB_DESC_ENDPOINT, 0x82, TUSB_XFER_BULK, U16_TO_U8S_LE(64), 0x00,
            // Bulk OUT endpoint 0x02
            7, TUSB_DESC_ENDPOINT, 0x02, TUSB_XFER_BULK, U16_TO_U8S_LE(64), 0x00,
        };

        uint8_t *p = _composed_config + hid_config_len;
        memcpy(p, vendor_block, HHL_TUSB_WEBUSB_ITF_DESC_LEN);
        p[2] = vendor_itf_num; // bInterfaceNumber

        // Fix up the configuration descriptor header.
        _composed_config[4] = (uint8_t)(vendor_itf_num + 1);  // bNumInterfaces
        _composed_config[2] = (uint8_t)(total & 0xFF);        // wTotalLength low
        _composed_config[3] = (uint8_t)((total >> 8) & 0xFF); // wTotalLength high
    }

    // Optional bMaxPower override (descriptor stores units of 2mA at offset 8).
    if (max_power_ma != 0)
    {
        uint16_t units = (uint16_t)(max_power_ma / 2u);
        if (units > 0xFF)
        {
            units = 0xFF;
        }
        _composed_config[8] = (uint8_t)units;
    }

    if (out_len != NULL)
    {
        *out_len = total;
    }
    return _composed_config;
}

const uint8_t *hhl_tusb_webusb_patch_device_descriptor(const uint8_t *device_desc, uint16_t device_desc_len)
{
    if (device_desc == NULL || device_desc_len < 4)
    {
        return NULL;
    }

    uint16_t len = device_desc_len;
    if (len > sizeof(_patched_device))
    {
        len = sizeof(_patched_device);
    }

    memcpy(_patched_device, device_desc, len);

    // bcdUSB lives at offset 2-3 (little endian). WebUSB requires >= 2.01 so
    // the host fetches the BOS descriptor.
    const uint16_t bcd_usb = (uint16_t)_patched_device[2] | ((uint16_t)_patched_device[3] << 8);
    if (bcd_usb < 0x0201)
    {
        _patched_device[2] = 0x10;
        _patched_device[3] = 0x02; // 0x0210
    }

    return _patched_device;
}

//--------------------------------------------------------------------+
// Runtime configuration + vendor control transfers
//--------------------------------------------------------------------+

#define HHL_TUSB_VENDOR_REQUEST_GET_MS_OS_DESCRIPTOR 7
#define HHL_TUSB_WEBUSB_URL_DESC_MAX 128

static hhl_tusb_webusb_config_s _webusb_runtime = {0};
static uint8_t _url_desc[HHL_TUSB_WEBUSB_URL_DESC_MAX];
static uint16_t _url_desc_len = 0;

static uint8_t _ms_os_10_compatible_id[] = {
    0x28, 0x00, 0x00, 0x00,
    0x00, 0x01,
    0x04, 0x00,
    0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00,
    0x01,
    'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static uint8_t _ms_extended_feature[] = {
    0x92, 0x00, 0x00, 0x00,
    0x00, 0x01,
    0x05, 0x00,
    0x01, 0x00,
    0x88, 0x00, 0x00, 0x00,
    0x07, 0x00, 0x00, 0x00,
    0x2A, 0x00,
    'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0,
    'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0,
    'G', 0, 'U', 0, 'I', 0, 'D', 0, 's', 0, 0x00, 0x00,
    0x50, 0x00, 0x00, 0x00,
    '{', 0, '6', 0, 'E', 0, '4', 0, '5', 0, '7', 0, '3', 0, '6', 0, 'A', 0, '-', 0,
    '2', 0, 'B', 0, '1', 0, 'B', 0, '-', 0, '4', 0, '0', 0, '7', 0, '8', 0, '-', 0,
    'B', 0, '7', 0, '7', 0, '2', 0, '-', 0, 'B', 0, '3', 0, 'A', 0, 'F', 0, '2', 0,
    'B', 0, '6', 0, 'F', 0, 'D', 0, 'E', 0, '1', 0, 'C', 0, '}', 0,
    0x00, 0x00, 0x00, 0x00,
};

void hhl_tusb_webusb_configure(const hhl_tusb_webusb_config_s *config)
{
    if (config == NULL)
    {
        memset(&_webusb_runtime, 0, sizeof(_webusb_runtime));
        _url_desc_len = 0;
        return;
    }

    _webusb_runtime = *config;
    _url_desc_len = 0;

    if (_webusb_runtime.url != NULL)
    {
        size_t url_len = strlen(_webusb_runtime.url);
        if (url_len > 0 && (3u + url_len) <= sizeof(_url_desc))
        {
            _url_desc[0] = (uint8_t)(3u + url_len);
            _url_desc[1] = 3; // WEBUSB URL type
            _url_desc[2] = 1; // https
            memcpy(&_url_desc[3], _webusb_runtime.url, url_len);
            _url_desc_len = (uint16_t)(3u + url_len);
        }
    }
}

bool hhl_tusb_webusb_vendor_control_xfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    if (stage != CONTROL_STAGE_SETUP)
    {
        return true;
    }

    if (request->bmRequestType_bit.type != TUSB_REQ_TYPE_VENDOR)
    {
        return false;
    }

    switch (request->bRequest)
    {
    case HHL_TUSB_VENDOR_REQUEST_GET_MS_OS_DESCRIPTOR:
        if (request->wIndex == 4)
        {
            return tud_control_xfer(rhport, request, _ms_os_10_compatible_id, sizeof(_ms_os_10_compatible_id));
        }
        if (request->wIndex == 5)
        {
            return tud_control_xfer(rhport, request, _ms_extended_feature, sizeof(_ms_extended_feature));
        }
        return false;

    case HHL_TUSB_VENDOR_REQUEST_WEBUSB:
        if (_webusb_runtime.popup_enable_flag != NULL && *_webusb_runtime.popup_enable_flag == 1 && _url_desc_len > 0)
        {
            return tud_control_xfer(rhport, request, _url_desc, _url_desc_len);
        }
        return false;

    case HHL_TUSB_VENDOR_REQUEST_MICROSOFT:
        if (request->wIndex == 7)
        {
            uint16_t total_len = 0;
            const uint8_t *ms_os_20 = hhl_tusb_webusb_ms_os_20_descriptor();
            memcpy(&total_len, ms_os_20 + 8, 2);
            return tud_control_xfer(rhport, request, (void *)(uintptr_t)ms_os_20, total_len);
        }
        return false;

    default:
        return false;
    }
}

//--------------------------------------------------------------------+
// WebUSB vendor bulk reporting (host firmware uses these instead of tud_vendor_*)
//--------------------------------------------------------------------+

static bool _hhl_tusb_webusb_write_available(void)
{
    if (!hhl_tusb_webusb_active())
    {
        return false;
    }
    return tud_vendor_n_write_available(HHL_TUSB_WEBUSB_VENDOR_INSTANCE);
}

bool hhl_tusb_webusb_report_ready_blocking(int timeout_ms)
{
    if (timeout_ms <= 0)
    {
        return false;
    }

    while (!_hhl_tusb_webusb_write_available() && timeout_ms > 0)
    {
        hhl_tusb_task();
        void (*sleep_ms)(uint32_t) = hhl_tusb_hook_platform_sleep_ms();
        if (sleep_ms != NULL)
        {
            sleep_ms(1);
        }
        timeout_ms--;
    }

    return _hhl_tusb_webusb_write_available();
}

bool hhl_tusb_webusb_report_send(const uint8_t *data, uint16_t len)
{
    if (!hhl_tusb_webusb_active() || data == NULL)
    {
        return false;
    }

    if (!hhl_tusb_webusb_report_ready_blocking(256))
    {
        return false;
    }

    uint8_t buf[HHL_TUSB_WEBUSB_REPORT_SIZE];
    memset(buf, 0, sizeof(buf));
    if (len > sizeof(buf))
    {
        len = sizeof(buf);
    }
    memcpy(buf, data, len);

    uint32_t written = tud_vendor_n_write(HHL_TUSB_WEBUSB_VENDOR_INSTANCE, buf, sizeof(buf));
    tud_vendor_n_flush(HHL_TUSB_WEBUSB_VENDOR_INSTANCE);
    return written > 0;
}
