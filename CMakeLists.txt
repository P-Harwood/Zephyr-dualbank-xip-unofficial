cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ra6m5_dualbank_xip)

target_sources(app PRIVATE src/main.c)
  zephyr_library_sources_ifdef(CONFIG_FLASH_RENESAS_RA_HP_WRITE_PROTECT soc_flash_renesas_ra_hp_ex_op.c)