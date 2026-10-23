#include "interrupt_maker.h"

volatile bool started_flash_write = false;
volatile bool ended_flash_write = false;
volatile int interrupts_during_write = 0;
volatile uint32_t interrupt_latency_us = 0;
volatile uint32_t most_late = 0;

static const struct device *const flash_write_counter = DEVICE_DT_GET(DT_NODELABEL(counter0));
static uint32_t alarm_due_ticks;

volatile bool flash_write_alarm_running = false;


static void arm_flash_write_alarm(const struct device *dev)
{
	struct counter_alarm_cfg alarm = {
		.callback = flash_write_alarm_callback,
		.ticks = counter_us_to_ticks(dev, FLASH_WRITE_ALARM_US),
	};
	uint32_t now;

	counter_get_value(dev, &now);
	alarm_due_ticks = (now - alarm.ticks) & AGT_COUNT_MASK;
	counter_set_channel_alarm(dev, 0, &alarm);
}


void flash_write_alarm_callback(const struct device *dev, uint8_t chan_id, uint32_t ticks,
				void *user_data)
{
	ARG_UNUSED(chan_id);
	ARG_UNUSED(user_data);

	if (started_flash_write && !ended_flash_write) {
		uint32_t ticks_late = (alarm_due_ticks - ticks) & AGT_COUNT_MASK;
		uint32_t us_late = counter_ticks_to_us(dev, ticks_late);

		interrupts_during_write++;
		if (us_late > most_late) {
			most_late = us_late;
		}
	}

	if (flash_write_alarm_running) {
		arm_flash_write_alarm(dev);
	}
}

void start_flash_write_alarm(void)
{
	counter_start(flash_write_counter);
	flash_write_alarm_running = true;
	arm_flash_write_alarm(flash_write_counter);
}

void stop_flash_write_alarm(void)
{
	flash_write_alarm_running = false;
	counter_cancel_channel_alarm(flash_write_counter, 0);
}


