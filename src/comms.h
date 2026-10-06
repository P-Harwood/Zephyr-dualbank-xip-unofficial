#ifndef COMMS_H_
#define COMMS_H_


#include <stdint.h>
#include <errno.h>


#define COMMS_UART


#define NO_TIMEOUT      0xFFFFFFFFU


#define COMMS_OK        0
#define COMMS_TIMEOUT   (-EAGAIN)


int  comms_open(void);
void comms_send(uint8_t *p_src, uint32_t len);
int  comms_read(uint8_t *p_dest, uint32_t *len, uint32_t timeout_milliseconds);
void comms_flush(void);

#endif
