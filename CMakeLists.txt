# HHL-TINYUSB-DRIVERS
#
# Portable collection of custom TinyUSB class drivers (XInput, Slippi/GameCube
# adapter) and the USB descriptors they own. Designed to be consumed as an
# INTERFACE library so the sources compile in the context of the host target
# that already links TinyUSB.
#
# Options:
#   HHL_TUSB_DRIVER_XINPUT_ENABLE (default ON)
#   HHL_TUSB_DRIVER_SLIPPI_ENABLE (default ON)

option(HHL_TUSB_DRIVER_XINPUT_ENABLE "Build the XInput TinyUSB driver" ON)
option(HHL_TUSB_DRIVER_SLIPPI_ENABLE "Build the Slippi/GameCube TinyUSB driver" ON)

add_library(hhl_tinyusb_drivers INTERFACE)

target_sources(hhl_tinyusb_drivers INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/src/hhl_tusb_drivers.c
  ${CMAKE_CURRENT_LIST_DIR}/src/hhl_tusb_callbacks.c
  ${CMAKE_CURRENT_LIST_DIR}/src/hhl_tusb_transport.c
  ${CMAKE_CURRENT_LIST_DIR}/src/hhl_tusb_strings.c
  ${CMAKE_CURRENT_LIST_DIR}/src/hhl_tusb_hid.c
  ${CMAKE_CURRENT_LIST_DIR}/src/hhl_tusb_webusb.c
  ${CMAKE_CURRENT_LIST_DIR}/src/hhl_tusb_xinput.c
  ${CMAKE_CURRENT_LIST_DIR}/src/hhl_tusb_slippi.c
)

target_include_directories(hhl_tinyusb_drivers INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_definitions(hhl_tinyusb_drivers INTERFACE
  HHL_TUSB_DRIVER_XINPUT_ENABLE=$<BOOL:${HHL_TUSB_DRIVER_XINPUT_ENABLE}>
  HHL_TUSB_DRIVER_SLIPPI_ENABLE=$<BOOL:${HHL_TUSB_DRIVER_SLIPPI_ENABLE}>
)

# TinyUSB headers (tusb.h / device/usbd_pvt.h) are required to compile the
# driver sources. The consuming target normally provides these already; link
# them here too so the interface is self describing.
if(TARGET tinyusb_device)
  target_link_libraries(hhl_tinyusb_drivers INTERFACE tinyusb_device)
endif()
if(TARGET tinyusb_board)
  target_link_libraries(hhl_tinyusb_drivers INTERFACE tinyusb_board)
endif()
