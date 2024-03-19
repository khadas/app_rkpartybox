#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include "pbox_socket.h"
#include "pbox_socketpair.h"

#include "pbox_common.h"
#include "pbox_keyadcSara.h"

#if ENABLE_SARAADC
#define MAX_SARA_ADC 1023
#define MIN_SARA_ADC 0

#define DEV_MIC1_BUTTON_BASS    "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define DEV_MIC1_BUTTON_TREBLE  "/sys/bus/iio/devices/iio:device0/in_voltage1_raw"
#define DEV_MIC1_BUTTON_REVERB  "/sys/bus/iio/devices/iio:device0/in_voltage2_raw"
#define DEV_MIC2_BUTTON_BASS    "/sys/bus/iio/devices/iio:device0/in_voltage3_raw"
#define DEV_MIC2_BUTTON_TREBLE  "/sys/bus/iio/devices/iio:device0/in_voltage4_raw"
#define DEV_MIC2_BUTTON_REVERB  "/sys/bus/iio/devices/iio:device0/in_voltage5_raw"

typedef struct {
    keycode_t index;
    char *dev;
} adckey_t;

const adckey_t adcKeyTable[KNOB_BUTTON_NUM] = {
    { MIC1_BUTTON_BASS,     DEV_MIC1_BUTTON_BASS    },
    { MIC1_BUTTON_TREBLE,   DEV_MIC1_BUTTON_TREBLE  },
    { MIC1_BUTTON_REVERB,   DEV_MIC1_BUTTON_REVERB  },
    { MIC2_BUTTON_BASS,     DEV_MIC2_BUTTON_BASS    },
    { MIC2_BUTTON_TREBLE,   DEV_MIC2_BUTTON_TREBLE  },
    { MIC2_BUTTON_REVERB,   DEV_MIC2_BUTTON_REVERB  },
};

int unix_socket_keyscan_notify_msg(void *info, int length)
{
    unix_socket_notify_msg(PBOX_MAIN_KEYSCAN, info, length);
}

void keyscan_pbox_notify_knob_event(keycode_t keycode, uint32_t value) {
    pbox_keyscan_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_KEYSCAN_KNOB_EVENT,
    };
    msg.keyinfo.keycode = keycode;
    msg.keyinfo.value = value;
    ALOGD("%s keycode:%d, value:%d\n", __func__, keycode, value);
    unix_socket_keyscan_notify_msg(&msg, sizeof(pbox_keyscan_msg_t));
}

static int adckey_init_fd(int fd[], int num) {
    for (int i = 0; i < num; i++) {
        keycode_t index = adcKeyTable[i].index;
        fd[index] = open(adcKeyTable[index].dev, O_RDONLY);
        if(fd[index] <= 0) {
            ALOGE("%s index:%d\n", __func__, index, fd[index]);
            return -1;
        }
    }
    return 0;
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
    if(value > (MAX_SARA_ADC-MIN_SARA_ADC)/2) {
        value = value + (MAX_SARA_ADC-MIN_SARA_ADC)/100;
    }

    //ALOGD("%s fd:%d buff:%s keyValue=%d\n", __func__, fd, buff, value);
    return value;
}

static void *adckey_sara_detect_server(void *arg)
{
    bool sara_init = false;
    int tmp, new;
    #define SARA_RANGE_100(X) X*100/(MAX_SARA_ADC-MIN_SARA_ADC)
    uint32_t saraSample[KNOB_BUTTON_NUM];
    int adckey_fd[KNOB_BUTTON_NUM];
    pthread_setname_np(pthread_self(), "pbox_sarakey");
    ALOGD("%s hello\n", __func__);
    PBOX_ARRAY_SET(adckey_fd, -1, sizeof(adckey_fd)/sizeof(adckey_fd[0]));
    if(adckey_init_fd(adckey_fd, KNOB_BUTTON_NUM) < 0) {
        ALOGE("%s fail\n", __func__);
        return (void*) 0;
    }

    while(true) {
        for(int i = 0; i < KNOB_BUTTON_NUM; i++) {
            keycode_t index = adcKeyTable[i].index;
            new = adckey_read(adckey_fd[index]);
            //new = SARA_RANGE_100(tmp);
            if(!sara_init||((new>= 0) && (new != saraSample[index]))) {
                if(!sara_init) {
                    saraSample[index] = new;
                    keyscan_pbox_notify_knob_event(index, SARA_RANGE_100(new));
                    continue;
                }
                if(abs(new - saraSample[index]) < (MAX_SARA_ADC-MIN_SARA_ADC)/200) {
                    continue;
                }

                saraSample[index] = new;
                keyscan_pbox_notify_knob_event(index, SARA_RANGE_100(new));
                ALOGD("i=%d,adckey button[%d] changing: %d->%d upstream:%d------------>\n"
                            , i, index, saraSample[index], new, SARA_RANGE_100(new));
            }
        }

        if(!sara_init)
            sara_init = true;
        usleep(200*1000);
    }
}

int pbox_create_KeyadcSaraTask(void)
{
    pthread_t keyadcSara_tid;
    int err;

    err = pthread_create(&keyadcSara_tid, NULL, &adckey_sara_detect_server, NULL);
    if (err != 0)
        ALOGE("cant create keyadc_sara_detect_server thread!");
    return err;
}
#endif