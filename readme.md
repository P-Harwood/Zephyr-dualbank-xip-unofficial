This project is an unofficial example of dual bank XIP for the RA6M5 using Zephyr. 

This tutorial uses Mcumgr cli for flashing the image to the device

    - https://github.com/apache/mynewt-mcumgr-cli

Alterations were applied to:

    - Zephyr v4.4.0-16332-g55f132b58cd
    - mcuboot v2.4.0-150-gaa32eaaa (Only patch required on this version is the RA6M5 erase size patch)

Example build command: 
    west build -b ek_ra6m5 -d build\v1 --sysbuild . -p always


Erase Board Command set:
rfp-cli -d RA -t jlink -if swd -erase-chip 
rfp-cli -d RA -t jlink -if swd -pv OFS_Alter_hexs/linear.hex
rfp-cli -d RA -t jlink -if swd -pv OFS_Alter_hexs/BankSwpDefault.hex


rfp-cli -d RA -t jlink -if swd -pv build\v1\mcuboot\zephyr\mcuboot_bank0.hex build\v2\mcuboot\zephyr\mcuboot_bank1_jlink.hex build\v2\db_xip\zephyr\app_bank1_jlink.hex build\v1\db_xip\zephyr\zephyr.signed.hex 

=== === JLink flashing commands === ===

JLink.exe -device R7FA6M5BH -if SWD -speed 4000 -autoconnect 1

Write both bootloaders and application images
    loadfile build\v2\mcuboot\zephyr\mcuboot_bank1_jlink.hex
    loadfile build\v2\db_xip\zephyr\app_bank1_jlink.hex
    loadfile build\v1\db_xip\zephyr\zephyr.signed.hex
    loadfile build\v1\mcuboot\zephyr\mcuboot_bank0.hex
    q

Write both bootloaders and Application Image 1
    loadfile build\v1\db_xip\zephyr\app_bank0.hex
    loadfile build\v1\mcuboot\zephyr\mcuboot_bank1_jlink.hex
    loadfile build\v1\mcuboot\zephyr\mcuboot_bank0.hex
    q

Write Application Image 2 to a running application
    h
    loadfile OFS_Alter_hexs/linear.hex
    loadfile build\v3\db_xip\zephyr\app_bank1_jlink.hex
    loadfile OFS_Alter_hexs/dual.hex
    g