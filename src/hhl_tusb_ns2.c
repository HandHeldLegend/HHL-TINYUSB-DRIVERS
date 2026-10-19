/*
 * HHL-TINYUSB-DRIVERS - Switch 2 controller over USB (HID + vendor bulk commands).
 *
 * Copyright (c) 2026 Hand Held Legend, LLC
 */

#include "hhl_tusb_ns2.h"
#include "hhl_tusb_private.h"
#include "hhl_tusb_strings.h"

#if (HHL_TUSB_DRIVER_NS2_ENABLE)

#include <string.h>

#define NS2_EP_SIZE 64
// Room for the longest reply (a memory read: header, arguments and 0x78 bytes)
#define NS2_REPLY_MAX 192
// The console waits for each reply, but answers can still meet a reply in flight
#define NS2_REPLY_QUEUE 3

typedef struct
{
    uint8_t itf_hid;
    uint8_t ep_in;
    uint8_t ep_out;
    uint8_t bulk_in;
    uint8_t bulk_out;
    bool zlp_pending;

    CFG_TUSB_MEM_ALIGN uint8_t in_buf[NS2_EP_SIZE];
    CFG_TUSB_MEM_ALIGN uint8_t out_buf[NS2_EP_SIZE];
    CFG_TUSB_MEM_ALIGN uint8_t cmd_buf[NS2_EP_SIZE];
    CFG_TUSB_MEM_ALIGN uint8_t reply_buf[NS2_REPLY_MAX];
    CFG_TUSB_MEM_ALIGN uint8_t ctrl_buf[NS2_EP_SIZE];
} ns2_interface_t;

typedef struct
{
    uint16_t len;
    uint8_t data[NS2_REPLY_MAX];
} ns2_reply_t;

CFG_TUSB_MEM_SECTION static ns2_interface_t _ns2_itf;
static hhl_tusb_ns2_config_s _ns2_cfg;

static ns2_reply_t _ns2_replies[NS2_REPLY_QUEUE];
static uint8_t _ns2_reply_head = 0;
static uint8_t _ns2_reply_count = 0;

void hhl_tusb_ns2_configure(const hhl_tusb_ns2_config_s *cfg)
{
    if (cfg)
        _ns2_cfg = *cfg;
    else
        memset(&_ns2_cfg, 0, sizeof(_ns2_cfg));
}

//--------------------------------------------------------------------+
// Replies on the bulk IN endpoint
//--------------------------------------------------------------------+

static bool _ns2_reply_start(uint8_t rhport)
{
    if (!_ns2_reply_count || !_ns2_itf.bulk_in)
        return false;
    if (!usbd_edpt_claim(rhport, _ns2_itf.bulk_in))
        return false;

    ns2_reply_t *r = &_ns2_replies[_ns2_reply_head];
    memcpy(_ns2_itf.reply_buf, r->data, r->len);
    // A reply that fills its last packet needs a zero-length packet to end it
    _ns2_itf.zlp_pending = r->len && (r->len % NS2_EP_SIZE) == 0;
    uint16_t len = r->len;

    _ns2_reply_head = (uint8_t)((_ns2_reply_head + 1) % NS2_REPLY_QUEUE);
    _ns2_reply_count--;

    return usbd_edpt_xfer(rhport, _ns2_itf.bulk_in, _ns2_itf.reply_buf, len);
}

bool hhl_tusb_ns2_reply(const uint8_t *data, uint16_t len)
{
    if (!data || !len || len > NS2_REPLY_MAX)
        return false;
    if (_ns2_reply_count == NS2_REPLY_QUEUE)
        return false;

    ns2_reply_t *r = &_ns2_replies[(_ns2_reply_head + _ns2_reply_count) % NS2_REPLY_QUEUE];
    memcpy(r->data, data, len);
    r->len = len;
    _ns2_reply_count++;

    if (!usbd_edpt_busy(0, _ns2_itf.bulk_in))
        _ns2_reply_start(0);
    return true;
}

//--------------------------------------------------------------------+
// Input reports
//--------------------------------------------------------------------+

bool tud_ns2_ready(void)
{
    return tud_ready() && _ns2_itf.ep_in && !usbd_edpt_busy(0, _ns2_itf.ep_in);
}

bool tud_ns2_report(uint8_t report_id, void const *report, uint16_t len)
{
    if (!tud_ns2_ready() || len > NS2_EP_SIZE - 1)
        return false;
    TU_VERIFY(usbd_edpt_claim(0, _ns2_itf.ep_in));

    memset(_ns2_itf.in_buf, 0, sizeof(_ns2_itf.in_buf));
    _ns2_itf.in_buf[0] = report_id;
    memcpy(&_ns2_itf.in_buf[1], report, len);
    return usbd_edpt_xfer(0, _ns2_itf.ep_in, _ns2_itf.in_buf, NS2_EP_SIZE);
}

//--------------------------------------------------------------------+
// Class driver
//--------------------------------------------------------------------+

static void _ns2_reset(uint8_t rhport)
{
    (void)rhport;
    tu_memclr(&_ns2_itf, sizeof(_ns2_itf));
    _ns2_reply_head = 0;
    _ns2_reply_count = 0;
}

static void _ns2_init(void)
{
    _ns2_reset(0);
}

static uint16_t _ns2_open(uint8_t rhport, tusb_desc_interface_t const *desc_itf, uint16_t max_len)
{
    uint8_t const *p_desc = (uint8_t const *)desc_itf;

    if (desc_itf->bInterfaceClass == TUSB_CLASS_HID)
    {
        uint16_t const len = (uint16_t)(sizeof(tusb_desc_interface_t) + sizeof(tusb_hid_descriptor_hid_t) +
                                        desc_itf->bNumEndpoints * sizeof(tusb_desc_endpoint_t));
        TU_ASSERT(max_len >= len, 0);

        p_desc = tu_desc_next(p_desc);
        TU_ASSERT(HID_DESC_TYPE_HID == tu_desc_type(p_desc), 0);
        p_desc = tu_desc_next(p_desc);
        TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, desc_itf->bNumEndpoints, TUSB_XFER_INTERRUPT,
                                      &_ns2_itf.ep_out, &_ns2_itf.ep_in), 0);
        _ns2_itf.itf_hid = desc_itf->bInterfaceNumber;

        if (_ns2_itf.ep_out)
            TU_ASSERT(usbd_edpt_xfer(rhport, _ns2_itf.ep_out, _ns2_itf.out_buf, NS2_EP_SIZE), 0);

        // A new configuration: the console starts its conversation over
        if (_ns2_cfg.reset)
            _ns2_cfg.reset();
        return len;
    }

    if (desc_itf->bInterfaceClass == TUSB_CLASS_VENDOR_SPECIFIC)
    {
        uint16_t const len = (uint16_t)(sizeof(tusb_desc_interface_t) +
                                        desc_itf->bNumEndpoints * sizeof(tusb_desc_endpoint_t));
        TU_ASSERT(max_len >= len, 0);

        p_desc = tu_desc_next(p_desc);
        TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, desc_itf->bNumEndpoints, TUSB_XFER_BULK,
                                      &_ns2_itf.bulk_out, &_ns2_itf.bulk_in), 0);
        TU_ASSERT(usbd_edpt_xfer(rhport, _ns2_itf.bulk_out, _ns2_itf.cmd_buf, NS2_EP_SIZE), 0);
        return len;
    }

    return 0;
}

static bool _ns2_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    TU_VERIFY(request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_INTERFACE);
    TU_VERIFY((uint8_t)request->wIndex == _ns2_itf.itf_hid);

    if (stage != CONTROL_STAGE_SETUP)
        return true;

    if (request->bmRequestType_bit.type == TUSB_REQ_TYPE_STANDARD)
    {
        // A Switch 1 or a computer reads the report descriptor; a Switch 2 never does
        if (request->bRequest == TUSB_REQ_GET_DESCRIPTOR &&
            tu_u16_high(request->wValue) == HID_DESC_TYPE_REPORT && _ns2_cfg.report_descriptor)
        {
            return tud_control_xfer(rhport, request, (void *)(uintptr_t)_ns2_cfg.report_descriptor,
                                    _ns2_cfg.report_descriptor_len);
        }
        return false;
    }

    if (request->bmRequestType_bit.type == TUSB_REQ_TYPE_CLASS)
    {
        switch (request->bRequest)
        {
        case HID_REQ_CONTROL_SET_IDLE:
        case HID_REQ_CONTROL_SET_PROTOCOL:
            return tud_control_status(rhport, request);
        default:
            return false;
        }
    }
    return false;
}

static bool _ns2_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes)
{
    (void)result;

    if (ep_addr == _ns2_itf.bulk_out)
    {
        if (_ns2_cfg.command_rx)
            _ns2_cfg.command_rx(_ns2_itf.cmd_buf, (uint16_t)xferred_bytes);
        TU_ASSERT(usbd_edpt_xfer(rhport, _ns2_itf.bulk_out, _ns2_itf.cmd_buf, NS2_EP_SIZE));
    }
    else if (ep_addr == _ns2_itf.bulk_in)
    {
        if (_ns2_itf.zlp_pending)
        {
            _ns2_itf.zlp_pending = false;
            return usbd_edpt_xfer(rhport, _ns2_itf.bulk_in, NULL, 0);
        }
        _ns2_reply_start(rhport);
    }
    else if (ep_addr == _ns2_itf.ep_out)
    {
        void (*out_cb)(const uint8_t *, uint16_t) = hhl_tusb_hook_hid_output_report();
        if (out_cb)
            out_cb(_ns2_itf.out_buf, (uint16_t)xferred_bytes);
        TU_ASSERT(usbd_edpt_xfer(rhport, _ns2_itf.ep_out, _ns2_itf.out_buf, NS2_EP_SIZE));
    }
    return true;
}

static const usbd_class_driver_t _ns2_driver = {
#if CFG_TUSB_DEBUG >= 2
    .name = "ns2",
#endif
    .init            = _ns2_init,
    .reset           = _ns2_reset,
    .open            = _ns2_open,
    .control_xfer_cb = _ns2_control_xfer_cb,
    .xfer_cb         = _ns2_xfer_cb,
    .sof             = NULL,
};

const usbd_class_driver_t *hhl_tusb_ns2_driver(void)
{
    return &_ns2_driver;
}

//--------------------------------------------------------------------+
// Driver ops
//--------------------------------------------------------------------+

static const uint8_t *_ns2_device_descriptor(void)
{
    return _ns2_cfg.device_descriptor;
}

static const uint8_t *_ns2_config_descriptor(uint8_t index)
{
    (void)index;
    return _ns2_cfg.config_descriptor;
}

static const uint8_t *_ns2_hid_report_descriptor(uint8_t instance)
{
    (void)instance;
    return _ns2_cfg.report_descriptor;
}

// The console's requests to the device before its first command
static bool _ns2_vendor_control_xfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    if (stage != CONTROL_STAGE_SETUP)
        return true;
    if (request->bmRequestType_bit.type != TUSB_REQ_TYPE_VENDOR || !_ns2_cfg.vendor_request)
        return false;

    int16_t n = _ns2_cfg.vendor_request(request->bRequest, request->wValue, _ns2_itf.ctrl_buf,
                                        sizeof(_ns2_itf.ctrl_buf));
    if (n < 0)
        return false;

    if (request->bmRequestType_bit.direction == TUSB_DIR_IN)
        return tud_control_xfer(rhport, request, _ns2_itf.ctrl_buf, tu_min16((uint16_t)n, request->wLength));
    return tud_control_status(rhport, request);
}

static const hhl_tusb_driver_ops_s _ns2_ops = {
    .device_descriptor        = _ns2_device_descriptor,
    .configuration_descriptor = _ns2_config_descriptor,
    .hid_report_descriptor    = _ns2_hid_report_descriptor,
    .bos_descriptor           = NULL,
    .ms_os_20_descriptor      = NULL,
    .descriptor_string        = hhl_tusb_strings_descriptor_cb,
    .vendor_control_xfer      = _ns2_vendor_control_xfer,
    .class_driver             = hhl_tusb_ns2_driver,
};

const hhl_tusb_driver_ops_s *hhl_tusb_ns2_ops(void)
{
    return &_ns2_ops;
}

#endif
