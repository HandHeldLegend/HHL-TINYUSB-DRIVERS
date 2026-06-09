#ifndef HHL_TUSB_STRINGS_H
#define HHL_TUSB_STRINGS_H

/**
 * Shared USB string descriptor builder used by all HHL TinyUSB drivers.
 *
 * String indices follow the USB convention used by our device descriptors:
 *   0 = language ID (owned by the library, English 0x0409)
 *   1 = manufacturer
 *   2 = product
 *   3 = serial number
 *
 * Each driver's provider routes GET STRING requests through
 * hhl_tusb_strings_descriptor_cb().
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const char *manufacturer;
    const char *product;
    const char *serial_number;
} hhl_tusb_strings_config_s;

/** Install the active string config (copies pointers only; strings must outlive USB). */
void hhl_tusb_strings_configure(const hhl_tusb_strings_config_s *config);

/** TinyUSB GET STRING handler for standard indices and 0xEE (MS OS 1.0). */
uint16_t const *hhl_tusb_strings_descriptor_cb(uint8_t index, uint16_t langid);

#ifdef __cplusplus
}
#endif

#endif /* HHL_TUSB_STRINGS_H */
