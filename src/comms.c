#include "comms.h"


#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>


#if defined(COMMS_USB)
#include <zephyr/usb/usb_device.h>
#define COMMS_DEV_NODE  DT_NODELABEL(cdc_acm_uart0)
#else
#define COMMS_DEV_NODE  DT_CHOSEN(zephyr_console)
#endif


#define RX_QUEUE_BYTES  512


static const struct device *const comms_dev = DEVICE_DT_GET(COMMS_DEV_NODE);


K_MSGQ_DEFINE(rx_q, 1, RX_QUEUE_BYTES, 1);

void comms_flush(void)
{
    k_msgq_purge(&rx_q);
}

static void comms_rx_isr(const struct device *dev, void *user_data)
{
    ARG_UNUSED(user_data);

    uart_irq_update(dev);

    if (uart_irq_rx_ready(dev)) {
        uint8_t byte;

        while (uart_fifo_read(dev, &byte, 1) == 1) {
            (void)k_msgq_put(&rx_q, &byte, K_NO_WAIT);
        }
    }
}




int comms_open(void)
{
    if (!device_is_ready(comms_dev)) {
        return -ENODEV;
    }


#if defined(COMMS_USB)
    int rc = usb_enable(NULL);
    if (rc != 0 && rc != -EALREADY) {
        return rc;
    }
    uint32_t dtr = 0U;
    while (!dtr) {
        uart_line_ctrl_get(comms_dev, UART_LINE_CTRL_DTR, &dtr);
        k_msleep(10);
    }
#endif
    uart_irq_callback_user_data_set(comms_dev, comms_rx_isr, NULL);
    uart_irq_rx_enable(comms_dev);


    return 0;
}


void comms_send(uint8_t *p_src, uint32_t len)
{
    for (uint32_t i = 0U; i < len; i++) {
        uart_poll_out(comms_dev, p_src[i]);
    }
}


int comms_read(uint8_t *p_dest, uint32_t *len, uint32_t timeout_milliseconds)
{
    const bool    forever  = (timeout_milliseconds == NO_TIMEOUT);
    const int64_t deadline = forever ? 0 : (k_uptime_get() + timeout_milliseconds);




    
for (uint32_t i = 0U; i < *len; i++) {
        k_timeout_t wait;


        if (forever) {
            wait = K_FOREVER;
        } else {
            int64_t remaining = deadline - k_uptime_get();
            if (remaining <= 0) {
                *len = i;
                return COMMS_TIMEOUT;
            }
            wait = K_MSEC(remaining);
        }
        if (k_msgq_get(&rx_q, &p_dest[i], wait) != 0) {
            *len = i;
            return COMMS_TIMEOUT;
        }
    }
    return COMMS_OK;
}
