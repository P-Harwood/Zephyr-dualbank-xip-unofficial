#include <zephyr/kernel.h>
#include <zephyr/dfu/mcuboot.h>
#include "header.h"
int main(void)
{
	dual_bank_info();
	return 0;
}
