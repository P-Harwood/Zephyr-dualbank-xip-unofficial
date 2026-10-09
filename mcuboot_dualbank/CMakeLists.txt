zephyr_compile_definitions(BSP_CFG_OPTION_SETTING_DUALSEL=0xFFFFFFF8U)
zephyr_linker_sources(SECTIONS dualsel.ld)

set(mcuboot_elf ${PROJECT_BINARY_DIR}/${KERNEL_ELF_NAME})
set(mcuboot_hex ${PROJECT_BINARY_DIR}/${KERNEL_HEX_NAME})

set_property(GLOBAL APPEND PROPERTY extra_post_build_commands
  COMMAND ${CMAKE_COMMAND} -E copy ${mcuboot_hex} ${PROJECT_BINARY_DIR}/mcuboot_bank0.hex
  COMMAND ${CMAKE_OBJCOPY} -O ihex -R .option_setting* --change-addresses 0x200000
          ${mcuboot_elf} ${PROJECT_BINARY_DIR}/mcuboot_bank1_rfp.hex
  COMMAND ${CMAKE_OBJCOPY} -O ihex -R .option_setting* --change-addresses 0x100000
          ${mcuboot_elf} ${PROJECT_BINARY_DIR}/mcuboot_bank1_jlink.hex
)

if(CONFIG_BOOT_IMAGE_JUMP_HOOKS)
  zephyr_library()
  zephyr_library_sources(bank_swap_hook.c)
  zephyr_library_link_libraries(MCUBOOT_BOOTUTIL)
endif()