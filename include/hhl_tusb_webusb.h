#ifndef HHL_TUSB_WEBUSB_H
#define HHL_TUSB_WEBUSB_H

/**
 * WebUSB / Microsoft OS 2.0 support owned by the driver library.
 *
 * The library owns the WebUSB BOS descriptor, the Microsoft OS 2.0 descriptor
 * set, and the vendor (WebUSB) interface descriptor block. A plain HID
 * configuration descriptor coming from another library (e.g. NS-LIB-HID or
 * SINPUT-LIB-HID) can be combined at runtime with the vendor interface so the
 * same HID descriptors gain WebUSB support without each HID library having to
 * hard code a vendor interface.
 *
 * Vendor control-transfer handling for WebUSB (URL, MS OS 1.0/2.0) is also
 * owned here and dispatched through the HID driver's provider ops.
 */

#include <stdint.h>
#include <stdbool.h>

#include "hhl_tusb.h"
#include "tusb.h"

#ifdef __cplusplus
extern "C" {
#endif

// Vendor request codes advertised by the WebUSB BOS. The host firmware's
// vendor control-transfer handler must use the same values.
#define HHL_TUSB_VENDOR_REQUEST_WEBUSB    1
#define HHL_TUSB_VENDOR_REQUEST_MICROSOFT 2

// Length in bytes of the vendor (WebUSB) interface descriptor block appended to
// a HID configuration: interface (9) + bulk IN endpoint (7) + bulk OUT (7).
#define HHL_TUSB_WEBUSB_ITF_DESC_LEN 23

/** WebUSB BOS descriptor (WebUSB platform + Microsoft OS 2.0 capability). */
const uint8_t *hhl_tusb_webusb_bos_descriptor(void);
uint16_t hhl_tusb_webusb_bos_descriptor_len(void);

/** Microsoft OS 2.0 descriptor set bound to the vendor (WebUSB) interface. */
const uint8_t *hhl_tusb_webusb_ms_os_20_descriptor(void);
uint16_t hhl_tusb_webusb_ms_os_20_descriptor_len(void);

/**
 * Build the final configuration descriptor from a HID-only base, optionally
 * appending the vendor (WebUSB) interface and/or overriding bMaxPower. The
 * result is written into a library-owned static buffer that remains valid
 * until the next call. When the vendor interface is appended the descriptor's
 * bNumInterfaces / wTotalLength are fixed up automatically and the appended
 * interface number follows the existing HID interface(s).
 *
 * @param hid_config              HID-only configuration descriptor bytes.
 * @param hid_config_len          Length of the HID-only configuration descriptor.
 * @param append_vendor_interface Append the WebUSB vendor interface when true.
 * @param max_power_ma            Override bMaxPower (mA); 0 keeps the base value.
 * @param out_len                 Receives the length of the resulting descriptor.
 * @return Pointer to the resulting descriptor, or NULL on error.
 */
const uint8_t *hhl_tusb_webusb_build_config(const uint8_t *hid_config, uint16_t hid_config_len,
                                            bool append_vendor_interface, uint16_t max_power_ma,
                                            uint16_t *out_len);

/**
 * Produce a copy of a device descriptor with bcdUSB raised to 0x0210 when
 * needed so the host requests the BOS descriptor (a prerequisite for WebUSB).
 * The copy lives in a library-owned static buffer valid until the next call.
 * If bcdUSB is already >= 0x0201 the bytes are copied unchanged.
 *
 * @param device_desc      Device descriptor bytes (tusb_desc_device_t layout).
 * @param device_desc_len  Length of the device descriptor.
 * @return Pointer to the (possibly patched) device descriptor copy, or NULL.
 */
const uint8_t *hhl_tusb_webusb_patch_device_descriptor(const uint8_t *device_desc, uint16_t device_desc_len);

/** Runtime WebUSB options (URL, popup gate). Call before enumeration. */
void hhl_tusb_webusb_configure(const hhl_tusb_webusb_config_s *config);

/**
 * Vendor control-transfer handler for WebUSB / MS OS requests on the vendor
 * interface. Used by the HID driver provider; returns false when inactive.
 */
bool hhl_tusb_webusb_vendor_control_xfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);

/** TinyUSB vendor class instance used by the WebUSB bulk interface. */
#define HHL_TUSB_WEBUSB_VENDOR_INSTANCE 0

/** Vendor bulk TX endpoint has 64-byte packets in the HOJA configuration. */
#define HHL_TUSB_WEBUSB_REPORT_SIZE 64

#ifdef __cplusplus
}
#endif

#endif /* HHL_TUSB_WEBUSB_H */
