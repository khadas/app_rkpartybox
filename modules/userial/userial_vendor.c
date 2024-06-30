/******************************************************************************
 *
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  Filename:      userial_vendor.c
 *
 *  Description:   Contains vendor-specific userial functions
 *
 ******************************************************************************/

#define LOG_TAG "userial_vendor"

#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>

#include "slog.h"
#include "userial_vendor.h"
#include "rk_utils.h"
/******************************************************************************
**  Constants & Macros
******************************************************************************/

#ifndef VNDUSERIAL_DBG
#define VNDUSERIAL_DBG FALSE
#endif

#if (VNDUSERIAL_DBG == TRUE)
#define VNDUSERIALDBG(param, ...) {ALOGD(param, ## __VA_ARGS__);}
#else
#define VNDUSERIALDBG(param, ...) {}
#endif

#define VND_PORT_NAME_MAXLEN    256

/*****************************************************************************
**   Helper Functions
*****************************************************************************/

/*******************************************************************************
**
** Function        userial_to_tcio_baud
**
** Description     helper function converts USERIAL baud rates into TCIO
**                  conforming baud rates
**
** Returns         TRUE/FALSE
**
*******************************************************************************/
bool userial_to_tcio_baud(uint32_t cfg_baud, uint32_t *baud)
{
    if (cfg_baud == USERIAL_BAUD_115200)
        *baud = B115200;
    else if (cfg_baud == USERIAL_BAUD_4M)
        *baud = B4000000;
    else if (cfg_baud == USERIAL_BAUD_3M)
        *baud = B3000000;
    else if (cfg_baud == USERIAL_BAUD_2M)
        *baud = B2000000;
    else if (cfg_baud == USERIAL_BAUD_1_5M)
        *baud = B1500000;
    else if (cfg_baud == USERIAL_BAUD_1M)
        *baud = B1000000;
    else if (cfg_baud == USERIAL_BAUD_921600)
        *baud = B921600;
    else if (cfg_baud == USERIAL_BAUD_460800)
        *baud = B460800;
    else if (cfg_baud == USERIAL_BAUD_230400)
        *baud = B230400;
    else if (cfg_baud == USERIAL_BAUD_57600)
        *baud = B57600;
    else if (cfg_baud == USERIAL_BAUD_38400)
        *baud = B38400;
    else if (cfg_baud == USERIAL_BAUD_19200)
        *baud = B19200;
    else if (cfg_baud == USERIAL_BAUD_9600)
        *baud = B9600;
    else if (cfg_baud == USERIAL_BAUD_1200)
        *baud = B1200;
    else if (cfg_baud == USERIAL_BAUD_600)
        *baud = B600;
    else
    {
        ALOGE( "userial vendor open: unsupported baud idx %i", cfg_baud);
        *baud = B115200;
        return false;
    }

    return true;
}

/*****************************************************************************
**   Userial Vendor API Functions
*****************************************************************************/


/*******************************************************************************
**
** Function        userial_vendor_open_inner
**
** Description     Open the serial port with the given configuration
**
** Returns         device fd
**
*******************************************************************************/
int userial_vendor_open_inner(char *dev, int speed, uint16_t fmt)
{
    struct termios termios;
    uint32_t baud;
    uint8_t data_bits;
    uint16_t parity;
    uint8_t stop_bits;
    int fd;

    if (!userial_to_tcio_baud(speed, &baud))
    {
        return -1;
    }

    if(fmt & USERIAL_DATABITS_8)
        data_bits = CS8;
    else if(fmt & USERIAL_DATABITS_7)
        data_bits = CS7;
    else if(fmt & USERIAL_DATABITS_6)
        data_bits = CS6;
    else if(fmt & USERIAL_DATABITS_5)
        data_bits = CS5;
    else
    {
        ALOGE("userial vendor open: unsupported data bits");
        return -1;
    }

    if(fmt & USERIAL_PARITY_NONE)
        parity = 0;
    else if(fmt & USERIAL_PARITY_EVEN)
        parity = PARENB;
    else if(fmt & USERIAL_PARITY_ODD)
        parity = (PARENB | PARODD);
    else
    {
        ALOGE("userial vendor open: unsupported parity bit mode");
        return -1;
    }

    if(fmt & USERIAL_STOPBITS_1)
        stop_bits = 0;
    else if(fmt & USERIAL_STOPBITS_2)
        stop_bits = CSTOPB;
    else
    {
        ALOGE("userial vendor open: unsupported stop bits");
        return -1;
    }

    ALOGI("userial vendor open: opening %s", dev);

    if ((fd = open(dev, O_RDWR)) == -1)
    {
        ALOGE("userial vendor open: unable to open %s", dev);
        return -1;
    }

    tcflush(fd, TCIOFLUSH);

    tcgetattr(fd, &termios);
    cfmakeraw(&termios);
    termios.c_cflag |= (stop_bits);
    termios.c_cflag &= (~CRTSCTS);
    tcsetattr(fd, TCSANOW, &termios);
    tcflush(fd, TCIOFLUSH);

    tcsetattr(fd, TCSANOW, &termios);
    tcflush(fd, TCIOFLUSH);
    tcflush(fd, TCIOFLUSH);

    /* set input/output baudrate */
    cfsetospeed(&termios, baud);
    cfsetispeed(&termios, baud);
    tcsetattr(fd, TCSANOW, &termios);

    ALOGD("device fd = %d open", fd);
    return fd;
}

int userial_uart_open_with_opt(char *dev, uint32_t speed, uint16_t fmt) {
    int fd;
    ALOGD("%s Device: %s, Speed: %u fmt:0x%04x\n", __func__, dev, speed, fmt);
    fd = userial_vendor_open_inner(dev, speed, fmt);

    return fd;
}

int userial_uart_open_without_opt(char *dev, uint32_t speed) {
    int fd;
    uint16_t fmt = USERIAL_DATABITS_8 | USERIAL_PARITY_NONE | USERIAL_STOPBITS_1;
    ALOGD("%s Device: %s, Speed: %u fmt:0x%04x\n", __func__, dev, speed, fmt);

    fd = userial_vendor_open_inner(dev, speed, fmt);

    return fd;
}

int userial_uart_open_impl(char *dev, uint32_t speed, uint16_t optional_param) {
    int fd;
    if(!optional_param)
        optional_param = USERIAL_DATABITS_8 | USERIAL_PARITY_NONE | USERIAL_STOPBITS_1;
    fd = userial_vendor_open_inner(dev, speed, optional_param);
    ALOGD("%s Device: %s, Speed: %u fmt:0x%04x\n", __func__, dev, speed, optional_param);
    return fd;
}

/*******************************************************************************
**
** Function        userial_vendor_close
**
** Description     Conduct vendor-specific close work
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_close(int fd)
{
    int result;

    if (fd == -1)
        return;

    ALOGD("device fd = %d close", fd);
    // flush Tx before close to make sure no chars in buffer
    tcflush(fd, TCIOFLUSH);
    if ((result = close(fd)) < 0)
        ALOGE( "close(fd:%d) FAILED result:%d", fd, result);
}

/*******************************************************************************
**
** Function        userial_vendor_set_baud
**
** Description     Set new baud rate
**
** Returns         None
**
*******************************************************************************/
void userial_vendor_set_baud(int fd, uint8_t userial_baud)
{
    struct termios termios;
    uint32_t tcio_baud;

    tcgetattr(fd, &termios);
    cfmakeraw(&termios);

    userial_to_tcio_baud(userial_baud, &tcio_baud);

    cfsetospeed(&termios, tcio_baud);
    cfsetispeed(&termios, tcio_baud);
    tcsetattr(fd, TCSANOW, &termios);
}

void userial_dump_data(uint8_t *data, uint16_t total) {
    ALOGW_PURE("raw data[%d]= {", total);
    for(int i= 0; i< total; i++) {
        ALOGW_PURE("%02x ", data[i]);
    }
    ALOGW_PURE("}\n");
}

uint16_t userial_vendor_send_data(int fd, uint8_t *data, uint16_t total) {
    int ret;
    assert(data != NULL);
    assert(total > 0);

    uint16_t len = total;
    uint16_t sended = 0;

    ALOGD("%s sending\n", __func__);
    userial_dump_data(data, total);

    while (len > 0) {
        OSI_NO_INTR(ret = write(fd, data + sended, len));
        switch (ret) {
            case -1:
            ALOGE("In %s, error writing to the uart serial port: %s", __func__, strerror(errno));
            goto done;
        case 0:
            // If we wrote nothing, don't loop more because we
            // can't go to infinity or beyond, ohterwise resend data
            ALOGE("%s, ret %zd", __func__, ret);
            goto done;
        default:
            sended += ret;
            len -= ret;
            break;
        }
    }

done:
    return sended;
}