// xmodem.c
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/dfu/flash_img.h>
#include <zephyr/dfu/mcuboot.h>
#include "xmodem.h"
#include "comms.h"
// Called by menu.c to download and program the new incoming image
unsigned char XmodemDownloadAndProgramFlash(void)
{
/*
XmodemDownloadAndProgramFlash() downloads data using the XModem protocol
developed in 1977 by Ward Christensen and programs it into the MCUboot
secondary slot (image-1).
The routine detects XM_ERRORs due to XM_TIMEOUTs, comms XM_ERRORs or invalid checksums.
The routine reports the following to the caller:
-Success
-Invalid address    (not implemented here as the address is checked prior to this
                    function being called)
-Comms XM_ERROR
-XM_TIMEOUT XM_ERROR
-Failure to program flash




Expects:
--------
Nothing - the destination is fixed by flash_img_init() to the MCUboot secondary
slot, so (unlike the original FSP version) there is no FlashAddress argument.


Returns:
--------
XM_OK               -   Download and Flash programming performed ok
XM_ADDRESS_ERROR    -   Address was either not on a 128-byte boundary or not in valid Flash
XM_COMMS_ERROR      -   Comms parity, framing or overrun XM_ERROR
XM_TIMEOUT          -   Transmitter did not respond to this receiver
XM_PROG_FAIL        -   Failed to program one or more bytes of the Flash memory
*/
    unsigned char   xm_ret;
    unsigned char   ExpectedBlkNum;
    unsigned char   RetryCounter;
    unsigned char   RxByteBufferIndex;
    unsigned char   checksum;
    unsigned char   StartCondition;
    unsigned char   RxByteBuffer[132] __aligned(4);
    uint8_t         tx_byte __aligned(4);
    uint32_t        rx_len;
    int             err;
    static struct flash_img_context flash_ctx;
    /* Target the MCUboot secondary slot (image-1). */
    // flash_img_init initalises context for writing to the flash
    if (flash_img_init(&flash_ctx) != 0)
    {
        return XM_PROG_FAIL;
    }
    // Drop anything typed after the menu choice, otherwise every frame is a byte out of step
    comms_flush();
    // first xmodem block number is 1
    ExpectedBlkNum = 1;
    StartCondition = 0;
    while(1)
    {
        //  {1}
        //  initialise Rx attempts
        RetryCounter = 10;


        //  decrement Rx attempts counter & get Rx byte with a 10 sec TIMEOUT repeat until Rx attempts is 0
        xm_ret = XM_TOUT;
        while ( (RetryCounter > 0) && (xm_ret == XM_TOUT) )
        {
            if (StartCondition == 0)
            {
                //  if this is the start of the xmodem frame
                //  send a NAK to the transmitter
                tx_byte = NAK;
                comms_send(&tx_byte, 1);    // Kick off the XModem transfer


                /* Request 132 bytes from host with a delay of 10 seconds */
                rx_len = 132;
                memset((void *)&RxByteBuffer[0], 0, 132);
                err = comms_read((uint8_t *)&RxByteBuffer[0], &rx_len, 10000);
                if(COMMS_OK == err)
                {
                    xm_ret = OK;
                }
                else if(COMMS_TIMEOUT == err)
                {
                    xm_ret = XM_TOUT;
                }
                else
                {
                    xm_ret = XM_ERROR;
                }
            }
            else
            {
                /* Request 132 bytes from host with a delay of 1 second */
                rx_len = 1;
                memset((void *)&RxByteBuffer[0], 0, 132);
                /* Get the first byte to check if it is EOT */
                err = comms_read((uint8_t *)&RxByteBuffer[0], &rx_len, 1000);
                if(COMMS_OK == err)
                {
                    xm_ret = OK;
                    if (EOT != RxByteBuffer[0])
                    {
                        /* Receive the rest of the frame */
                        rx_len = 132 - 1;
                        err = comms_read((uint8_t *)&RxByteBuffer[1], &rx_len, 1000);
                        if(COMMS_OK == err)
                        {
                            xm_ret = OK;
                        }
                        else if(COMMS_TIMEOUT == err)
                        {
                            xm_ret = XM_TOUT;
                        }
                        else
                        {
                            xm_ret = XM_ERROR;
                        }
                    }
                }
                else if(COMMS_TIMEOUT == err)
                {
                    xm_ret = XM_TOUT;
                }
                else
                {
                    xm_ret = XM_ERROR;
                }
            }
            RetryCounter--;
        }


        StartCondition = 1;


        if ( xm_ret == XM_ERROR )
        {
            return ( XM_COMMS_ERROR );
        }
        else if ( xm_ret == XM_TOUT )
        {
            //  if timed out after 10 attempts
            printk("timeout, %u bytes in last read\n", rx_len);
            return ( XM_TIMEOUT );
        }
        else
        {
            // if first received byte is 'end of frame'
            // return ACK to sender
            if ( RxByteBuffer[0] == EOT )
            {
                /* Flush the final buffered write before acknowledging. */
                err = flash_img_buffered_write(&flash_ctx, NULL, 0, true);
                if (err != 0)
                {
                    tx_byte = NAK;
                    comms_send(&tx_byte, 1);
                    tx_byte = CAN;
                    comms_send(&tx_byte, 1);
                    return ( XM_PROG_FAIL );
                }
                tx_byte = ACK;
                comms_send(&tx_byte, 1);
                return( XM_OK );
            }
            else
            {
                // data Rx ok
                // calculate the checksum of the data bytes only
                int cs = 0;
                for (RxByteBufferIndex=0; RxByteBufferIndex<128; RxByteBufferIndex++)
                {
                    cs += RxByteBuffer[RxByteBufferIndex + 3];
                }


                /* This int to unsigned char conversion needed to eliminate conversion warning with checksum calculation. */
                checksum = (unsigned char)cs;


                //  if SOH, BLK#, 255-BLK# or checksum not valid
                //  (BLK# is valid if the same as expected blk counter or is 1 less
                if ( !( (RxByteBuffer[0] == SOH) && ((RxByteBuffer[1] == ExpectedBlkNum) || (RxByteBuffer[1] == ExpectedBlkNum - 1) ) && (RxByteBuffer[2] + RxByteBuffer[1] == 255 ) && (RxByteBuffer[131] == checksum) ) )
                {
                    //  send NAK and loop back to (1)
                    printk("bad frame: %02x %02x %02x, sum %02x expected %02x\n",
                           RxByteBuffer[0], RxByteBuffer[1], RxByteBuffer[2],
                           RxByteBuffer[131], checksum);
                    tx_byte = NAK;
                    comms_send(&tx_byte, 1);
                }
                else
                {
                    //  if blk# is expected blk num
                    if ( RxByteBuffer[1] == ExpectedBlkNum )
                    {
                        //  Program the received data into flash
                        /* flash_img streams to the secondary slot and tracks the write
                           offset, so there is no manual address and no interrupt juggling
                           (the flash driver owns read-while-write). */
                        err = flash_img_buffered_write(&flash_ctx, &RxByteBuffer[3], 128, false);


                        if(0 == err)
                        {
                            //  if prog routine passed ok increment block number
                            //  (flash_img advances the flash offset internally)
                            ExpectedBlkNum++;
                            tx_byte = ACK;
                            comms_send(&tx_byte, 1);


                            //  loop back to (1)
                        }
                        else
                        {
                            // prog fail
                            tx_byte = NAK;
                            comms_send(&tx_byte, 1);
                            // cancel xmodem download
                            tx_byte = CAN;
                            comms_send(&tx_byte, 1);


                            return( XM_PROG_FAIL );
                        }
                    }
                    else
                    {
                        //  block number is valid but this data block has already been received
                        //  send ACK and loop to (1)
                        tx_byte = ACK;
                        comms_send(&tx_byte, 1);
                    }
                }
            }
        }
    }
}