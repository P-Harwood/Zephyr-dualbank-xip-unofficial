#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/flash/ra_flash_api_extensions.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/reboot.h>

#include "bootutil/bootutil.h"
#include "bootutil/bootutil_log.h"
#include "bootutil/fault_injection_hardening.h"

/* Declare apart of mcuboot's logging group*/
BOOT_LOG_MODULE_DECLARE(mcuboot);

// Overwrite weak function
fih_ret boot_image_jump_hook(struct boot_rsp *response)
{
	int status;

	/**br_image_off is boot response offset of the chosen slot
	 * This if statement is comparing if the offset of the chosen
	 * slot matches the offset of slot1. 
	*/
	if (response->br_image_off != PARTITION_OFFSET(slot1_partition)) {
		FIH_RET(FIH_SUCCESS);
	}

	BOOT_LOG_INF("Image is in bank 1, swapping banks and resetting");

	/** Calls the code edited into upstream in soc_flash_renesas_ra_hp.c
	 * 
	 * 	flash ex op with the bankswap parameter calls flash_ra_bank_swap()
	 *  upstream altered to include flash_ra_bank_swap()
	*/
	status = flash_ex_op(PARTITION_DEVICE(slot1_partition), FLASH_RA_EX_OP_BANK_SWAP,
			 (uintptr_t)NULL, NULL);

	if (status == 0) {
		/** Success case, image swapped - reboot */
		sys_reboot(SYS_REBOOT_COLD);
	}


	/* Failure case */
	BOOT_LOG_ERR("Bank swap failed (%d)", status);
	FIH_RET(FIH_FAILURE);
}
