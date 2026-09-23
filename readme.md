This project is an unofficial example of dual bank XIP for the RA6M5 using Zephyr. 

This tutorial uses Mcumgr cli for flashing the image to the device

    - https://github.com/apache/mynewt-mcumgr-cli

Alterations were applied to:

    - Zephyr v4.4.0-16332-g55f132b58cd
    - mcuboot v2.4.0-150-gaa32eaaa (Only patch required on this version is the RA6M5 erase size patch)


Requires alterations to upstream Drivers (in commit format for readability):
    https://github.com/renesas/zephyr/commit/d006070918b364a0bc051111f89a93b31faa3b77

Example build command: 

    west build -b ek_ra6m5 -d build\v1 --sysbuild . -p always

To upload firmware use the Renesas Flash programmer. The following generated files must be flashed:

    \build\v1\db_xip\zephyr\zephyr.signed.confirmed.hex
    \build\v1\mcuboot\zephyr\zephyr.hex
    \build\v1\mcuboot\zephyr\zephyr_bank1_dual.hex

To flash a new firmware into second slot, update VERSION to a newer version number, then build a new image:

    west build -b ek_ra6m5 -d build\v2 --sysbuild . -p always

Then run mcumgr inside a terminal:

    mcumgr --conntype serial --connstring "COM5,baud=115200" image upload build\v2\db_xip\zephyr\zephyr.signed.bin

The example application has a uart console. Access this with teraterm.  Commands:

    dualboot        - list commands
    dualboot list   - provides details on the images loaded on the device
    dualboot swap   - swap the banks and restart the device
