#ifndef HHL_TUSB_CONFIG_H
#define HHL_TUSB_CONFIG_H

/**
 * TinyUSB configuration requirements for HHL-TINYUSB-DRIVERS.
 *
 * Include this from your project's tusb_config.h so the custom class drivers
 * have the CFG_TUD_* counts and endpoint buffer sizes they require. All values
 * are guarded with #ifndef so a project may override them before inclusion.
 */

#ifndef HHL_TUSB_DRIVER_XINPUT_ENABLE
#define HHL_TUSB_DRIVER_XINPUT_ENABLE 1
#endif

#ifndef HHL_TUSB_DRIVER_SLIPPI_ENABLE
#define HHL_TUSB_DRIVER_SLIPPI_ENABLE 1
#endif

//--------------------------------------------------------------------+
// XInput (Xbox 360) class driver
//--------------------------------------------------------------------+
#if (HHL_TUSB_DRIVER_XINPUT_ENABLE)
  #ifndef CFG_TUD_XINPUT
  #define CFG_TUD_XINPUT 1
  #endif

  #ifndef CFG_TUD_XINPUT_EP_BUFSIZE
  #define CFG_TUD_XINPUT_EP_BUFSIZE 64
  #endif
#endif

//--------------------------------------------------------------------+
// Slippi (GameCube adapter) HID class driver
//--------------------------------------------------------------------+
#if (HHL_TUSB_DRIVER_SLIPPI_ENABLE)
  // Number of GameCube adapter interfaces
  #ifndef CFG_TUD_GC
  #define CFG_TUD_GC 1
  #endif

  // GameCube adapter IN report is 37 bytes, OUT (rumble) is up to 6 bytes
  #ifndef CFG_TUD_GC_TX_BUFSIZE
  #define CFG_TUD_GC_TX_BUFSIZE 37
  #endif

  #ifndef CFG_TUD_GC_RX_BUFSIZE
  #define CFG_TUD_GC_RX_BUFSIZE 6
  #endif
#endif

#endif /* HHL_TUSB_CONFIG_H */
