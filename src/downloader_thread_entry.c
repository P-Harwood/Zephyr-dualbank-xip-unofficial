#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/dfu/mcuboot.h>

#include "comms.h"
#include "menu.h"
#include "header.h"

// MCUBoot magic number to be placed in the header of the image
#define IMAGE_MAGIC 0x96f3b83d  


void downloader_thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);


    // Check to see if UART is ready and status is ok
    if (comms_open() != 0) {
        // If the UART is not ready then exit with panic
        k_panic();
    }


    // Infinite loop of running menu during the thread
    while (1) {
        menu();
    }
}


// Define and instantly launch the downloader thread with a piority of 5
K_THREAD_DEFINE(downloader_tid, 4096, downloader_thread_entry,
                   NULL, NULL, NULL, 5, 0, 0);






// Struct which contains image version to be put into the header of the image
struct image_version {
    uint8_t  iv_major;
    uint8_t  iv_minor;
    uint16_t iv_revision;
    uint32_t iv_build_num;
};


// Image header which brings all the information together
struct image_header {
    uint32_t ih_magic;
    uint32_t ih_load_addr;
    uint16_t ih_hdr_size;            
    uint16_t ih_protect_tlv_size;    
    uint32_t ih_img_size;            
    uint32_t ih_flags;              
    struct image_version ih_ver;
    uint32_t _pad1;
};



/** @brief Check if dualbank mode is enabled
 *	@retval boolean is dualbank enabled
*/
static bool dual_bank_mode(void)
{
	bool enabled = (sys_read32(DUALSEL_ADDR) & BANKMD_MASK) == 0U;
	return enabled;
}

/** @brief Check if the banks have been swapped by checking the bank swap register
 *	@retval boolean is dualbank enabled
*/
static bool banks_swapped(void)
{
	bool swapped = (sys_read32(BANKSEL_ADDR) & BANKSWP_MASK) == 0U;
	return swapped;
}



static void send_str(const char *str)
{
	comms_send((uint8_t *)str, strlen(str));
}

/** @brief Prints image details such as version and size for a given slot
 *
 *	@retval void
 */
static void print_slot(const char *title, uint8_t area_id, off_t offset)
{
	char tx_str[80];
	struct mcuboot_img_header header;
	int rc;

	send_str("****************************\r\n");
	snprintf(tx_str, sizeof(tx_str), "* %s *\r\n", title);
	send_str(tx_str);
	send_str("****************************\r\n");

	rc = boot_read_bank_header(area_id, &header, sizeof(header));
	if (rc != 0) {
		snprintf(tx_str, sizeof(tx_str), "Image version:        <no valid header> (%d)\r\n", rc);
		send_str(tx_str);
		snprintf(tx_str, sizeof(tx_str), "Image start address:  0x%08lx\r\n", (unsigned long)offset);
		send_str(tx_str);
		return;
	}

	struct mcuboot_img_sem_ver image_version = header.h.v1.sem_ver;

	snprintf(tx_str, sizeof(tx_str), "Image version:        %u.%u (Rev: %u, Build: %u)\r\n",
		 image_version.major, image_version.minor, image_version.revision,
		 image_version.build_num);
	send_str(tx_str);

	snprintf(tx_str, sizeof(tx_str), "Image start address:  0x%08lx\r\n", (unsigned long)offset);
	send_str(tx_str);

	snprintf(tx_str, sizeof(tx_str), "Header size:          0x%04x (%u bytes)\r\n",
		 CONFIG_ROM_START_OFFSET, CONFIG_ROM_START_OFFSET);
	send_str(tx_str);

	snprintf(tx_str, sizeof(tx_str), "Image size:           0x%08x (%u bytes)\r\n",
		 header.h.v1.image_size, header.h.v1.image_size);
	send_str(tx_str);
}


void dual_bank_info(void)
{
	char tx_str[50];

	
	sprintf((char *)tx_str, "\n=== RA6M5 dual bank direct-XIP demo ===\n");
	comms_send(tx_str, strlen((char *)tx_str));

	sprintf((char *)tx_str, "Running image:        v%s\n", CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION);
	comms_send(tx_str, strlen((char *)tx_str));

	sprintf((char *)tx_str, "Code flash mode:      %s bank\n", dual_bank_mode() ? "dual" : "linear");
	comms_send(tx_str, strlen((char *)tx_str));

	sprintf((char *)tx_str, "Bank at address 0:    %s\n", banks_swapped() ? "bank 1 (swapped)" : "bank 0");
	comms_send(tx_str, strlen((char *)tx_str));

	sprintf((char *)tx_str, "DUALSEL:              0x%08x\n", sys_read32(DUALSEL_ADDR));
	comms_send(tx_str, strlen((char *)tx_str));

	sprintf((char *)tx_str, "BANKSEL / _SEL:       0x%08x / 0x%08x\n", sys_read32(BANKSEL_ADDR), sys_read32(BANKSEL_SEL_ADDR));
	comms_send(tx_str, strlen((char *)tx_str));

	sprintf((char *)tx_str, "Vector table (VTOR):  0x%08x\n", SCB->VTOR);
	comms_send(tx_str, strlen((char *)tx_str));


	print_slot("Primary Image Slot", PARTITION_ID(slot0_partition), PARTITION_OFFSET(slot0_partition));
	
	print_slot("Secondary Image Slot", PARTITION_ID(slot1_partition), PARTITION_OFFSET(slot1_partition));

	
	sprintf((char *)tx_str, "=======================================\n\n");
	comms_send(tx_str, strlen((char *)tx_str));
}