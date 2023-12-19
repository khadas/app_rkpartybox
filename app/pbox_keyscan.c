/*
 *  Copyright (c) 2005-2017 Fuzhou Rockchip Electronics Co.Ltd
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
#define _GNU_SOURCE
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
#include <pthread.h>
#include <signal.h>
#include<fcntl.h>
#include<stdbool.h>
#include <malloc.h>
#include "pbox_keyscan.h"
#include "pbox_socket.h"
#include "pbox_common.h"
#include "pbox_keyscan_app.h"

#ifdef RK_VAD
#include "vad.h"
#endif

#define DEV_INPUT_EVENT     "/dev/input"
#define EVENT_DEV_NAME      "event"
#define BITS_PER_LONG       (sizeof(long) * 8)

extern struct dot_key support_keys[];
extern const size_t support_keys_size;
pthread_mutex_t ev_mutex;

char rk29_keypad[] = {"rk29-keypad"};
char gpio_keys[] = {"gpio-keys"};
char rk29_rotary[] = {"rotary"};
char adc_keys[] = {"adc-keys"};
char rk8xx_pwrkey[] = {"rk8xx_pwrkey"};
char aw9163_ts[] = {"aw9163_ts"};


struct dot_key key_read[MAX_KEY_BUFFERED];
struct dot_support_event_type support_event[] =
{
    {KEY_EVENT, rk29_keypad},
    {KEY_EVENT, gpio_keys},
    {KEY_EVENT, adc_keys},
    {KEY_EVENT, rk8xx_pwrkey},
    {KEY_EVENT, aw9163_ts},
    {ROTARY_EVENT, rk29_rotary},
};

int unix_socket_keyscan_notify_msg(void *info, int length)
{
    unix_socket_notify_msg(PBOX_MAIN_KEYSCAN, info, length);
}

int key_event_notify(struct dot_key *key)
{
    pbox_keyevent_msg_t msg = {0};
    msg.key_code = key->key_code;
    msg.key_code_b = key->key_code_b;
    msg.is_long_press = key->is_long_press;
    msg.is_key_valid = key->is_key_valid;
    msg.time.tv_sec = key->time.tv_sec;
    msg.time.tv_usec = key->time.tv_usec;

    printf("%s sock send: code: %d, press ? %d\n", __func__, msg.key_code, msg.is_key_valid);
    memset(key_read, 0, sizeof(key_read));
    unix_socket_keyscan_notify_msg(&msg, sizeof(pbox_keyevent_msg_t));
}


/**
 * Print   device information (no events). This information includes
 * version numbers, device name and all bits supported by this device.
 *
 * @param fd The file descriptor to the device.
 * @return 0 on success or 1 otherwise.
 */
int print_device_info(int fd)
{
    int version;
    unsigned short id[4];
    char name[256] = "Unknown";

    if(ioctl(fd, EVIOCGVERSION, &version))
    {
        perror("evtest: can't get version\n");
        return 1;
    }

    printf("Input driver version is %d.%d.%d\n",
               version >> 16, (version >> 8) & 0xff, version & 0xff);

    ioctl(fd, EVIOCGID, id);
    printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
               id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

    ioctl(fd, EVIOCGNAME(sizeof(name)), name);
    printf("Input device name: \"%s\"\n", name);

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
int is_event_device(const struct dirent *dir)
{
    return strncmp(EVENT_DEV_NAME, dir->d_name, 5) == 0;
}

/**
 * Scans all /dev/input/event*, open muli event devices
 * by specifying event type.
 * @param fds: the address that fds store
 * @return fd counts.
 .
 */
int find_multi_event_dev(int event_type, int *fds)
{
    struct dirent **namelist;
    int i, ndev;
    char fname[64];
    int fd = -1, ret = -1;
    char name[256] = "???";
    int count = 0;

    if(event_type < EVENT_START || event_type > EVENT_END)
    {
        fprintf(stderr, "Invalid event type:%d\n", event_type);
        return 0;
    }

    ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, versionsort);
    if(ndev <= 0)
    {
        return 0;
    }

    for(i = 0; i < ndev; i++)
    {
        int j = 0;
        int events_nums = sizeof(support_event) / sizeof(struct dot_support_event_type);
        snprintf(fname, sizeof(fname),
                 "%s/%s", DEV_INPUT_EVENT, namelist[i]->d_name);
        fd = open(fname, O_RDONLY);
        if(fd < 0)
        {
            continue;
        }
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);
        fprintf(stderr, "%s:	%s,i=%d\n", fname, name, i);
        ret = -1;
        for(j = 0; j < events_nums; j++)
        {
            if(support_event[j].event_type == event_type && strstr(name, support_event[j].name))
            {
                /* find according event device */
                printf("find event device:%s\n", namelist[i]->d_name);
                ret = fd;
                print_device_info(fd);
                break;
            }
        }
        if(ret < 0)
        {
            close(fd);
        }
        else
        {
            fds[count++] = fd;
        }
        free(namelist[i]);
    }

    if(count == 0)
    {
        printf("Can't find device by event_type[%d,%s]\n", event_type, support_event[event_type].name);
    }
    free(namelist);
    return count;
}

void *event_read_thread_ex(void * arg)
{
    int key_fds[10], max_fd;
    int rd, ret;
    unsigned int i, j, k;
    fd_set rdfs;
    struct input_event ev[64];
    struct dot_key current_dot_key;
    struct timeval sel_timeout_tv;
    int hasLongLongFunc = 0;
    int key_fds_count;
    prctl(PR_SET_NAME,"event_read_thread_ex");
    if(getuid() != 0)
    {
        fprintf(stderr, "Not running as root, no devices may be available.\n");
        return NULL;
    }

    key_fds_count = find_multi_event_dev(KEY_EVENT, key_fds);
    printf("--find_multi_event_dev count=%d\n",key_fds_count);
    printf ("vol up %d, down %d, play %d, mode:%d, mic %d\n", KEY_VOLUMEUP, KEY_VOLUMEDOWN, KEY_PLAYPAUSE, KEY_MODE, KEY_MICMUTE);
    if(key_fds_count <= 0)
    {
        printf("-------------- key event thread exit because event key fd is null ------------\n");
    }

    if(key_fds_count > 0 ) {
        max_fd = 0;
        for(i = 0 ; i < key_fds_count; i++) {
            if(max_fd < key_fds[i])
            {
                max_fd = key_fds[i];
            }
        }
    } else {
        fprintf(stderr, "didn't find any valid key fd and valid rotary fd \n");
        return NULL;
    }

    memset(key_read, 0, sizeof(key_read));
    memset(&current_dot_key, 0x00 ,sizeof(struct dot_key));
    while(1)
    {
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
        if(ret == 0)
        {
            pthread_mutex_lock(&ev_mutex);
            //if the key volume up or down is pressed more than 500 ms, then the key will be recognized as long press key
            //, and is_key_valid will be false until the key has been released .
            //the comsumer thread have to continuiously change the volume if the key have not been released more than 800ms.
            if((current_dot_key.key_code == KEY_VOLUMEDOWN || current_dot_key.key_code == KEY_VOLUMEUP)  && false
                && 0 == current_dot_key.key_code_b) //repeat func
            {
                struct timeval tv_vol_change, tv_vol_delta;
                int delta_vol_time;

                gettimeofday(&tv_vol_change, 0);
                tv_vol_delta.tv_sec = tv_vol_change.tv_sec - current_dot_key.time.tv_sec;
                tv_vol_delta.tv_usec = tv_vol_change.tv_usec - current_dot_key.time.tv_usec;
                delta_vol_time = tv_vol_delta.tv_sec * 1000 + tv_vol_delta.tv_usec / 1000;
                if(delta_vol_time >= 500)
                {
                    for(j = 0; j < MAX_KEY_BUFFERED; j++){
                        if(!key_read[j].is_key_valid && key_read[j].key_code == 0)
                        {
                            printf("add repeat-------------------------------index=%d\n\n",j);
                            memcpy(&key_read[j], &current_dot_key, sizeof(struct dot_key));
                            key_read[j].is_long_press = 1; //repeat
                            key_read[j].is_key_valid = 0;
                            current_dot_key.is_long_press = 1; //repeat
                            current_dot_key.is_key_valid = 0;
                            break;
                        }
                    }
                }
            } else {
                if(0 != current_dot_key.key_code)
                {
                    struct timeval tv_now, tv_delta;
                    int delta_time;

                    gettimeofday(&tv_now, 0);
                    printf("Now: time %ld.%06ld\n", tv_now.tv_sec, tv_now.tv_usec);
                    tv_delta.tv_sec = tv_now.tv_sec - current_dot_key.time.tv_sec;
                    tv_delta.tv_usec = tv_now.tv_usec - current_dot_key.time.tv_usec;
                    delta_time = tv_delta.tv_sec * 1000 + tv_delta.tv_usec / 1000;
                    //printf("Delta: time %ld.%06ld, delta_time=%d\n", tv_delta.tv_sec, tv_delta.tv_usec, delta_time);

                    if(current_dot_key.is_combain_key && delta_time > KEY_LONG_PRESS_PREIOD) {
                        printf("key[0x%x] [0x%x]  combain key\n", current_dot_key.key_code,  current_dot_key.key_code_b);
                        current_dot_key.is_long_press = 3;
                        current_dot_key.is_key_valid = 1;
                    }else if(delta_time > KEY_LONG_PRESS_PREIOD && delta_time < KEY_VERY_LONG_PRESS_PERIOD) {
                        printf("key[0x%x] is long long key????\n", current_dot_key.key_code);
                        for(j = 0; j < support_keys_size; j++)
                        {
                            if(support_keys[j].key_code == current_dot_key.key_code && 2 == support_keys[j].is_long_press){
                                printf("key[0x%x] has longlong key event\n", current_dot_key.key_code);
                                hasLongLongFunc = 1;
                                break;
                            }
                        }
                        if((current_dot_key.key_code == KEY_MICMUTE &&  delta_time > 5000)){
                            hasLongLongFunc = 0;
                         }else if((current_dot_key.key_code == KEY_MICMUTE &&  delta_time < 5000)){
                            hasLongLongFunc = 1;
                         }
                        if(!hasLongLongFunc){
                            printf("key[0x%x] long key\n", current_dot_key.key_code);
                            current_dot_key.is_long_press = 1;
                            current_dot_key.is_key_valid = 1;
                            hasLongLongFunc = 0;
                        }
                    } else if(delta_time > KEY_VERY_LONG_PRESS_PERIOD) {
                        printf("key[0x%x] long long key\n", current_dot_key.key_code);
                        current_dot_key.is_long_press = 2;
                        current_dot_key.is_key_valid = 1;
                        hasLongLongFunc = 0;
                    }

                    if(current_dot_key.is_key_valid) {
                        for(j = 0; j < MAX_KEY_BUFFERED; j++)
                        {
                            if(!key_read[j].is_key_valid && key_read[j].key_code == 0) { //key up fuc
                                memcpy(&key_read[j], &current_dot_key, sizeof(struct dot_key));
                                memset(&current_dot_key, 0x00 ,sizeof(struct dot_key));
                                if(key_read[j].key_code != 0)
                                key_event_notify(&key_read[j]);
                            }
                        }
                    }
                }
            }
            pthread_mutex_unlock(&ev_mutex);
        } else if(ret == -1) {
            perror("select error\n");
            continue;
        }

        k = 0;
        while(k < key_fds_count)
        {
            int key_fd = key_fds[k++];
            if(FD_ISSET(key_fd, &rdfs))
            {
                rd = read(key_fd, ev, sizeof(ev));

                if(rd < (int) sizeof(struct input_event))
                {
                    printf("[key]expected %d bytes, got %d, ignore the value\n", (int) sizeof(struct input_event), rd);
                    continue;
                }

                pthread_mutex_lock(&ev_mutex);
                for(i = 0; i < rd / sizeof(struct input_event); i++)
                {
                    int type, code;

                    type = ev[i].type;
                    code = ev[i].code;
                    printf("Event: time %ld.%06ld,\n", ev[i].time.tv_sec, ev[i].time.tv_usec);
                    #ifdef RK_VAD
                    clear_vad_count();//has key event,clear vad count.
                    #endif

                    if(type == EV_SYN)
                    {
                        printf("-------------- SYN_REPORT ------------\n");
                    }
                    else if(type == EV_KEY)//only process EV_KEY,skip EV_REL,EV_ABS,EV_MSC which may introduct errors
                    {
                        printf("input: type=%x,code=%x,key %s, current key event code=%x\n", type, code, ev[i].value ? "down" : "up", current_dot_key.key_code);
                        if(ev[i].value == 1)   //press down
                        {
                            //cmcc_interrupt_remind(100);
                            if(0 == current_dot_key.key_code && current_dot_key.key_code != code ) {
                                current_dot_key.key_code = code;
                                current_dot_key.time.tv_sec =  ev[i].time.tv_sec;
                                current_dot_key.time.tv_usec = ev[i].time.tv_usec;
                            } else if(current_dot_key.key_code == code ) { //repeated

                            } else {
                                int delta_time;
                                struct timeval  tv_delta;
                                tv_delta.tv_sec = ev[i].time.tv_sec - current_dot_key.time.tv_sec;
                                tv_delta.tv_usec = ev[i].time.tv_usec - current_dot_key.time.tv_usec;
                                delta_time = tv_delta.tv_sec * 1000 + tv_delta.tv_usec / 1000;
                                printf("combain key delta time  %ld.%06ld\n", tv_delta.tv_sec, tv_delta.tv_usec);
                                if(delta_time < 400) { //400ms combain key
                                    current_dot_key.key_code_b = code;
                                    current_dot_key.time.tv_sec = ev[i].time.tv_sec;
                                    current_dot_key.time.tv_usec = ev[i].time.tv_usec;
                                    current_dot_key.is_combain_key = 1; //combain key flag
                                }
                            }
                            key_event_notify(&current_dot_key);
                        }
                        else     //press up, signal wakeword thread to get a valid key
                        {
                            if(0 != current_dot_key.key_code)
                            {
                                struct timeval tv_now, tv_delta;
                                int delta_time;

                                gettimeofday(&tv_now, 0);
                                printf("Now: time %ld.%06ld\n", tv_now.tv_sec, tv_now.tv_usec);
                                tv_delta.tv_sec = ev[i].time.tv_sec - current_dot_key.time.tv_sec;
                                tv_delta.tv_usec = ev[i].time.tv_usec - current_dot_key.time.tv_usec;
                                delta_time = tv_delta.tv_sec * 1000 + tv_delta.tv_usec / 1000;
                                printf("Delta: time %ld.%06ld, delta_time=%d\n", tv_delta.tv_sec, tv_delta.tv_usec, delta_time);
                            
                                for(j = 0; j < MAX_KEY_BUFFERED; j++)
                                {
                                     if((current_dot_key.key_code == KEY_VOLUMEDOWN || current_dot_key.key_code == KEY_VOLUMEUP)  //repeat end
                                        && 0 == current_dot_key.key_code_b 
                                        && 1 == current_dot_key.is_long_press
                                        && !current_dot_key.is_key_valid 
                                        && false) {
                                        if( key_read[j].key_code == current_dot_key.key_code
                                            && !key_read[j].is_key_valid 
                                            && 0 ==  key_read[j].key_code_b
                                            && 1 == key_read[j].is_long_press)
                                        {
                                            printf("repeat end\n");
                                            //memset(&key_read[j], 0, sizeof(struct dot_key));
                                            break;
                                        } else {
                                            continue;
                                        }
                                    } else if(!key_read[j].is_key_valid && key_read[j].key_code == 0) { //key up fuc
                                        memcpy(&key_read[j], &current_dot_key, sizeof(struct dot_key));
                                        key_read[j].is_key_valid = 1;
                                        if(key_read[j].is_combain_key && delta_time > KEY_LONG_PRESS_PREIOD) {
                                            key_read[j].is_long_press = 3;
                                        }else if(delta_time > KEY_LONG_PRESS_PREIOD && delta_time < KEY_VERY_LONG_PRESS_PERIOD) {
                                            key_read[j].is_long_press = 1;
                                        } else  if(delta_time > KEY_VERY_LONG_PRESS_PERIOD) {
                                            key_read[j].is_long_press = 2;
                                        }
                                        printf("key up, keycode1=%x,keycode2=%x,valid=%d,longtype=%d, combain=%d\n", key_read[j].key_code, key_read[j].key_code_b, key_read[j].is_key_valid, key_read[j].is_long_press, key_read[j].is_combain_key);
                                        key_event_notify(&key_read[j]);
                                        break;
                                        //memset(&key_read[j], 0, sizeof(struct dot_key));
                                    }
                                    else if(j == MAX_KEY_BUFFERED - 1)
                                    {
                                        printf("Can't find valid buffer, you should increase the value of MAX_KEY_BUFFERED\n");
                                        break;
                                    }
                                }
                                hasLongLongFunc = 0;
                            }
                            
                            memset(&current_dot_key, 0x00 ,sizeof(struct dot_key));
                        }
                    }
                }
                pthread_mutex_unlock(&ev_mutex);
            }
        }
    }
}

int pbox_create_KeyScanTask(void)
{
    pthread_t evt_reader;
    int err;

    pthread_mutex_init(&ev_mutex, NULL);
    err = pthread_create(&evt_reader, NULL, &event_read_thread_ex, NULL);
    if (err != 0)
        printf("cant creat thread event_read_thread_ex");
    return err;
}
