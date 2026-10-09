#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);


void blinky_thread_entry(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    if (!gpio_is_ready_dt(&led)) {
        return;
    }
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    while (1) {
        gpio_pin_toggle_dt(&led);
        k_msleep(2000);
    }
}
K_THREAD_DEFINE(blinky_tid, 1024, blinky_thread_entry,
                NULL, NULL, NULL, 7, 0, 0);
