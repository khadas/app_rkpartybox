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
#include "pbox_socket.h"
#include "pbox_lvgl.h"
#include "pbox_btsink_app.h"
#include "pbox_rockit_app.h"
#include "pbox_lvgl_app.h"
#include "pbox_keyscan_app.h"
#include "pbox_usb_app.h"
#include "pbox_light_effect.h"

void maintask_timer_fd_process(int timer_fd);

static int quit = 0;
#define PBOX_TIMER_INTERVAL 10

int maintask_read_event(int source, int fd) {
    int result = 0;

    //printf("%s source:%d fd:%d\n", __func__, source, fd);
    switch (source) {
        #if ENABLE_LCD_DISPLAY
        case PBOX_MAIN_LVGL: {
            maintask_lvgl_fd_process(fd);
        } break;
        #endif
        case PBOX_MAIN_BT: {
            maintask_bt_fd_process(fd);
        } break;

        case PBOX_MAIN_ROCKIT: {
            maintask_rockit_fd_process(fd);
        } break;

        case PBOX_MAIN_KEYSCAN: {
            maintask_keyscan_fd_process(fd);
        } break;

        case PBOX_MAIN_USBDISK: {
            maintask_usb_fd_process(fd);
        } break;

        case PBOX_MAIN_FD_TIMER: {
            maintask_timer_fd_process(fd);
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
    int pbox_fds[PBOX_MAIN_NUM] = {-1, -1, -1, -1, -1};
	pthread_setname_np(pthread_self(), "party_main");
	signal(SIGINT, sigterm_handler);
#if ENABLE_LCD_DISPLAY
    pbox_fds[PBOX_MAIN_LVGL] = create_udp_socket(SOCKET_PATH_LVGL_CLINET);
#endif
    pbox_fds[PBOX_MAIN_BT] = create_udp_socket(SOCKET_PATH_BTSINK_CLIENT);
    pbox_fds[PBOX_MAIN_ROCKIT] = create_udp_socket(SOCKET_PATH_ROCKIT_CLINET);
    pbox_fds[PBOX_MAIN_KEYSCAN] = create_udp_socket(SOCKET_PATH_KEY_SCAN_CLINET);
    pbox_fds[PBOX_MAIN_USBDISK] = create_udp_socket(SOCKET_PATH_USB_CLIENT);
    pbox_fds[PBOX_MAIN_FD_TIMER] = create_fd_timer();
    //battery_fd;
#if ENABLE_LCD_DISPLAY
    pbox_create_lvglTask();
#endif
    pbox_create_rockitTask();
    pbox_create_lightEffectTask();
    pbox_create_KeyScanTask();
    pbox_create_KeyProcessTask();
    pbox_create_bttask();
    pbox_create_usb_task();

    pbox_light_effect_send_cmd(RK_ECHO_LED_OFF_EVT, NULL, NULL);
    pbox_light_effect_send_cmd(RK_ECHO_SYSTEM_BOOTING_EVT, NULL, NULL);

    fd_set read_fds;
    FD_ZERO(&read_fds);
    for (i= 0, max_fd = pbox_fds[0]; i < ARRAYSIZE(pbox_fds); i++) {
        FD_SET(pbox_fds[i], &read_fds);
        printf("pbox_fds[%i]=%d, maxfd=%d\n", i, pbox_fds[i], max_fd);
        if (max_fd < pbox_fds[i])
            max_fd = pbox_fds[i];
    }

    start_fd_timer(pbox_fds[PBOX_MAIN_FD_TIMER], 2, PBOX_TIMER_INTERVAL, true); //every 10ms a timer.
    while (!quit) {
        int ret;
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
        for (int i = 0; i < ARRAYSIZE(pbox_fds); i++) {
            if((ret = FD_ISSET(pbox_fds[i], &read_set)) == 0)
                continue;
            maintask_read_event(i , pbox_fds[i]);
        }
    }

    for(i =0; i< ARRAYSIZE(pbox_fds); i++) {
        close(pbox_fds[i]);
    }
}

static uint64_t msTimePassed = 0;
static bool isPoweron = false;
void maintask_timer_fd_process(int timer_fd) {
    uint64_t expirations;

    int ret = read(timer_fd, &expirations, sizeof(expirations));
    if (ret <= 0) {
        if (ret == 0) {
            printf("%s: Connection closed\n", __func__);
        } else if (errno != EINTR) {
            perror("recvfrom");
        }
        return;
    }

    msTimePassed += PBOX_TIMER_INTERVAL;
    //printf("working time:%llu\n", msTimePassed);

    if (0 == msTimePassed%10) {
        //every 10ms send command to reflash lvgl ui.
        pbox_app_lcd_dispplayReflash();
    }

    if(0 == msTimePassed%40) {
        //send commamd to get engery.
        pbox_app_rockit_get_player_energy();
    }

    if((isPoweron == false) && (0 == msTimePassed%100)) {
        isPoweron = true;
        pbox_app_usb_pollState();
    }
}
