#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "pbox_interface.h"
#include "pbox_common.h"  // Assuming this header file contains the upper layer's enum definition
#include "slog.h"

InterfacePlayStatus map_play_status_to_interface(uint32_t status) {
    switch (status) {
        case IDLE:
            return INTERFACE_IDLE;
        case PLAYING:
            return INTERFACE_PLAYING;
        case _PAUSE:
            return INTERFACE_PAUSE;
        case _STOP:
            return INTERFACE_STOP;
        default:
            return INTERFACE_PLAY_NUM;
    }
}

int32_t get_maxMicVolume(void) {
    return MAX_MIC_PHONE_VOLUME;
}

int32_t get_minMicVolume(void) {
    return MIN_MIC_PHONE_VOLUME;
}

int adckey_read(int fd) {
    char buff[6]= {0};
    int value;
    lseek(fd, 0, SEEK_SET);
    int ret = read(fd, buff, sizeof(buff));
    if (ret < 0) {
        char str[32] = {0};
        snprintf(str, sizeof(str)-1, "%s fd:%02d, ret:%d", __func__, fd, ret);
        perror(str);
        return -1;
    }
    assert(ret==0);

    buff[strlen(buff)-1] = 0;
    value = atoi(buff);
    if(value > (hal_max_saradc_val() -hal_min_saradc_val())/2) {
        value = value + (hal_max_saradc_val()-hal_min_saradc_val())/100;
    }

    //ALOGD("%s fd:%d buff:%s keyValue=%d\n", __func__, fd, buff, value);
    if(value > hal_max_saradc_val()) value = hal_max_saradc_val();
    return value;
}

#define DEV_SARADC3_BUTTON    "/sys/bus/iio/devices/iio:device0/in_voltage3_raw"
bool is_rolling_board(void) {
    int fd;
    int value;

    if (ENABLE_EXT_BT_MCU == 1)
        return false;
    fd = open(DEV_SARADC3_BUTTON, O_RDONLY);
    if(fd <= 0) {
        ALOGE("%s err: %d\n", __func__, fd);
        return false;
    }

    value = adckey_read(fd);
    close(fd);
    ALOGE("%s value: %d\n", __func__, value);
    if (value > 1000)                                 //rolling board is 1004
        return true;
    else
        return false;
}