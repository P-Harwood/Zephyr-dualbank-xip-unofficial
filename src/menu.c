#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/storage/flash_map.h>
#include <stdio.h>
#include <string.h>


#include "menu.h"
#include "header.h"
#include "comms.h"
#include "xmodem.h"


// UI Banner
const char *gp_banner[] = {
    "  /\\/\\   ___ _ __  _   _",
    " /    \\ / _ \\ '_ \\| | | |",
    "/ /\\/\\ \\  __/ | | | |_| |",
    "\\/    \\/\\___|_| |_|\\__,_|",
};
void menu(void)
{
    printk("New software version v1.4\n"); // Optional print statment, good for seeing software version
    printk("Inside Menu\n");


    uint8_t tx_str[80]; // buffer for text to be output
    uint8_t rx_str[20]; // buffer to hold receiving text
    int     err;


    // Start at a new line
    sprintf((char *)tx_str, "\n\r");
    comms_send(tx_str, strlen((char *)tx_str));


    //Print the banner
    for (uint8_t b = 0; b < 4; b++) {
        comms_send((uint8_t *)gp_banner[b], strlen(gp_banner[b]));
        comms_send((uint8_t *)"\n\r", 2);
    }
    // Print user options
    sprintf((char *)tx_str, "\n\r1 - Display image slot info\n\r");
    comms_send(tx_str, strlen((char *)tx_str));


    sprintf((char *)tx_str, "2 - Download new update image (XModem)\n\r");
    comms_send(tx_str, strlen((char *)tx_str));


    sprintf((char *)tx_str, "3 - Reboot\n\r");
    comms_send(tx_str, strlen((char *)tx_str));


    comms_send((uint8_t *)">", 1);


    // Prepare a variable to hold the user input
    rx_str[0] = 0;
    uint32_t len = 1;
    // Call comms_read to wait for user input and store the new input in rx_Str
    err = comms_read(rx_str, &len, NO_TIMEOUT);
    if (err != COMMS_OK) {
        return;    
    }
    // Echo the user input back
    comms_send(rx_str, 1);
    comms_send((uint8_t *)"\n\r", 2);


    // Switch the user input
    switch (rx_str[0])
    {
        case '1':
        {
            uint8_t tx_str[80]; // buffer for text to be output

            sprintf((char *)tx_str, "Opening dual bank info\n\r");
            comms_send(tx_str, strlen((char *)tx_str));
            // Display slot info  
            dual_bank_info();
            break;
        }
        case '2':
        {
            sprintf((char *)tx_str, "Start Xmodem transfer...\r\n");
            comms_send(tx_str, strlen((char *)tx_str));


            // Call Xmodem code which handles downloading and inserting new image
            unsigned char xm_err = XmodemDownloadAndProgramFlash();


            // Handle outcomes from Xmodem
            if (XM_OK == xm_err)
            {
                // Nothing to mark: on reset MCUboot boots the newest valid image
                // and swaps banks first if that image is in the secondary slot
                sprintf((char *)tx_str, "Image download successful - reboot (3) to apply\r\n");
                comms_send(tx_str, strlen((char *)tx_str));
            }
            else
            {
                switch (xm_err) {
                    case XM_ADDRESS_ERROR:
                        sprintf((char *)tx_str, "ERROR: Flash address invalid\r\n");
                        break;
                    case XM_COMMS_ERROR:
                        sprintf((char *)tx_str, "ERROR: Comms error during Xmodem download\r\n");
                        break;
                    case XM_TIMEOUT:
                        sprintf((char *)tx_str, "ERROR: Timeout during Xmodem download\r\n");
                        break;
                    case XM_PROG_FAIL:
                        sprintf((char *)tx_str, "ERROR: Flash programming error\r\n");
                        break;
                    default:
                        sprintf((char *)tx_str, "ERROR: unknown (%d)\r\n", xm_err);
                        break;
                }
                comms_send(tx_str, strlen((char *)tx_str));
            }
            break;
        }
        case '3':
        {
            // Reboot the device
            sprintf((char *)tx_str, "Resetting the device...\r\n");
            comms_send(tx_str, strlen((char *)tx_str));
            sys_reboot(SYS_REBOOT_COLD);
            break;
        }
        default:
        {
            break;
        }

    }
}
