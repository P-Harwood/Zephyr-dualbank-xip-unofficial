This project is an unofficial example of dual bank XIP for the RA6M5 using Zephyr. 

Install Steps (Start in desired install directory):
    
    python -m venv .venv
    cd .venv\Scripts
    activate.bat
    cd ../../
    pip install west
    west init -m https://github.com/P-Harwood/Zephyr-dualbank-xip-unofficial --mr main .
    west update
    west packages pip --install
    west zephyr-export


Example build commands to build a batch of 5 test scripts: 
(DCONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION has higher priority than VERSION)

    west build -b ek_ra6m5 -d build\v1 --sysbuild . -p always -- -DCONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION=\"1.1.0\" 
    west build -b ek_ra6m5 -d build\v2 --sysbuild . -p always -- -DCONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION=\"1.2.0\" 
    west build -b ek_ra6m5 -d build\v3 --sysbuild . -p always -- -DCONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION=\"1.3.0\"
    west build -b ek_ra6m5 -d build\v4 --sysbuild . -p always -- -DCONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION=\"1.4.0\"
    west build -b ek_ra6m5 -d build\v5 --sysbuild . -p always -- -DCONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION=\"1.5.0\"


Erase Board Command set:
    rfp-cli.exe -d RA -t jlink -if swd -erase-chip 
    rfp-cli.exe -d RA -t jlink -if swd -pv OFS_Alter_hexs/linear.hex
    rfp-cli.exe -d RA -t jlink -if swd -pv OFS_Alter_hexs/BankSwpDefault.hex


=== === rfp cli commands === ===
To flash a file with rfp to the board, execute this command with the following file(s)
    rfp-cli.exe -d RA -t jlink -if swd -pv 

e.g. for Both applications and both bootloaders:
    rfp-cli.exe -d RA -t jlink -if swd -pv build\v1\mcuboot\zephyr\mcuboot_bank0.hex build\v2\mcuboot\zephyr\mcuboot_bank1_jlink.hex build\v2\db_xip\zephyr\app_bank1_jlink.hex build\v1\db_xip\zephyr\zephyr.signed.hex 

For just image 1 and boot loader 1
    rfp-cli.exe -d RA -t jlink -if swd -pv build\v1\mcuboot\zephyr\mcuboot_bank0.hex build\v1\db_xip\zephyr\zephyr.signed.hex 

For both bootloaders and image 1
    rfp-cli.exe -d RA -t jlink -if swd -pv build\v1\mcuboot\zephyr\mcuboot_bank0.hex build\v2\mcuboot\zephyr\mcuboot_bank1_jlink.hex build\v1\db_xip\zephyr\zephyr.signed.hex 

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