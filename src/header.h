#ifndef HEADER_H_
#define HEADER_H_


void dual_bank_info(void);


#define DUALSEL_ADDR     DT_REG_ADDR(DT_NODELABEL(option_setting_dualsel))
#define BANKSEL_ADDR     0x0100A190U
#define BANKSEL_SEL_ADDR DT_REG_ADDR(DT_NODELABEL(option_setting_banksel_sel))

#define BANKMD_MASK  0x7U	/* Bank mode mask for DualSel register (0xF - 0b111) */
#define BANKSWP_MASK 0x7U	/* Bank Swap mask for BankSel register (0xF - 0b111) */

/* Bank 1 offset from the start of code flash, 0x200000 on every dual bank part */
#define FLASH_HP_BANK1_OFFSET                                                                      \
	(BSP_FEATURE_FLASH_HP_CF_DUAL_BANK_START - BSP_FEATURE_FLASH_CODE_FLASH_START)
    
#define enabled_led_index 2 /* The index of enabled - modify this value not max_led_index */
#define max_led_index 3 /* Maximum allowed enabled leds*/

#endif


