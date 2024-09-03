/*
 *  Copyright (c) 2020 Rockchip Electronics Co.Ltd
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */
#include <linux/version.h>
#include <linux/input.h>
#include <dirent.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>
#include <malloc.h>
#include <assert.h>
#include <sys/socket.h>
#include "pbox_keyscan.h"
#include "pbox_socket.h"
#include "pbox_socketpair.h"
#include "pbox_common.h"
#include "pbox_keyscan_app.h"
#include "os_minor_type.h"
#include "os_task.h"
#include "hal_partybox.h"
#include "pbox_interface.h"

#ifdef RK_VAD
#include "vad.h"
#endif

#define ADCKEY_MIC1_GPIO 79
#define ADCKEY_MIC2_GPIO 80

#define ADCKEY_GPIO_DIRECTION 1

#if ENABLE_EXT_BT_MCU
#define DEV_MIC1_BUTTON_BASS    "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define DEV_MIC1_BUTTON_TREBLE  "/sys/bus/iio/devices/iio:device0/in_voltage1_raw"
#define DEV_MIC1_BUTTON_REVERB  "/sys/bus/iio/devices/iio:device0/in_voltage2_raw"
#define DEV_MIC2_BUTTON_BASS    "/sys/bus/iio/devices/iio:device0/in_voltage3_raw"
#define DEV_MIC2_BUTTON_TREBLE  "/sys/bus/iio/devices/iio:device0/in_voltage4_raw"
#define DEV_MIC2_BUTTON_REVERB  "/sys/bus/iio/devices/iio:device0/in_voltage5_raw"
#else
#define DEV_SARADC4_BUTTON    "/sys/bus/iio/devices/iio:device0/in_voltage4_raw"
#define DEV_SARADC5_BUTTON    "/sys/bus/iio/devices/iio:device0/in_voltage5_raw"
#endif



#if ENABLE_EXT_BT_MCU
struct _adcKeyTable {
    hal_key_t index;
    char *dev;
};
const struct _adcKeyTable adcKeyTable[] = {
    { HKEY_MIC1BASS,    DEV_MIC1_BUTTON_BASS    },
    { HKEY_MIC1TREB,    DEV_MIC1_BUTTON_TREBLE  },
    { HKEY_MIC1REVB,    DEV_MIC1_BUTTON_REVERB  },
    { HKEY_MIC2BASS,    DEV_MIC2_BUTTON_BASS    },
    { HKEY_MIC2TREB,    DEV_MIC2_BUTTON_TREBLE  },
    { HKEY_MIC2REVB,    DEV_MIC2_BUTTON_REVERB  },
};
#else
struct _adcKeyTable {
    hal_key_t index;
    char *dev;
    int gpio1_value;
    int gpio2_value;
};
const struct _adcKeyTable adcKeyTable[] = {
    { HKEY_MIC1BASS,   DEV_SARADC4_BUTTON,   0,   0},
    { HKEY_MIC2BASS,   DEV_SARADC5_BUTTON,   0,   0},
    { HKEY_MIC1REVB,   DEV_SARADC4_BUTTON,   0,   1},
    { HKEY_MIC2REVB,   DEV_SARADC5_BUTTON,   0,   1},
    { HKEY_MIC1TREB,   DEV_SARADC4_BUTTON,   1,   0},
    { HKEY_MIC2TREB,   DEV_SARADC5_BUTTON,   1,   0},
    { HKEY_MIC1_VOL,   DEV_SARADC4_BUTTON,   1,   1},
    { HKEY_MIC2_VOL,   DEV_SARADC5_BUTTON,   1,   1},
};
#endif

int adckey_read(int fd);

bool is_saradc_board() {
    return hal_get_sara_adc_usage();
}

static int adckey_init_fd(int fd[], int num) {
    int i, j;

    for (i = 0; i < num; i++) {
        for (j = 0; j < i; j++) {
            if (!strcmp(adcKeyTable[i].dev, adcKeyTable[j].dev))
                break;
        }
        if (j < i) {
            fd[i] = fd[j];
        } else {
            fd[i] = open(adcKeyTable[i].dev, O_RDONLY);
            if(fd[i] <= 0) {
                ALOGE("%s index:%d\n", __func__, i, fd[i]);
                return -1;
            }
        }
    }
    return 0;
}

static int adckey_deinit_fd(int fd[], int num) {
    int i, j;

    for (i = 0; i < num; i++) {
        for (j = 0; j < i; j++) {
            if (!strcmp(adcKeyTable[i].dev, adcKeyTable[j].dev))
                break;
        }
        if (j == i) {
            close(fd[i]);
        }
        fd[i] = -1;
    }
    return 0;
}

void adckey_init_gpio() {
    os_gpio_init(ADCKEY_MIC1_GPIO);
    os_gpio_init(ADCKEY_MIC2_GPIO);
    os_set_gpio_pin_direction(ADCKEY_MIC1_GPIO, ADCKEY_GPIO_DIRECTION);
    os_set_gpio_pin_direction(ADCKEY_MIC2_GPIO, ADCKEY_GPIO_DIRECTION);
}

void adckey_deinit_gpio() {
    os_gpio_deinit(ADCKEY_MIC1_GPIO);
    os_gpio_deinit(ADCKEY_MIC2_GPIO);
}

void switch_adckey_gpio_chn(int num) {
    #if !ENABLE_EXT_BT_MCU
    os_set_gpio_value(ADCKEY_MIC1_GPIO, adcKeyTable[num].gpio1_value);
    os_set_gpio_value(ADCKEY_MIC2_GPIO, adcKeyTable[num].gpio2_value);
    #endif
}

static float convert_sara_to_standard(int group, int value) {
    switch (group) {
        case HKEY_MIC2BASS:
        case HKEY_MIC1BASS: {
            return ORG2TARGET(value, float, MIN_BASS_VALUE, MAX_BASS_VALUE, 0, hal_max_saradc_val());
        } break;

        case HKEY_MIC1TREB:
        case HKEY_MIC2TREB: {
            return ORG2TARGET(value, float, MIN_TREBLE_VALUE, MAX_TREBLE_VALUE, 0, hal_max_saradc_val());
        } break;

        case HKEY_MIC1REVB:
        case HKEY_MIC2REVB: {
            return ORG2TARGET(value, float, MIN_REVERB_VALUE, MAX_REVERB_VALUE, 0, hal_max_saradc_val());
        } break;

        case HKEY_MIC1_VOL: {
            float vol = ORG2TARGET(value, float, MIN_MIC_PHONE_VOLUME, MAX_MIC_PHONE_VOLUME, 0, hal_max_saradc_val());
            return vol;
        } break;
        case HKEY_MIC2_VOL: {
            float vol = ORG2TARGET(value, float, MIN_MIC_PHONE_VOLUME, MAX_MIC_PHONE_VOLUME, 0, hal_max_saradc_val());
            return vol;
        } break;
    }

    return value;
}

#define DEV_INPUT_EVENT     "/dev/input"
#define EVENT_DEV_NAME      "event"
#define BITS_PER_LONG       (sizeof(long) * 8)

extern const struct dot_key support_keys[];
extern const size_t support_keys_size;
pthread_mutex_t ev_mutex;

char rk29_keypad[] = {"rk29-keypad"};
char gpio_keys[] = {"gpio-keys"};
char rk29_rotary[] = {"rotary"};
char adc_keys[] = {"adc-keys"};
char rk8xx_pwrkey[] = {"rk8xx_pwrkey"};
char aw9163_ts[] = {"aw9163_ts"};


struct dot_key key_read;
struct dot_key current_dot_key;
struct dot_support_event_type support_event[] =
{
    {KEY_EVENT, rk29_keypad, 0},
    {KEY_EVENT, gpio_keys, 1},
    {KEY_EVENT, adc_keys, 2},
    {KEY_EVENT, rk8xx_pwrkey, 3},
    {KEY_EVENT, aw9163_ts, 4},
    {KEY_EVENT, rk29_rotary, 5},
};

int unix_socket_keyscan_notify_msg(void *info, int length) {
    unix_socket_notify_msg(PBOX_MAIN_KEYSCAN, info, length);
}

/**
 * Print   device information (no events). This information includes
 * version numbers, device name and all bits supported by this device.
 *
 * @param fd The file descriptor to the device.
 * @return 0 on success or 1 otherwise.
 */
int print_device_info(int fd) {
    int version;
    unsigned short id[4];
    char name[256] = "Unknown";

    if(ioctl(fd, EVIOCGVERSION, &version)) {
        perror("evtest: can't get version\n");
        return 1;
    }

    ALOGI("Input driver version is %d.%d.%d\n",
               version >> 16, (version >> 8) & 0xff, version & 0xff);

    ioctl(fd, EVIOCGID, id);
    ALOGI("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
               id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

    ioctl(fd, EVIOCGNAME(sizeof(name)), name);
    ALOGI("Input device name: \"%s\"\n", name);

    return 0;
}

/**
 * Filter for the AutoDevProbe scandir on /dev/input.
 *
 * @param dir The current directory entry provided by scandir.
 *
 * @return Non-zero if the given directory entry starts with "event", or zero
 * otherwise.
 */
int is_event_device(const struct dirent *dir) {
    return strncmp(EVENT_DEV_NAME, dir->d_name, 5) == 0;
}

/**
 * Scans all /dev/input/event*, open muli event devices
 * by specifying event type.
 * @param fds: the address that fds store
 * @return fd counts.
 .
 */
int find_multi_event_dev(int event_type, int *fds, int *types) {
    struct dirent **namelist;
    int i, ndev;
    char fname[64];
    int fd = -1, ret = -1;
    char name[256] = "???";
    int count = 0;

    if(event_type < EVENT_START || event_type > EVENT_END) {
        fprintf(stderr, "Invalid event type:%d\n", event_type);
        return 0;
    }

    ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, versionsort);
    if(ndev <= 0) {
        return 0;
    }

    for(i = 0; i < ndev; i++) {
        int j = 0;
        int events_nums = sizeof(support_event) / sizeof(struct dot_support_event_type);
        snprintf(fname, sizeof(fname),
                 "%s/%s", DEV_INPUT_EVENT, namelist[i]->d_name);
        fd = open(fname, O_RDONLY);
        if(fd < 0) {
            continue;
        }
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);
        fprintf(stderr, "%s:	%s,i=%d\n", fname, name, i);
        ret = -1;
        for(j = 0; j < events_nums; j++) {
            if(support_event[j].event_type == event_type && strstr(name, support_event[j].name)) {
                /* find according event device */
                ALOGI("find event device:%s, %d\n", namelist[i]->d_name, support_event[j].key_type);
                ret = fd;
                print_device_info(fd);
                break;
            }
        }
        if(ret < 0) {
            close(fd);
        } else {
            types[count] = support_event[j].key_type;
            fds[count++] = fd;
        }
        os_free(namelist[i]);
    }

    if(count == 0) {
        ALOGI("Can't find device by event_type[%d,%s]\n", event_type, support_event[event_type].name);
    }
    os_free(namelist);
    return count;
}

void *pbox_KeyEvent_server(void * arg) {
    int i;
    bool sara_init = false;
    int tmp, new;
    uint32_t saraSample[sizeof(adcKeyTable)/sizeof(struct _adcKeyTable)];
    int adckey_fd[sizeof(adcKeyTable)/sizeof(struct _adcKeyTable)];
    os_sem_t* quit_sem = os_task_get_quit_sem(os_gettid());

    ALOGD("%s hello\n", __func__);
    PBOX_ARRAY_SET(adckey_fd, -1, sizeof(adckey_fd)/sizeof(adckey_fd[0]));

    bool rolling_board = false;
    rolling_board = is_rolling_board();
    if (rolling_board || is_saradc_board()) {
        if(adckey_init_fd(adckey_fd, sizeof(adcKeyTable)/sizeof(struct _adcKeyTable)) < 0) {
            ALOGE("%s fail\n", __func__);
            return (void*) 0;
        }
    }
    if (rolling_board)
        adckey_init_gpio();

    while(os_sem_trywait(quit_sem) != 0) {
        if (key_read.is_key_valid == 1) {
            for(i = 0;  i < support_keys_size; i++){
                if(key_read.key_code == support_keys[i].key_code && support_keys[i].press_type == K_DQC)
                    break;
            }
            if(i < support_keys_size) {
                usleep(KEY_DOUBLE_CLICK_PERIOD);
            }
            pthread_mutex_lock(&ev_mutex);
            pbox_keyevent_msg_t msg = {0};
            msg.key_code = key_read.key_code;
            msg.key_code_b = key_read.key_code_b;
            msg.press_type = key_read.press_type;
            msg.is_key_valid = key_read.is_key_valid;
            msg.unix_time = key_read.utime;

            ALOGD("%s sock send: code: %d, valid ? %d\n", __func__, msg.key_code, msg.is_key_valid);
            memset(&key_read, 0, sizeof(struct dot_key));
            memset(&current_dot_key, 0, sizeof(struct dot_key));
            unix_socket_keyscan_notify_msg(&msg, sizeof(pbox_keyevent_msg_t));
            pthread_mutex_unlock(&ev_mutex);
        }

        do {
            if (!(rolling_board || is_saradc_board()))
                break;
            for(int i = 0; i < sizeof(adcKeyTable)/sizeof(struct _adcKeyTable); i++) {
                bool notify = false;
                if (rolling_board)
                    switch_adckey_gpio_chn(i);

                new = adckey_read(adckey_fd[i]);
                if(!sara_init) {
                    notify = true;
                    ALOGD("%s %d\n", __func__, __LINE__);
                }

                if(new != saraSample[i]) {
                    if(abs(new - saraSample[i]) > 30) {
                        notify = true;
                        ALOGD("%s %d [%d->%d]\n", __func__, __LINE__, saraSample[i], new);
                    }
                }

                if(notify) {
                    pbox_keyevent_msg_t msg = {0, 0, K_KNOB, 1};
                    msg.key_code = adcKeyTable[i].index;
                    msg.value = convert_sara_to_standard(adcKeyTable[i].index, new);
                    ALOGD("i=%d,adckey button[%d] changing: %d->%d upstream:%f------------>\n"
                                , i, adcKeyTable[i].index, saraSample[i], new, msg.value);
                    unix_socket_keyscan_notify_msg(&msg, sizeof(pbox_keyevent_msg_t));
                    saraSample[i] = new;
                }
            }
            if(!sara_init)
                sara_init = true;
        } while(0);

         usleep(100 * 1000);
    }

    if(rolling_board)
        adckey_deinit_gpio();
    if(rolling_board || is_saradc_board())
        adckey_deinit_fd(adckey_fd, sizeof(adcKeyTable)/sizeof(adcKeyTable[0]));
}

void *pbox_KeyEventScan(void * arg) {
    int key_fds[10], max_fd, key_types[10];
    int rd, ret;
    unsigned int i, j, k, m;
    fd_set rdfs;
    struct input_event ev[64];
    struct timeval sel_timeout_tv;
    int hasLongLongFunc = 0;
    int key_fds_count;
    os_sem_t* quit_sem = os_task_get_quit_sem(os_gettid());

    if(getuid() != 0) {
        fprintf(stderr, "Not running as root, no devices may be available.\n");
        return NULL;
    }

    key_fds_count = find_multi_event_dev(KEY_EVENT, key_fds, key_types);
    ALOGD("--find_multi_event_dev count=%d\n",key_fds_count);
    ALOGD ("vol up %d, down %d, play %d, mode:%d, mic %d\n", KEY_VOLUMEUP, KEY_VOLUMEDOWN, KEY_PLAYPAUSE, KEY_MODE, KEY_MICMUTE);
    if(key_fds_count <= 0) {
        ALOGE("-------------- key event thread exit because event key fd is null ------------\n");
    }

    if(key_fds_count > 0 ) {
        max_fd = 0;
        for(i = 0 ; i < key_fds_count; i++) {
            if(max_fd < key_fds[i]) {
                max_fd = key_fds[i];
            }
        }
    } else {
        fprintf(stderr, "didn't find any valid key fd and valid rotary fd \n");
        return NULL;
    }

    memset(&key_read, 0, sizeof(struct dot_key));
    memset(&current_dot_key, 0x00 ,sizeof(struct dot_key));
    while(os_sem_trywait(quit_sem) != 0) {
        int ev_signaled = 0;
        sel_timeout_tv.tv_sec = 0;
        sel_timeout_tv.tv_usec = 500000;

        FD_ZERO(&rdfs);
        if(key_fds_count > 0)
        for(i = 0 ; i < key_fds_count; i++){
            FD_SET(key_fds[i], &rdfs);
        }

        /*use select timeout to detect long time press, if large than 3 sec, means long time press*/
        ret = select(max_fd + 1, &rdfs, NULL, NULL, &sel_timeout_tv);
        if(ret == 0) {
            pthread_mutex_lock(&ev_mutex);

            if(0 != current_dot_key.key_code) {
                uint64_t tv_now;
                int delta_time;

                tv_now = os_unix_time_ms();
                delta_time = tv_now - current_dot_key.ptime;
                ALOGD("Now: time %ld delta_time=%d\n", tv_now, delta_time);

                if(current_dot_key.is_combain_key && delta_time > KEY_LONG_PRESS_PREIOD) {
                    ALOGD("key[0x%x] [0x%x]  combain key\n", current_dot_key.key_code,  current_dot_key.key_code_b);
                    current_dot_key.press_type = K_COMB;
                    current_dot_key.is_key_valid = 1;
                } else if (delta_time > KEY_LONG_PRESS_PREIOD && delta_time < KEY_VERY_LONG_PRESS_PERIOD) {
                    ALOGD("key[0x%x] is long long key????\n", current_dot_key.key_code);
                    for(j = 0; j < support_keys_size; j++) {
                        if(support_keys[j].key_code == current_dot_key.key_code && K_VLONG == support_keys[j].press_type) {
                            ALOGI("key[0x%x] has longlong key event\n", current_dot_key.key_code);
                            hasLongLongFunc = 1;
                            break;
                        }
                    }
                    if ((current_dot_key.key_code == KEY_MICMUTE &&  delta_time > 5000)) {
                        hasLongLongFunc = 0;
                     } else if ((current_dot_key.key_code == KEY_MICMUTE &&  delta_time < 5000)) {
                        hasLongLongFunc = 1;
                     }
                    if (!hasLongLongFunc) {
                        ALOGI("key[0x%x] long key\n", current_dot_key.key_code);
                        current_dot_key.press_type = K_LONG;
                        current_dot_key.is_key_valid = 1;
                        hasLongLongFunc = 0;
                    }
                } else if(delta_time > KEY_VERY_LONG_PRESS_PERIOD) {
                    ALOGI("key[0x%x] long long key\n", current_dot_key.key_code);
                    current_dot_key.press_type = K_VLONG;
                    current_dot_key.is_key_valid = 1;
                    hasLongLongFunc = 0;
                }

                if(current_dot_key.is_key_valid) {
                    memcpy(&key_read, &current_dot_key, sizeof(struct dot_key));
                    memset(&current_dot_key, 0x00 ,sizeof(struct dot_key));
                }
            }
            pthread_mutex_unlock(&ev_mutex);
        } else if(ret == -1) {
            perror("select error\n");
            continue;
        }

        for(k = 0; k < key_fds_count; k++) {
            int key_fd = key_fds[k];
            if(FD_ISSET(key_fd, &rdfs)) {
                rd = read(key_fd, ev, sizeof(ev));

                if(rd < (int) sizeof(struct input_event)) {
                    ALOGD("[key]expected %d bytes, got %d, ignore the value\n", (int) sizeof(struct input_event), rd);
                    continue;
                }

                pthread_mutex_lock(&ev_mutex);
                for(i = 0; i < rd / sizeof(struct input_event); i++) {
                    int type, code;

                    type = ev[i].type;
                    code = hal_key_convert_kernel_to_upper(ev[i].code);
                    uint32_t ev_time = ev[i].time.tv_sec*1000+ ev[i].time.tv_usec/1000;
                    ALOGD("Event: time %ld.%06ld,\n", ev[i].time.tv_sec, ev[i].time.tv_usec);
                    #ifdef RK_VAD
                    clear_vad_count();//has key event,clear vad count.
                    #endif

                    if(type == EV_SYN) {
                        ALOGD("-------------- SYN_REPORT ------------\n");
                    }
                    else if(type == EV_KEY) {               //only process EV_KEY,skip EV_REL,EV_ABS,EV_MSC which may introduct errors
                        ALOGW("input: keytype=%d %d,type=%x,code=%x, %s, keycode[kernel-app]=[%03d-%02d]\n", 
                                k, key_types[k], type, code, ev[i].value ? "down" : "up", ev[i].code, current_dot_key.key_code);
                        if(ev[i].value == 1) {                   //press down
                            //cmcc_interrupt_remind(100);
                            if (0 == current_dot_key.key_code && current_dot_key.key_code != code ) {
                                current_dot_key.key_code = code;
                                current_dot_key.ptime = ev_time;
                                current_dot_key.utime = 0;
                            } else if (current_dot_key.key_code == code ) { //repeated
                                current_dot_key.ptime =  ev_time;
                            } else {
                                int delta_time;
                                uint64_t tv_delta;
                                delta_time = ev_time - current_dot_key.ptime;
                                ALOGD("combain key delta time  %ld\n", tv_delta);
                                if (delta_time < 400) { //400ms combain key
                                    current_dot_key.key_code_b = code;
                                    current_dot_key.ptime = ev_time;
                                    current_dot_key.is_combain_key = 1; //combain key flag
                                }
                            }
                            //key_event_notify(&current_dot_key);
                        }
                        else {    //press up, signal wakeword thread to get a valid key
                            //if(0 != current_dot_key.key_code) {
                                uint64_t tv_now;
                                int delta_time, repeat_time;

                                tv_now = os_unix_time_ms();
                                repeat_time = ev_time - current_dot_key.utime;
                                current_dot_key.utime = ev_time;
                                delta_time = ev_time - current_dot_key.ptime;
                                ALOGD("Now: time %ld, delta_time=%d, repeat_time=%d\n",tv_now, delta_time, repeat_time);

                                memcpy(&key_read, &current_dot_key, sizeof(struct dot_key));
                                key_read.is_key_valid = 1;
                                if (key_read.is_combain_key && delta_time > KEY_LONG_PRESS_PREIOD) {
                                    key_read.press_type = K_COMB;
                                } else if(delta_time > KEY_LONG_PRESS_PREIOD && delta_time < KEY_VERY_LONG_PRESS_PERIOD) {
                                    key_read.press_type = K_LONG;
                                } else if(delta_time > KEY_VERY_LONG_PRESS_PERIOD) {
                                    key_read.press_type = K_VLONG;
                                } else if (repeat_time < (KEY_DOUBLE_CLICK_PERIOD / 1000)) {
                                    key_read.press_type = K_DQC;
                                }

                                ALOGD("key up, keycode1=%x,keycode2=%x,valid=%d,longtype=%d, combain=%d\n", key_read.key_code, key_read.key_code_b, key_read.is_key_valid, key_read.press_type, key_read.is_combain_key);
                                hasLongLongFunc = 0;
                            //}
                        }
                    } else if (type == EV_REL) {
                        ALOGD("input: keytype=%d,type=%x,code=%x,key %d\n", key_types[k], type, code, ev[i].value);
                        if (ev[i].value == 1)
                            key_read.key_code = HKEY_ROTARY_POS;
                        else
                            key_read.key_code = HKEY_ROTARY_NEG;
                        key_read.key_code_b = 0;
                        key_read.is_key_valid = 1;
                        key_read.press_type = K_SHORT;
                        key_read.is_combain_key = 0;
                    }
                }
                pthread_mutex_unlock(&ev_mutex);
            }
        }
    }
    for (int i = 0; i < key_fds_count; i++) {
         close(key_fds[i]);
    }
}

os_task_t *key_reader_task, *key_process_task;
int pbox_stop_KeyScanTask(void) {
    if (key_reader_task != NULL) {
        os_task_destroy(key_reader_task);
    }
    if (key_process_task != NULL) {
        os_task_destroy(key_process_task);
    }

    pthread_mutex_destroy(&ev_mutex);
    return 0;
}

int pbox_create_KeyScanTask(void)
{
    int err;

    pthread_mutex_init(&ev_mutex, NULL);
    err = (key_reader_task = os_task_create("event_read_thread_ex", &pbox_KeyEventScan, 0, NULL))? 0:-1;
    if (err != 0) {
        ALOGE("cant creat thread pbox_KeyEventScan");
        return err;
    }

    err = (key_process_task = os_task_create("pbox_keysend", &pbox_KeyEvent_server, 0, NULL))? 0:-1;
    if (err != 0) {
        ALOGE("cant creat thread pbox_KeyEvent_server");
    }
    return err;
}
