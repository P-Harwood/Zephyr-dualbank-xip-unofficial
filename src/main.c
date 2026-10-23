#include <zephyr/kernel.h>
#include <zephyr/dfu/mcuboot.h>
#include "header.h"
#include <zephyr/drivers/gpio.h>

#ifdef CONFIG_FLASH_RENESAS_RA_HP_IRQ_ENABLED
#warning "RA flash: Interupts will occur during flash write"
#else
#warning "RA flash: Interupts will NOT occur during flash write"
#endif


int main(void)
{
	dual_bank_info();
	return 0;
}