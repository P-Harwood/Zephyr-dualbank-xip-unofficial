cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ra6m5_dualbank_xip)

target_sources(app PRIVATE
    src/main.c
    src/comms.c
    src/menu.c
    src/xmodem.c
    src/downloader_thread_entry.c
    src/blinky_thread_entry.c)

if(CONFIG_BOOTLOADER_MCUBOOT)
  set(signed_hex ${ZEPHYR_BINARY_DIR}/zephyr.signed.hex)
  set(signed_bin ${ZEPHYR_BINARY_DIR}/zephyr.signed.bin)

  add_custom_target(bank_images ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${signed_hex} ${ZEPHYR_BINARY_DIR}/app_bank0.hex
    COMMAND ${CMAKE_OBJCOPY} -I ihex -O ihex --change-addresses 0x200000
            ${signed_hex} ${ZEPHYR_BINARY_DIR}/app_bank1_rfp.hex
    COMMAND ${CMAKE_OBJCOPY} -I ihex -O ihex --change-addresses 0x100000
            ${signed_hex} ${ZEPHYR_BINARY_DIR}/app_bank1_jlink.hex
    COMMAND ${CMAKE_COMMAND} -E copy ${signed_bin} ${ZEPHYR_BINARY_DIR}/app_xmodem.bin
  )
  add_dependencies(bank_images ${logical_target_for_zephyr_elf})
endif()