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
        printf("%s error", __func__);
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
        printf("%s error", __func__);
        return;
    }
}