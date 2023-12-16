#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <assert.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/select.h>
#include <sys/time.h>
#include "rk_comm_karaoke.h"
#include "rk_btsink.h"
#include "pbox_common.h"
#include "pbox_rockit.h"
//#include "pbox_socket.h"
#include "pbox_btsink_app.h"

static int quit = 0;

int maintask_read_event(int source, int fd) {
    int result = 0;

    //printf("%s source:%d fd:%d\n", __func__, source, fd);
    switch (source) {
        case PBOX_MAIN_LVGL: {
            maintask_lvgl_fd_process(fd);
        } break;

        case PBOX_MAIN_BT: {
            maintask_bt_fd_process(fd);
        } break;

        case PBOX_MAIN_ROCKIT: {
            maintask_rockit_fd_process(fd);
        } break;

        case PBOX_MAIN_KEYSCAN: {
            maintask_keyscan_fd_process(fd);
        } break;
    }
    return 0;
}

static void sigterm_handler(int sig)
{
    fprintf(stderr, "signal %d\n", sig);
    quit = 1;
}

void main(int argc, char **argv) {
    int max_fd, i;
    int pbox_fds[PBOX_MAIN_NUM] = {-1, -1, -1, -1};//lvgl canceled
	pthread_setname_np(pthread_self(), "party_main");
	signal(SIGINT, sigterm_handler);

    pbox_fds[PBOX_MAIN_LVGL] = create_udp_socket(SOCKET_PATH_LVGL_CLINET);
    pbox_fds[PBOX_MAIN_BT] = create_udp_socket(SOCKET_PATH_BTSINK_CLIENT);
    pbox_fds[PBOX_MAIN_ROCKIT] = create_udp_socket(SOCKET_PATH_ROCKIT_CLINET);
    pbox_fds[PBOX_MAIN_KEYSCAN] = create_udp_socket(SOCKET_PATH_KEY_SCAN_CLINET);
    //battery_fd, usb_fd;

    //pbox_create_lvglTask();
    pbox_create_rockitTask();
    //pbox_create_ledEffectTask();
    //pbox_create_KeyScanTask();
    pbox_create_bttask();

    fd_set read_fds;
    FD_ZERO(&read_fds);
    for (i= 0, max_fd = pbox_fds[0]; i < sizeof(pbox_fds)/sizeof(int); i++) {
        FD_SET(pbox_fds[i], &read_fds);
        printf("pbox_fds[%i]=%d, maxfd=%d\n", i, pbox_fds[i], max_fd);
        if (max_fd < pbox_fds[i])
            max_fd = pbox_fds[i];
    }

    while (!quit) {
        fd_set read_set = read_fds;
        int result = select(max_fd+1, &read_set, NULL, NULL, NULL);
        if ((result == 0) || (result < 0 && (errno != EINTR))) {
            printf("select timeout");
            continue;
        }

        if(result < 0) {
            break;
        }

        //printf("%s result:%d\n", __func__, result);

        for (int i = 0; i < sizeof(pbox_fds)/sizeof(int); i++) {
            if(FD_ISSET(pbox_fds[i], &read_fds) == 0)
                continue;
            maintask_read_event(i , pbox_fds[i]);
        }
    }
}
