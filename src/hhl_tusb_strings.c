/*
 * HHL-TINYUSB-DRIVERS - Shared USB string descriptor builder.
 *
 * Copyright (c) 2026 Hand Held Legend, LLC
 */

#include <string.h>

#include "hhl_tusb_strings.h"

#include "tusb.h"

#define HHL_TUSB_STRING_INDEX_LANGUAGE    0
#define HHL_TUSB_STRING_INDEX_MANUFACTURER 1
#define HHL_TUSB_STRING_INDEX_PRODUCT      2
#define HHL_TUSB_STRING_INDEX_SERIAL       3
#define HHL_TUSB_MS_OS_STRING_INDEX        0xEE
#define HHL_TUSB_VENDOR_REQUEST_GET_MS_OS_DESCRIPTOR 7

// English (US) language ID for string index 0.
static const uint8_t _language_id[] = {0x09, 0x04};

// Microsoft OS 1.0 string descriptor (index 0xEE).
static const uint8_t _ms_os_string_desc[] = {
    0x12,
    TUSB_DESC_STRING,
    'M', 0x00, 'S', 0x00, 'F', 0x00, 'T', 0x00, '1', 0x00, '0', 0x00, '0', 0x00,
    HHL_TUSB_VENDOR_REQUEST_GET_MS_OS_DESCRIPTOR,
    0x00,
};

static hhl_tusb_strings_config_s _strings = {0};
static uint16_t _desc_str[64];
static uint16_t _ms_os_string_u16[sizeof(_ms_os_string_desc) / 2];

static const char *_string_for_index(uint8_t index)
{
    switch (index)
    {
    case HHL_TUSB_STRING_INDEX_MANUFACTURER:
        return _strings.manufacturer;
    case HHL_TUSB_STRING_INDEX_PRODUCT:
        return _strings.product;
    case HHL_TUSB_STRING_INDEX_SERIAL:
        return _strings.serial_number;
    default:
        return NULL;
    }
}

void hhl_tusb_strings_configure(const hhl_tusb_strings_config_s *config)
{
    if (config == NULL)
    {
        memset(&_strings, 0, sizeof(_strings));
        return;
    }
    _strings = *config;
}

uint16_t const *hhl_tusb_strings_descriptor_cb(uint8_t index, uint16_t langid)
{
    (void)langid;

    if (index == HHL_TUSB_MS_OS_STRING_INDEX)
    {
        for (uint8_t i = 0, j = 0; i < sizeof(_ms_os_string_desc); i += 2, j++)
        {
            _ms_os_string_u16[j] = (uint16_t)(_ms_os_string_desc[i + 1] << 8) | _ms_os_string_desc[i];
        }
        memcpy(&_desc_str[0], _ms_os_string_u16, sizeof(_ms_os_string_u16));
        return _desc_str;
    }

    uint8_t chr_count = 0;

    if (index == HHL_TUSB_STRING_INDEX_LANGUAGE)
    {
        memcpy(&_desc_str[1], _language_id, sizeof(_language_id));
        chr_count = 1;
    }
    else
    {
        const char *str = _string_for_index(index);
        if (str == NULL)
        {
            return NULL;
        }

        chr_count = (uint8_t)strlen(str);
        if (chr_count > 31)
        {
            chr_count = 31;
        }
        for (uint8_t i = 0; i < chr_count; i++)
        {
            _desc_str[1 + i] = (uint16_t)str[i];
        }
    }

    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
    return _desc_str;
}
