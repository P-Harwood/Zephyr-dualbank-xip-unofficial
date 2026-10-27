#include <zephyr/kernel.h>
#include <zephyr/dfu/mcuboot.h>
#include "header.h"
#include <zephyr/drivers/gpio.h>

int main(void)
{
	dual_bank_info();
	return 0;
}