#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/select.h>
#include <sys/time.h>
#include "pbox_common.h"

int create_fd_timer (void) {
    return timerfd_create(CLOCK_REALTIME, 0);
}

void pause_fd_timer(int timer_fd) {
    struct itimerspec its;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    if (timerfd_settime(timer_fd, 0, &its, NULL) == -1) {
        ALOGE("%s error", __func__);
        return;
    }
}

void start_fd_timer(int timer_fd, int start, int interval, bool loop) {
    struct itimerspec its;
    its.it_value.tv_sec = start/1000;
    its.it_value.tv_nsec = (start%1000)*1000*1000;
    its.it_interval.tv_sec = interval/1000;
    its.it_interval.tv_nsec = (interval%1000)*1000*1000;;

    if (timerfd_settime(timer_fd, 0, &its, NULL) == -1) {
        ALOGE("%s error", __func__);
        return;
    }
}

uint64_t time_get_os_boot_ms(void) { 
    return time_get_os_boot_us() / 1000;
}

uint64_t time_get_os_boot_us(void) {
  struct timespec ts_now = {};
  clock_gettime(CLOCK_BOOTTIME, &ts_now);

  return ((uint64_t)ts_now.tv_sec * 1000000L) +
         ((uint64_t)ts_now.tv_nsec / 1000);
}

int findMax(int array[], int size) {
    if (size <= 0) {
        ALOGE("%s size=%d error", __func__, size);
        return 0;
    }

    int max = array[0];
    for (int i = 1; i < size; i++) {
        if (array[i] > max) {
            max = array[i];
        }
    }
    return max;
}

#define ACODEC_FILE_PATH "/sys/devices/platform/ff560000.acodec/rk3308-acodec-dev/dac_output"
static void set_background_setting(void *arg) {
    exec_command_system("echo 1 > "ACODEC_FILE_PATH);
    exec_command_system("sleep 1");
    exec_command_system("echo 11 > "ACODEC_FILE_PATH);
}

void pbox_init_background(void) {
    pthread_t vocal_cpuset;
    int ret = pthread_create(&vocal_cpuset, NULL, set_background_setting, NULL);
    if (ret < 0)
    {
        ALOGE("pbox_init_background fail\n");
    }
}