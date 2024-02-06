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
#include "pbox_app.h"
#include "rk_comm_karaoke.h"
#include "rk_btsink.h"
#include "pbox_common.h"
#include "pbox_rockit.h"
#include "pbox_socket.h"
#include "pbox_socketpair.h"
#include "pbox_lvgl.h"
#include "pbox_btsink_app.h"
#include "pbox_rockit_app.h"
#include "pbox_lvgl_app.h"
#include "pbox_keyscan_app.h"
#include "pbox_usb.h"
#include "pbox_usb_app.h"
#include "pbox_light_effect_app.h"

void maintask_timer_fd_process(int timer_fd);

static int quit = 0;
#define PBOX_TIMER_INTERVAL 10

pbox_pipe_t pbox_pipe_fds[PBOX_SOCKPAIR_NUM];

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
        #if ENABLE_USE_SOCBT
            maintask_btsoc_fd_process(fd);
        #else
            maintask_bt_fd_process(fd);
        #endif
        } break;

        #if ENABLE_RK_ROCKIT
        case PBOX_MAIN_ROCKIT: {
            maintask_rockit_fd_process(fd);
        } break;
        #endif

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
    int pbox_fds[PBOX_MAIN_NUM] = {0};
    pthread_setname_np(pthread_self(), "party_main");
    signal(SIGINT, sigterm_handler);
    pbox_version_print();

    for (i = 0; i< PBOX_SOCKPAIR_NUM; i++) {
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100*1000;

        if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0 , pbox_pipe_fds[i].fd) == -1) {
            printf("Couldn't create pbox_fds[%d]: %s", i, strerror(errno));
            goto pbox_main_exit;
        }
        printf("main: pbox_pipe_fds[%d]={%d, %d}\n", i, pbox_pipe_fds[i].fd[0], pbox_pipe_fds[i].fd[1]);
    }

#if ENABLE_LCD_DISPLAY
    pbox_fds[PBOX_MAIN_LVGL] = get_client_socketpair_fd(PBOX_SOCKPAIR_LVGL);
#endif
    pbox_fds[PBOX_MAIN_BT] = get_client_socketpair_fd(PBOX_SOCKPAIR_BT);
    #if ENABLE_RK_ROCKIT
    pbox_fds[PBOX_MAIN_ROCKIT] = get_client_socketpair_fd(PBOX_SOCKPAIR_ROCKIT);
    #endif
    pbox_fds[PBOX_MAIN_KEYSCAN] = get_client_socketpair_fd(PBOX_SOCKPAIR_KEYSCAN);
    pbox_fds[PBOX_MAIN_USBDISK] = get_client_socketpair_fd(PBOX_SOCKPAIR_USBDISK);
    pbox_fds[PBOX_MAIN_FD_TIMER] = create_fd_timer();
    //battery_fd;
#if ENABLE_LCD_DISPLAY
    pbox_create_lvglTask();
#endif
#if ENABLE_RK_ROCKIT
    pbox_create_rockitTask();
#endif
#if ENABLE_RK_LED_EFFECT
    pbox_create_lightEffectTask();
#endif
    pbox_create_KeyScanTask();
    pbox_create_KeyProcessTask();
    pbox_create_usb_task();
    #if ENABLE_USE_SOCBT
    pbox_create_btsoc_task();
    #else
    pbox_create_bttask();
    #endif
    pbox_app_led_startup_effect();

    fd_set read_fds;
    FD_ZERO(&read_fds);
    for (i= 0, max_fd = pbox_fds[0]; i < ARRAYSIZE(pbox_fds); i++) {
        FD_SET(pbox_fds[i], &read_fds);
        if (max_fd < pbox_fds[i])
            max_fd = pbox_fds[i];
        printf("pbox_fds[%i]=%d, maxfd=%d\n", i, pbox_fds[i], max_fd);
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

pbox_main_exit:
    for(i =0; i< ARRAYSIZE(pbox_fds); i++) {
        if(i == PBOX_MAIN_FD_TIMER) {
            close(pbox_fds[i]);
            continue;
        }

        if(pbox_pipe_fds[i].fd[0] !=0) {
            shutdown(pbox_pipe_fds[i].fd[0], SHUT_WR);
            shutdown(pbox_pipe_fds[i].fd[0], SHUT_RD);
            close(pbox_pipe_fds[i].fd[0]);
            pbox_pipe_fds[i].fd[0] = 0;
        }

        if(pbox_pipe_fds[i].fd[1] !=0) {
            shutdown(pbox_pipe_fds[i].fd[1], SHUT_WR);
            shutdown(pbox_pipe_fds[i].fd[1], SHUT_RD);
            close(pbox_pipe_fds[i].fd[1]);
            pbox_pipe_fds[i].fd[1] = 0;
        }
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

    if((0 == msTimePassed%50) && (pboxUIdata->play_status == PLAYING)) {
        //send commamd to get engery.
        pbox_app_rockit_get_player_energy();
    }

    if ((0 == msTimePassed%1000) && (pboxUIdata->play_status == PLAYING)) {
        //every one second send command to refresh position
        pbox_app_rockit_get_music_current_postion();
    }

    if((isPoweron == false) && (0 == msTimePassed%100)) {
        isPoweron = true;
        pbox_app_usb_pollState();
        pbox_app_music_set_volume(50, DISP_All);
        pbox_app_music_set_recoder_revert(PBOX_REVERT_KTV, DISP_All);
        pbox_app_music_set_echo_3a(true, DISP_All);
        pbox_app_music_set_mic_mute(false, DISP_All);
        pbox_app_btsoc_reply_poweron(true);
    }
}
