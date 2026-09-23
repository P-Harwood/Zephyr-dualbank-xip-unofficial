#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/flash/ra_flash_api_extensions.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/shell/shell.h>
#include <cmsis_core.h>
#include <zephyr/dfu/mcuboot.h>


#define DUALSEL_ADDR     DT_REG_ADDR(DT_NODELABEL(option_setting_dualsel))
#define BANKSEL_ADDR     0x0100A190U
#define BANKSEL_SEL_ADDR DT_REG_ADDR(DT_NODELABEL(option_setting_banksel_sel))

#define BANKMD_MASK  0x7U	/* Bank mode mask for DualSel register (0xF - 0b111) */
#define BANKSWP_MASK 0x7U	/* Bank Swap mask for BankSel register (0xF - 0b111) */

#define enabled_led_index 3 /* The index of enabled - modify this value not max_led_index */
#define max_led_index 3 /* Maximum allowed enabled leds*/

BUILD_ASSERT(max_led_index >= enabled_led_index, "max_led_index must be larger than or equal to enabled_led_index");
BUILD_ASSERT(enabled_led_index >= 0, "enabled_led_index must be larger or equal to 0");

/* LEDs to blink for example application */
static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_NODELABEL(led1), gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(led2), gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(led3), gpios),
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

/** @brief Prints image details such as version and size for a given slot
 * 
 * 	@retval void
 */
static void print_slot(const char *title, uint8_t area_id, off_t offset)
{
	struct mcuboot_img_header header;
	int rc;

	printk("****************************\n");
	printk("* %s *\n", title);
	printk("****************************\n");

	rc = boot_read_bank_header(area_id, &header, sizeof(header));
	if (rc != 0) {
		printk("Image version:        <no valid header> (%d)\n", rc);
		printk("Image start address:  0x%08lx\n", (unsigned long)offset);
		return;
	}

	struct mcuboot_img_header_v1 header_info = header.h.v1;
	struct mcuboot_img_sem_ver image_version = header_info.sem_ver;

	printk("Image version:        %u.%u (Rev: %u, Build: %u)\n", image_version.major,
	       image_version.minor, image_version.revision, image_version.build_num);

	printk("Image start address:  0x%08lx\n", (unsigned long)offset);

	printk("Header size:          0x%04x (%u bytes)\n", CONFIG_ROM_START_OFFSET,
	       CONFIG_ROM_START_OFFSET);

	printk("Image size:           0x%08x (%u bytes)\n", header_info.image_size,
	       header_info.image_size);
}

/** @brief 	prints the status of the dual bank application, what is running
 * 			what the status of each slot is
 * 
 * 	@retval void 
 */
static void dual_bank_info(void)
{


	printk("\n=== RA6M5 dual bank direct-XIP demo ===\n");

	printk("Running image:        v%s\n", CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION);
	printk("Code flash mode:      %s bank\n", dual_bank_mode() ? "dual" : "linear");
	printk("Bank at address 0:    %s\n", banks_swapped() ? "bank 1 (swapped)" : "bank 0");

	printk("DUALSEL:              0x%08x\n", sys_read32(DUALSEL_ADDR));
	printk("BANKSEL / _SEL:       0x%08x / 0x%08x\n", sys_read32(BANKSEL_ADDR), sys_read32(BANKSEL_SEL_ADDR));

	printk("Vector table (VTOR):  0x%08x\n", SCB->VTOR);

	print_slot("Primary Image Slot", PARTITION_ID(slot0_partition), PARTITION_OFFSET(slot0_partition));
	print_slot("Secondary Image Slot", PARTITION_ID(slot1_partition), PARTITION_OFFSET(slot1_partition));

	printk("=======================================\n\n");
}


/** @brief Wrapper which calls dual bank info, called via UART command 
 *  @retval int - always 0. 
 */
static int dual_bank_info_wrapper(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(sh);
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	dual_bank_info();

	return 0;
}

/** @brief swaps the banks and then restarts the board */
static int bank_swap_command(const struct shell *sh, size_t argc, char **argv)
{
	const struct device *flash = PARTITION_DEVICE(slot1_partition);
	int rc;

	ARG_UNUSED(argc);
	ARG_UNUSED(argv);


	/* Ensure that flash peripheral is ready*/
	if (!device_is_ready(flash)) {
		shell_error(sh, "code flash device not ready");
		return -ENODEV;
	}

	/* Print to the UART shell*/
	shell_print(sh, "Swapping banks and resetting...");

	/* Execute the flash swap command */
	rc = flash_ex_op(flash, FLASH_RA_EX_OP_BANK_SWAP, (uintptr_t)NULL, NULL);
	if (rc != 0) {
		/* If the bank swap command failed then report the error*/
		shell_error(sh, "bank swap failed: %d%s", rc,
			    (rc == -ENOTSUP) ? " (code flash is in linear mode?)" : "");
		return rc;
	}

	k_sleep(K_MSEC(100));
	/* Reboot the device*/
	sys_reboot(SYS_REBOOT_COLD);

	return 0;
}

/** Make a UART commandline, two enabled commands which are routed to the given functions */
SHELL_STATIC_SUBCMD_SET_CREATE(dualbank_cmds,
	SHELL_CMD(info, NULL, "Show bank state and both image slots", dual_bank_info_wrapper),
	SHELL_CMD(swap, NULL, "Swap code flash banks and reset (bypasses MCUboot)", bank_swap_command),
	SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(dualbank, &dualbank_cmds, "Dual bank demo commands", NULL);




int main(void)
{

	dual_bank_info(); /* Print the current status of the application*/

	/* For each enabled LED, check it is working, if one is not working then break the applicaiton*/
	for (size_t i = 0; i < enabled_led_index; i++) {
		if (!gpio_is_ready_dt(&leds[i])) {
			printk("LED %u not ready\n", (unsigned int)i);
			return 0;
		}
		gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
	}

	/* Inform that the blinking has started*/
	printk("Blinking %u of %u LEDs\n", (unsigned int)enabled_led_index, (unsigned int)ARRAY_SIZE(leds));

	/* Forever while loop of toggling pins and waiting 500ms per loop*/
	while (1) {
		for (size_t i = 0; i < enabled_led_index; i++) {
			gpio_pin_toggle_dt(&leds[i]);
		}
		k_sleep(K_MSEC(500));
	}

	return 0;
}
