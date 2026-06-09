#include "hhl_tusb_xinput.h"
#include "hhl_tusb_strings.h"

#if (HHL_TUSB_DRIVER_XINPUT_ENABLE)

#include <string.h>

#ifndef CFG_TUD_XINPUT_EP_BUFSIZE
#define CFG_TUD_XINPUT_EP_BUFSIZE 64
#endif

//--------------------------------------------------------------------+
// Descriptors
//--------------------------------------------------------------------+

static const tusb_desc_device_t _xinput_device_descriptor = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0xFF,
    .bDeviceSubClass    = 0xFF,
    .bDeviceProtocol    = 0xFF,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = HHL_TUSB_XINPUT_VID,
    .idProduct          = HHL_TUSB_XINPUT_PID,
    .bcdDevice          = 0x0572,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01,
};

#define XINPUT_CONFIG_DESCRIPTOR_LEN 48
static const uint8_t _xinput_configuration_descriptor[XINPUT_CONFIG_DESCRIPTOR_LEN] = {
    0x09,       // bLength
    0x02,       // bDescriptorType (Configuration)
    0x30, 0x00, // wTotalLength 48
    0x01,       // bNumInterfaces 1
    0x01,       // bConfigurationValue
    0x00,       // iConfiguration (String Index)
    0x80,       // bmAttributes
    0xFA,       // bMaxPower 500mA

    0x09, // bLength
    0x04, // bDescriptorType (Interface)
    0x00, // bInterfaceNumber 0
    0x00, // bAlternateSetting
    0x02, // bNumEndpoints 2
    0xFF, // bInterfaceClass
    0x5D, // bInterfaceSubClass
    0x01, // bInterfaceProtocol
    0x00, // iInterface (String Index)

    0x10,       // bLength
    0x21,       // bDescriptorType (HID)
    0x10, 0x01, // bcdHID 1.10
    0x01,       // bCountryCode
    0x24,       // bNumDescriptors
    0x81,       // bDescriptorType[0] (Unknown 0x81)
    0x14, 0x03, // wDescriptorLength[0] 788
    0x00,       // bDescriptorType[1] (Unknown 0x00)
    0x03, 0x13, // wDescriptorLength[1] 4867
    0x02,       // bDescriptorType[2] (Unknown 0x02)
    0x00, 0x03, // wDescriptorLength[2] 768
    0x00,       // bDescriptorType[3] (Unknown 0x00)

    0x07,       // bLength
    0x05,       // bDescriptorType (Endpoint)
    0x81,       // bEndpointAddress (IN/D2H)
    0x03,       // bmAttributes (Interrupt)
    0x20, 0x00, // wMaxPacketSize 32
    0x01,       // bInterval

    0x07,       // bLength
    0x05,       // bDescriptorType (Endpoint)
    0x02,       // bEndpointAddress (OUT/H2D)
    0x03,       // bmAttributes (Interrupt)
    0x20, 0x00, // wMaxPacketSize 32
    0x01,       // bInterval
};

const uint8_t *hhl_tusb_xinput_device_descriptor(void)
{
    return (const uint8_t *)&_xinput_device_descriptor;
}

uint16_t hhl_tusb_xinput_device_descriptor_len(void)
{
    return (uint16_t)sizeof(_xinput_device_descriptor);
}

const uint8_t *hhl_tusb_xinput_configuration_descriptor(void)
{
    return _xinput_configuration_descriptor;
}

uint16_t hhl_tusb_xinput_configuration_descriptor_len(void)
{
    return (uint16_t)sizeof(_xinput_configuration_descriptor);
}

//--------------------------------------------------------------------+
// Class driver state
//--------------------------------------------------------------------+

typedef struct
{
    uint8_t itf_num;
    uint8_t ep_in;
    uint8_t ep_out;

    CFG_TUSB_MEM_ALIGN uint8_t epin_buf[CFG_TUD_XINPUT_EP_BUFSIZE];
    CFG_TUSB_MEM_ALIGN uint8_t epout_buf[CFG_TUD_XINPUT_EP_BUFSIZE];
} xinputd_interface_t;

CFG_TUSB_MEM_SECTION static xinputd_interface_t _xinputd_itf;

//--------------------------------------------------------------------+
// USBD-CLASS API
//--------------------------------------------------------------------+

static void xinputd_reset(uint8_t rhport)
{
    (void)rhport;
    tu_memclr(&_xinputd_itf, sizeof(_xinputd_itf));
}

static void xinputd_init(void)
{
    xinputd_reset(0);
}

static uint16_t xinputd_open(uint8_t rhport, tusb_desc_interface_t const *desc_itf, uint16_t max_len)
{
    // Verify our descriptor is the correct class
    TU_VERIFY(0x5D == desc_itf->bInterfaceSubClass, 0);

    // len = interface + hid + n*endpoints
    uint16_t const drv_len = (uint16_t)(sizeof(tusb_desc_interface_t) +
                                        desc_itf->bNumEndpoints * sizeof(tusb_desc_endpoint_t)) +
                             16;

    TU_ASSERT(max_len >= drv_len, 0);

    uint8_t const *p_desc = tu_desc_next(desc_itf);
    uint8_t total_endpoints = 0;
    while ((total_endpoints < desc_itf->bNumEndpoints) && (drv_len <= max_len))
    {
        tusb_desc_endpoint_t const *desc_ep = (tusb_desc_endpoint_t const *)p_desc;
        if (TUSB_DESC_ENDPOINT == tu_desc_type(desc_ep))
        {
            TU_ASSERT(usbd_edpt_open(rhport, desc_ep));

            if (tu_edpt_dir(desc_ep->bEndpointAddress) == TUSB_DIR_IN)
            {
                _xinputd_itf.ep_in = desc_ep->bEndpointAddress;
            }
            else
            {
                _xinputd_itf.ep_out = desc_ep->bEndpointAddress;
            }
            total_endpoints += 1;
        }
        p_desc = tu_desc_next(p_desc);
    }

    _xinputd_itf.itf_num = desc_itf->bInterfaceNumber;

    return drv_len;
}

static bool xinputd_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    (void)rhport;
    (void)stage;
    (void)request;
    return true;
}

static bool xinputd_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes)
{
    (void)result;

    uint8_t instance = 0;

    // Sent report successfully
    if (ep_addr == _xinputd_itf.ep_in)
    {
        if (tud_hid_report_complete_cb)
        {
            tud_hid_report_complete_cb(instance, _xinputd_itf.epin_buf, (uint16_t)xferred_bytes);
        }
    }
    // Received report
    else if (ep_addr == _xinputd_itf.ep_out)
    {
        tud_hid_set_report_cb(instance, 0, HID_REPORT_TYPE_OUTPUT, _xinputd_itf.epout_buf, (uint16_t)xferred_bytes);
        TU_ASSERT(usbd_edpt_xfer(rhport, _xinputd_itf.ep_out, _xinputd_itf.epout_buf, sizeof(_xinputd_itf.epout_buf)));
    }

    return true;
}

static const usbd_class_driver_t _tud_xinput_driver =
    {
#if CFG_TUSB_DEBUG >= 2
        .name = "XINPUT",
#endif
        .init            = xinputd_init,
        .reset           = xinputd_reset,
        .open            = xinputd_open,
        .control_xfer_cb = xinputd_control_xfer_cb,
        .xfer_cb         = xinputd_xfer_cb,
        .sof             = NULL,
};

const usbd_class_driver_t *hhl_tusb_xinput_driver(void)
{
    return &_tud_xinput_driver;
}

//--------------------------------------------------------------------+
// Driver ops
//--------------------------------------------------------------------+

static const uint8_t *_xinput_ops_config_descriptor(uint8_t index)
{
    (void)index;
    return hhl_tusb_xinput_configuration_descriptor();
}

static const hhl_tusb_driver_ops_s _xinput_ops = {
    .device_descriptor        = hhl_tusb_xinput_device_descriptor,
    .configuration_descriptor = _xinput_ops_config_descriptor,
    .hid_report_descriptor    = NULL,
    .bos_descriptor           = NULL,
    .ms_os_20_descriptor      = NULL,
    .descriptor_string        = hhl_tusb_strings_descriptor_cb,
    .vendor_control_xfer      = NULL,
    .class_driver             = hhl_tusb_xinput_driver,
};

const hhl_tusb_driver_ops_s *hhl_tusb_xinput_ops(void)
{
    return &_xinput_ops;
}

const hhl_tusb_driver_ops_s *hhl_tusb_xinput_provider(void)
{
    return hhl_tusb_xinput_ops();
}

//--------------------------------------------------------------------+
// Application API
//--------------------------------------------------------------------+

void tud_xinput_getout(void)
{
    if (tud_ready() && (!usbd_edpt_busy(0, _xinputd_itf.ep_out)))
    {
        usbd_edpt_claim(0, _xinputd_itf.ep_out);
        usbd_edpt_xfer(0, _xinputd_itf.ep_out, _xinputd_itf.epout_buf, sizeof(_xinputd_itf.epout_buf));
        usbd_edpt_release(0, _xinputd_itf.ep_out);
    }
}

bool tud_n_xinput_report(uint8_t report_id, void const *report, uint16_t len)
{
    uint8_t const rhport = 0;

    // Remote wakeup
    if (tud_suspended())
    {
        tud_remote_wakeup();
    }

    TU_VERIFY(usbd_edpt_claim(rhport, _xinputd_itf.ep_in));

    _xinputd_itf.epin_buf[0] = report_id;
    memcpy(&_xinputd_itf.epin_buf[1], report, len);

    bool out = usbd_edpt_xfer(rhport, _xinputd_itf.ep_in, _xinputd_itf.epin_buf, len + 1);

    usbd_edpt_release(0, _xinputd_itf.ep_in);
    tud_xinput_getout();

    return out;
}

bool tud_xinput_report(uint8_t report_id, void const *report, uint16_t len)
{
    return tud_n_xinput_report(report_id, report, len);
}

bool tud_xinput_ready(void)
{
    uint8_t const rhport = 0;
    uint8_t const ep_in = _xinputd_itf.ep_in;
    return tud_ready() && (ep_in != 0) && !usbd_edpt_busy(rhport, ep_in);
}

#endif /* HHL_TUSB_DRIVER_XINPUT_ENABLE */
