#ifndef FLAGS_H_
#define FLAGS_H_

#include <stdbool.h>
#include <stdint.h>
#include <zephyr/device.h>
#include <soc.h>
#include <zephyr/drivers/counter.h>

#define ALARM_CHANNEL_ID 0
#define ALARM_FLAGS 0


#define FLASH_WRITE_ALARM_US 100
#define AGT_COUNT_MASK       0xFFFFU


extern volatile bool started_flash_write;
extern volatile bool ended_flash_write;
extern volatile int interrupts_during_write;
extern volatile uint32_t interrupt_latency_us;
extern volatile uint32_t most_late;


void start_flash_write_alarm(void);
void stop_flash_write_alarm(void);
void flash_write_alarm_callback(const struct device *dev, uint8_t chan_id, uint32_t ticks,
				void *user_data);


#endif