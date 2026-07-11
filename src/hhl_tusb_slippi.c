#include "hhl_tusb_slippi.h"
#include "hhl_tusb_strings.h"
#include "hhl_tusb_webusb.h"

#if (HHL_TUSB_DRIVER_SLIPPI_ENABLE)

#include <string.h>
#include <stddef.h>

#ifndef CFG_TUD_GC
#define CFG_TUD_GC 1
#endif

#ifndef CFG_TUD_GC_TX_BUFSIZE
#define CFG_TUD_GC_TX_BUFSIZE 37
#endif

#ifndef CFG_TUD_GC_RX_BUFSIZE
#define CFG_TUD_GC_RX_BUFSIZE 6
#endif

//--------------------------------------------------------------------+
// Descriptors
//--------------------------------------------------------------------+

/**** GameCube Adapter HID Report Descriptor ****/
static const uint8_t _gc_hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0xA1, 0x03,        //   Collection (Report)
    0x85, 0x11,        //     Report ID (17)
    0x19, 0x00,        //     Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //     Usage Maximum (0xFF)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x05,        //     Report Count (5)
    0x91, 0x00,        //     Output (Data,Array,Abs)
    0xC0,              //   End Collection
    0xA1, 0x03,        //   Collection (Report)
    0x85, 0x21,        //     Report ID (33)
    0x05, 0x00,        //     Usage Page (Undefined)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0xFF,        //     Logical Maximum (-1)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x02,        //     Input (Data,Var,Abs)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x08,        //     Usage Maximum (0x08)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x02,        //     Input (Data,Var,Abs)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x32,        //     Usage (Z)
    0x09, 0x33,        //     Usage (Rx)
    0x09, 0x34,        //     Usage (Ry)
    0x09, 0x35,        //     Usage (Rz)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x06,        //     Report Count (6)
    0x81, 0x02,        //     Input (Data,Var,Abs)
    0xC0,              //   End Collection
    0xA1, 0x03,        //   Collection (Report)
    0x85, 0x13,        //     Report ID (19)
    0x19, 0x00,        //     Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //     Usage Maximum (0xFF)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x91, 0x00,        //     Output (Data,Array,Abs)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};

/**** GameCube Adapter Device Descriptor ****/
static const tusb_desc_device_t _slippi_device_descriptor = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = HHL_TUSB_SLIPPI_VID,
    .idProduct          = HHL_TUSB_SLIPPI_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01,
};

/**** GameCube Adapter Configuration Descriptor ****/
#define SLIPPI_CONFIG_DESCRIPTOR_LEN 41
static const uint8_t _slippi_configuration_descriptor[SLIPPI_CONFIG_DESCRIPTOR_LEN] = {
    // Configuration: number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, SLIPPI_CONFIG_DESCRIPTOR_LEN, TUSB_DESC_CONFIG_ATT_SELF_POWERED, 500),

    // Interface
    9, TUSB_DESC_INTERFACE, 0x00, 0x00, 0x02, TUSB_CLASS_HID, 0x00, 0x00, 0x00,
    // HID Descriptor
    9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0110), 0x00, 0x01, HID_DESC_TYPE_REPORT, U16_TO_U8S_LE(sizeof(_gc_hid_report_descriptor)),
    // Endpoint Descriptor (IN)
    7, TUSB_DESC_ENDPOINT, 0x82, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(37), 1,
    // Endpoint Descriptor (OUT)
    7, TUSB_DESC_ENDPOINT, 0x01, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(6), 1,
};

/**** GameCube Adapter BOS Descriptor ****/
static const uint8_t _gc_desc_bos[] = {
    // BOS descriptor
    0x05,       // Descriptor size (5 bytes)
    0x0F,       // Descriptor type (BOS)
    0x21, 0x00, // Length of this + subordinate descriptors (33 bytes)
    0x01,       // Number of subordinate descriptors

    // Microsoft OS 2.0 Platform Capability Descriptor
    0x1C, // Descriptor size (28 bytes)
    0x10, // Descriptor type (Device Capability)
    0x05, // Capability type (Platform)
    0x00, // Reserved

    // MS OS 2.0 Platform Capability ID (D8DD60DF-4589-4CC7-9CD2-659D9E648A9F)
    0xDF, 0x60, 0xDD, 0xD8,
    0x89, 0x45,
    0xC7, 0x4C,
    0x9C, 0xD2,
    0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F,

    0x00, 0x00, 0x03, 0x06, // Windows version (8.1) (0x06030000)
    0x9E, 0x00,             // Size, MS OS 2.0 descriptor set (158 bytes)
    0x02,                   // bMS_VendorCode (VENDOR_REQUEST_MICROSOFT)
    0x00                    // Doesn't support alternate enumeration
};

/**** GameCube Adapter Microsoft OS 2.0 Descriptor Set ****/
#define GC_MS_OS_20_DESC_LEN 158
static const uint8_t _gc_desc_ms_os_20[GC_MS_OS_20_DESC_LEN] = {
    0x0A, 0x00,             // Descriptor size (10 bytes)
    0x00, 0x00,             // MS OS 2.0 descriptor set header
    0x00, 0x00, 0x03, 0x06, // Windows version (8.1) (0x06030000)
    0x9E, 0x00,             // Size, MS OS 2.0 descriptor set (158 bytes)

    // Microsoft OS 2.0 compatible ID descriptor
    0x14, 0x00,                                     // Descriptor size (20 bytes)
    0x03, 0x00,                                     // MS OS 2.0 compatible ID descriptor
    0x57, 0x49, 0x4E, 0x55, 0x53, 0x42, 0x00, 0x00, // WINUSB string
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Sub-compatible ID

    // Registry property descriptor
    0x80, 0x00, // Descriptor size (130 bytes)
    0x04, 0x00, // Registry Property descriptor
    0x01, 0x00, // Strings are null-terminated Unicode
    0x28, 0x00, // Size of Property Name (40 bytes)

    // Property Name ("DeviceInterfaceGUID")
    0x44, 0x00, 0x65, 0x00, 0x76, 0x00, 0x69, 0x00, 0x63, 0x00, 0x65, 0x00,
    0x49, 0x00, 0x6E, 0x00, 0x74, 0x00, 0x65, 0x00, 0x72, 0x00, 0x66, 0x00,
    0x61, 0x00, 0x63, 0x00, 0x65, 0x00, 0x47, 0x00, 0x55, 0x00, 0x49, 0x00,
    0x44, 0x00, 0x00, 0x00,

    0x4E, 0x00, // Size of Property Data (78 bytes)

    // Vendor-defined Property Data: {ecceff35-146c-4ff3-acd9-8f992d09acdd}
    0x7B, 0x00, 0x65, 0x00, 0x63, 0x00, 0x63, 0x00, 0x65, 0x00, 0x66, 0x00,
    0x66, 0x00, 0x33, 0x00, 0x35, 0x00, 0x2D, 0x00, 0x31, 0x00, 0x34, 0x00,
    0x36, 0x00, 0x33, 0x00, 0x2D, 0x00, 0x34, 0x00, 0x66, 0x00, 0x66, 0x00,
    0x33, 0x00, 0x2D, 0x00, 0x61, 0x00, 0x63, 0x00, 0x64, 0x00, 0x39, 0x00,
    0x2D, 0x00, 0x38, 0x00, 0x66, 0x00, 0x39, 0x00, 0x39, 0x00, 0x32, 0x00,
    0x64, 0x00, 0x30, 0x00, 0x39, 0x00, 0x61, 0x00, 0x63, 0x00, 0x64, 0x00,
    0x64, 0x00, 0x7D, 0x00, 0x00, 0x00};

TU_VERIFY_STATIC(sizeof(_gc_desc_ms_os_20) == GC_MS_OS_20_DESC_LEN, "Incorrect size");

// Microsoft OS 1.0 descriptors (string 0xEE / vendor request 7). Required for
// WinUSB auto-association when the host does not fetch BOS (bcdUSB 0x0200).
#define HHL_TUSB_VENDOR_REQUEST_GET_MS_OS_DESCRIPTOR 7

static uint8_t _ms_os_10_compatible_id[] = {
    0x28, 0x00, 0x00, 0x00, // Descriptor length (40 bytes)
    0x00, 0x01,             // Version 1.0
    0x04, 0x00,             // Compatibility ID index
    0x01,                   // Number of sections
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00,                   // Interface number (IF0)
    0x01,                   // Reserved
    'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// DeviceInterfaceGUIDs with Dolphin/Slippi GUID {ecceff35-146c-4ff3-acd9-8f992d09acdd}
static uint8_t _ms_os_10_extended_feature[] = {
    0x92, 0x00, 0x00, 0x00, // Descriptor length (146 bytes)
    0x00, 0x01,             // Version 1.0
    0x05, 0x00,             // Extended property index
    0x01, 0x00,             // Number of sections
    0x88, 0x00, 0x00, 0x00, // Size of property section (136 bytes)
    0x07, 0x00, 0x00, 0x00, // Property data type (REG_MULTI_SZ)
    0x2A, 0x00,             // Property name length
    'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0,
    'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0,
    'G', 0, 'U', 0, 'I', 0, 'D', 0, 's', 0, 0x00, 0x00,
    0x50, 0x00, 0x00, 0x00, // Property data length (80 bytes)
    '{', 0, 'e', 0, 'c', 0, 'c', 0, 'e', 0, 'f', 0, 'f', 0, '3', 0, '5', 0, '-', 0,
    '1', 0, '4', 0, '6', 0, 'c', 0, '-', 0, '4', 0, 'f', 0, 'f', 0, '3', 0, '-', 0,
    'a', 0, 'c', 0, 'd', 0, '9', 0, '-', 0, '8', 0, 'f', 0, '9', 0, '9', 0, '2', 0,
    'd', 0, '0', 0, '9', 0, 'a', 0, 'c', 0, 'd', 0, 'd', 0, '}', 0,
    0x00, 0x00, 0x00, 0x00,
};

const uint8_t *hhl_tusb_slippi_device_descriptor(void)
{
    return (const uint8_t *)&_slippi_device_descriptor;
}

uint16_t hhl_tusb_slippi_device_descriptor_len(void)
{
    return (uint16_t)sizeof(_slippi_device_descriptor);
}

const uint8_t *hhl_tusb_slippi_configuration_descriptor(void)
{
    return _slippi_configuration_descriptor;
}

uint16_t hhl_tusb_slippi_configuration_descriptor_len(void)
{
    return (uint16_t)sizeof(_slippi_configuration_descriptor);
}

const uint8_t *hhl_tusb_slippi_hid_report_descriptor(void)
{
    return _gc_hid_report_descriptor;
}

uint16_t hhl_tusb_slippi_hid_report_descriptor_len(void)
{
    return (uint16_t)sizeof(_gc_hid_report_descriptor);
}

const uint8_t *hhl_tusb_slippi_bos_descriptor(void)
{
    return _gc_desc_bos;
}

uint16_t hhl_tusb_slippi_bos_descriptor_len(void)
{
    return (uint16_t)sizeof(_gc_desc_bos);
}

const uint8_t *hhl_tusb_slippi_ms_os_20_descriptor(void)
{
    return _gc_desc_ms_os_20;
}

uint16_t hhl_tusb_slippi_ms_os_20_descriptor_len(void)
{
    return (uint16_t)sizeof(_gc_desc_ms_os_20);
}

//--------------------------------------------------------------------+
// Class driver state
//--------------------------------------------------------------------+

typedef struct
{
    uint8_t itf_num;
    uint8_t ep_in;
    uint8_t ep_out;
    uint8_t itf_protocol;

    uint8_t protocol_mode;
    uint8_t idle_rate;
    uint16_t report_desc_len;

    CFG_TUSB_MEM_ALIGN uint8_t epin_buf[CFG_TUD_GC_TX_BUFSIZE];
    CFG_TUSB_MEM_ALIGN uint8_t epout_buf[CFG_TUD_GC_RX_BUFSIZE];

    tusb_hid_descriptor_hid_t const *hid_descriptor;
} slippid_interface_t;

CFG_TUSB_MEM_SECTION static slippid_interface_t _slippid_itf[CFG_TUD_GC];

static inline uint8_t slippi_get_index_by_itfnum(uint8_t itf_num)
{
    for (uint8_t i = 0; i < CFG_TUD_GC; i++)
    {
        if (itf_num == _slippid_itf[i].itf_num)
            return i;
    }

    return 0xFF;
}

//--------------------------------------------------------------------+
// Application API
//--------------------------------------------------------------------+

bool tud_slippi_n_ready(uint8_t instance)
{
    uint8_t const rhport = 0;
    uint8_t const ep_in = _slippid_itf[instance].ep_in;
    return tud_ready() && (ep_in != 0) && !usbd_edpt_busy(rhport, ep_in);
}

bool tud_slippi_ready(void)
{
    return tud_slippi_n_ready(0);
}

bool tud_slippi_n_report(uint8_t instance, uint8_t report_id, void const *report, uint16_t len)
{
    (void)len;
    uint8_t const rhport = 0;
    slippid_interface_t *p_hid = &_slippid_itf[instance];

    TU_VERIFY(usbd_edpt_claim(rhport, p_hid->ep_in));

    p_hid->epin_buf[0] = report_id;
    memcpy(p_hid->epin_buf + 1, report, CFG_TUD_GC_TX_BUFSIZE - 1);

    return usbd_edpt_xfer(rhport, p_hid->ep_in, p_hid->epin_buf, CFG_TUD_GC_TX_BUFSIZE);
}

bool tud_slippi_report(uint8_t report_id, void const *report, uint16_t len)
{
    return tud_slippi_n_report(0, report_id, report, len);
}

uint8_t tud_slippi_n_interface_protocol(uint8_t instance)
{
    return _slippid_itf[instance].itf_protocol;
}

uint8_t tud_slippi_n_get_protocol(uint8_t instance)
{
    return _slippid_itf[instance].protocol_mode;
}

//--------------------------------------------------------------------+
// USBD-CLASS API
//--------------------------------------------------------------------+

static void slippid_reset(uint8_t rhport)
{
    (void)rhport;
    tu_memclr(_slippid_itf, sizeof(_slippid_itf));
}

static void slippid_init(void)
{
    slippid_reset(0);
}

static uint16_t slippid_open(uint8_t rhport, tusb_desc_interface_t const *desc_itf, uint16_t max_len)
{
    // len = interface + hid + n*endpoints
    uint16_t const drv_len = (uint16_t)(sizeof(tusb_desc_interface_t) + sizeof(tusb_hid_descriptor_hid_t) +
                                        desc_itf->bNumEndpoints * sizeof(tusb_desc_endpoint_t));
    TU_ASSERT(max_len >= drv_len, 0);

    // Find available interface
    slippid_interface_t *p_hid = NULL;
    uint8_t hid_id;
    for (hid_id = 0; hid_id < CFG_TUD_GC; hid_id++)
    {
        if (_slippid_itf[hid_id].ep_in == 0)
        {
            p_hid = &_slippid_itf[hid_id];
            break;
        }
    }
    TU_ASSERT(p_hid, 0);

    uint8_t const *p_desc = (uint8_t const *)desc_itf;

    //------------- HID descriptor -------------//
    p_desc = tu_desc_next(p_desc);
    TU_ASSERT(HID_DESC_TYPE_HID == tu_desc_type(p_desc), 0);
    p_hid->hid_descriptor = (tusb_hid_descriptor_hid_t const *)p_desc;

    //------------- Endpoint Descriptor -------------//
    p_desc = tu_desc_next(p_desc);
    TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, desc_itf->bNumEndpoints, TUSB_XFER_INTERRUPT, &p_hid->ep_out, &p_hid->ep_in), 0);

    if (desc_itf->bInterfaceSubClass == HID_SUBCLASS_BOOT)
        p_hid->itf_protocol = desc_itf->bInterfaceProtocol;

    p_hid->protocol_mode = HID_PROTOCOL_REPORT; // Per Specs: default is report mode
    p_hid->itf_num = desc_itf->bInterfaceNumber;

    // Use offsetof to avoid pointer to the odd/misaligned address
    p_hid->report_desc_len = tu_unaligned_read16((uint8_t const *)p_hid->hid_descriptor + offsetof(tusb_hid_descriptor_hid_t, wReportLength));

    // Prepare for output endpoint
    if (p_hid->ep_out)
    {
        if (!usbd_edpt_xfer(rhport, p_hid->ep_out, p_hid->epout_buf, sizeof(p_hid->epout_buf)))
        {
            TU_LOG_FAILED();
            TU_BREAKPOINT();
        }
    }

    return drv_len;
}

static bool slippid_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    TU_VERIFY(request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_INTERFACE);

    uint8_t const hid_itf = slippi_get_index_by_itfnum((uint8_t)request->wIndex);
    TU_VERIFY(hid_itf < CFG_TUD_GC);

    slippid_interface_t *p_hid = &_slippid_itf[hid_itf];

    if (request->bmRequestType_bit.type == TUSB_REQ_TYPE_STANDARD)
    {
        //------------- STD Request -------------//
        if (stage == CONTROL_STAGE_SETUP)
        {
            uint8_t const desc_type = tu_u16_high(request->wValue);

            if (request->bRequest == TUSB_REQ_GET_DESCRIPTOR && desc_type == HID_DESC_TYPE_HID)
            {
                TU_VERIFY(p_hid->hid_descriptor);
                TU_VERIFY(tud_control_xfer(rhport, request, (void *)(uintptr_t)p_hid->hid_descriptor, p_hid->hid_descriptor->bLength));
            }
            else if (request->bRequest == TUSB_REQ_GET_DESCRIPTOR && desc_type == HID_DESC_TYPE_REPORT)
            {
                tud_control_xfer(rhport, request, (void *)(uintptr_t)hhl_tusb_slippi_hid_report_descriptor(), p_hid->report_desc_len);
            }
            else
            {
                return false; // stall unsupported request
            }
        }
    }
    else if (request->bmRequestType_bit.type == TUSB_REQ_TYPE_CLASS)
    {
        //------------- Class Specific Request -------------//
        switch (request->bRequest)
        {
        case HID_REQ_CONTROL_GET_REPORT:
            if (stage == CONTROL_STAGE_SETUP)
            {
                uint8_t const report_type = tu_u16_high(request->wValue);
                uint8_t const report_id = tu_u16_low(request->wValue);

                uint8_t *report_buf = p_hid->epin_buf;
                uint16_t req_len = tu_min16(request->wLength, CFG_TUD_GC_TX_BUFSIZE);

                uint16_t xferlen = 0;

                if ((report_id != HID_REPORT_TYPE_INVALID) && (req_len > 1))
                {
                    *report_buf++ = report_id;
                    req_len--;

                    xferlen++;
                }

                xferlen += tud_hid_get_report_cb(hid_itf, report_id, (hid_report_type_t)report_type, report_buf, req_len);
                TU_ASSERT(xferlen > 0);

                tud_control_xfer(rhport, request, p_hid->epin_buf, xferlen);
            }
            break;

        case HID_REQ_CONTROL_SET_REPORT:
            if (stage == CONTROL_STAGE_SETUP)
            {
                TU_VERIFY(request->wLength <= sizeof(p_hid->epout_buf));
                tud_control_xfer(rhport, request, p_hid->epout_buf, request->wLength);
            }
            else if (stage == CONTROL_STAGE_ACK)
            {
                uint8_t const report_type = tu_u16_high(request->wValue);
                uint8_t const report_id = tu_u16_low(request->wValue);

                uint8_t const *report_buf = p_hid->epout_buf;
                uint16_t report_len = tu_min16(request->wLength, CFG_TUD_GC_RX_BUFSIZE);

                if ((report_id != HID_REPORT_TYPE_INVALID) && (report_len > 1) && (report_id == report_buf[0]))
                {
                    report_buf++;
                    report_len--;
                }

                tud_hid_set_report_cb(hid_itf, report_id, (hid_report_type_t)report_type, report_buf, report_len);
            }
            break;

        case HID_REQ_CONTROL_SET_IDLE:
            if (stage == CONTROL_STAGE_SETUP)
            {
                p_hid->idle_rate = tu_u16_high(request->wValue);
                if (tud_hid_set_idle_cb)
                {
                    TU_VERIFY(tud_hid_set_idle_cb(hid_itf, p_hid->idle_rate));
                }

                tud_control_status(rhport, request);
            }
            break;

        case HID_REQ_CONTROL_GET_IDLE:
            if (stage == CONTROL_STAGE_SETUP)
            {
                tud_control_xfer(rhport, request, &p_hid->idle_rate, 1);
            }
            break;

        case HID_REQ_CONTROL_GET_PROTOCOL:
            if (stage == CONTROL_STAGE_SETUP)
            {
                tud_control_xfer(rhport, request, &p_hid->protocol_mode, 1);
            }
            break;

        case HID_REQ_CONTROL_SET_PROTOCOL:
            if (stage == CONTROL_STAGE_SETUP)
            {
                tud_control_status(rhport, request);
            }
            else if (stage == CONTROL_STAGE_ACK)
            {
                p_hid->protocol_mode = (uint8_t)request->wValue;
                if (tud_hid_set_protocol_cb)
                {
                    tud_hid_set_protocol_cb(hid_itf, p_hid->protocol_mode);
                }
            }
            break;

        default:
            return false; // stall unsupported request
        }
    }
    else
    {
        return false; // stall unsupported request
    }

    return true;
}

static bool slippid_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes)
{
    (void)result;

    uint8_t instance = 0;
    slippid_interface_t *p_hid = _slippid_itf;

    for (instance = 0; instance < CFG_TUD_GC; instance++)
    {
        p_hid = &_slippid_itf[instance];
        if ((ep_addr == p_hid->ep_out) || (ep_addr == p_hid->ep_in))
            break;
    }
    TU_ASSERT(instance < CFG_TUD_GC);

    // Sent report successfully
    if (ep_addr == p_hid->ep_in)
    {
        if (tud_hid_report_complete_cb)
        {
            tud_hid_report_complete_cb(instance, p_hid->epin_buf, (uint16_t)xferred_bytes);
        }
    }
    // Received report
    else if (ep_addr == p_hid->ep_out)
    {
        tud_hid_set_report_cb(instance, 0, HID_REPORT_TYPE_OUTPUT, p_hid->epout_buf, (uint16_t)xferred_bytes);
        TU_ASSERT(usbd_edpt_xfer(rhport, p_hid->ep_out, p_hid->epout_buf, sizeof(p_hid->epout_buf)));
    }

    return true;
}

static const usbd_class_driver_t _tud_slippi_driver =
    {
#if CFG_TUSB_DEBUG >= 2
        .name = "slippi",
#endif
        .init            = slippid_init,
        .reset           = slippid_reset,
        .open            = slippid_open,
        .control_xfer_cb = slippid_control_xfer_cb,
        .xfer_cb         = slippid_xfer_cb,
        .sof             = NULL,
};

const usbd_class_driver_t *hhl_tusb_slippi_driver(void)
{
    return &_tud_slippi_driver;
}

//--------------------------------------------------------------------+
// Driver ops
//--------------------------------------------------------------------+

static const uint8_t *_slippi_ops_config_descriptor(uint8_t index)
{
    (void)index;
    return hhl_tusb_slippi_configuration_descriptor();
}

static const uint8_t *_slippi_hid_report_descriptor(uint8_t instance)
{
    (void)instance;
    return hhl_tusb_slippi_hid_report_descriptor();
}

static const uint8_t *_slippi_ms_os_20_descriptor(uint16_t *len)
{
    if (len != NULL)
    {
        *len = hhl_tusb_slippi_ms_os_20_descriptor_len();
    }
    return hhl_tusb_slippi_ms_os_20_descriptor();
}

static bool _slippi_vendor_control_xfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
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
        // MS OS 1.0 (advertised by string 0xEE / MSFT100)
        if (request->wIndex == 4)
        {
            return tud_control_xfer(rhport, request, _ms_os_10_compatible_id, sizeof(_ms_os_10_compatible_id));
        }
        if (request->wIndex == 5)
        {
            return tud_control_xfer(rhport, request, _ms_os_10_extended_feature, sizeof(_ms_os_10_extended_feature));
        }
        return false;

    case HHL_TUSB_VENDOR_REQUEST_MICROSOFT:
        // MS OS 2.0 (advertised by BOS platform capability)
        if (request->wIndex == 7)
        {
            uint16_t total_len = 0;
            const uint8_t *ms_os_20 = hhl_tusb_slippi_ms_os_20_descriptor();
            memcpy(&total_len, ms_os_20 + 8, 2);
            return tud_control_xfer(rhport, request, (void *)(uintptr_t)ms_os_20, total_len);
        }
        return false;

    default:
        return false;
    }
}

static const hhl_tusb_driver_ops_s _slippi_ops = {
    .device_descriptor        = hhl_tusb_slippi_device_descriptor,
    .configuration_descriptor = _slippi_ops_config_descriptor,
    .hid_report_descriptor    = _slippi_hid_report_descriptor,
    .bos_descriptor           = hhl_tusb_slippi_bos_descriptor,
    .ms_os_20_descriptor      = _slippi_ms_os_20_descriptor,
    .descriptor_string        = hhl_tusb_strings_descriptor_cb,
    .vendor_control_xfer      = _slippi_vendor_control_xfer,
    .class_driver             = hhl_tusb_slippi_driver,
};

const hhl_tusb_driver_ops_s *hhl_tusb_slippi_ops(void)
{
    return &_slippi_ops;
}

const hhl_tusb_driver_ops_s *hhl_tusb_slippi_provider(void)
{
    return hhl_tusb_slippi_ops();
}

#endif /* HHL_TUSB_DRIVER_SLIPPI_ENABLE */
