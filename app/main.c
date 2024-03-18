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
#include <getopt.h>
#include "pbox_app.h"
#include "rc_partybox.h"
#include "rk_btsink.h"
#include "pbox_common.h"
#include "pbox_rockit.h"
#include "pbox_socket.h"
#include "pbox_socketpair.h"
#include "pbox_lvgl.h"
#include "pbox_btsink_app.h"
#include "pbox_rockit_app.h"
#include "pbox_lvgl_app.h"
#if ENABLE_SARAADC
#include "pbox_keyadcSara.h"
#include "pbox_keyadcSara_app.h"
#else
#include "pbox_keyscan_app.h"
#endif
#include "pbox_usb.h"
#include "pbox_usb_app.h"
#include "pbox_light_effect_app.h"
#include "pbox_soc_bt_app.h"
#include "pbox_store_app.h"
#include "slog.h"

void maintask_timer_fd_process(int timer_fd);

static int main_loop = 1;
#define PBOX_TIMER_INTERVAL 10

pbox_pipe_t pbox_pipe_fds[PBOX_SOCKPAIR_NUM];

int maintask_read_event(int source, int fd) {
    int result = 0;

    //ALOGD("%s source:%d fd:%d\n", __func__, source, fd);
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
    main_loop = 0;
}

static const char *log_level_str = "warn";
static const char *pbox_ini_path = "/data/rkpartybox.ini";
static void pbox_debug_init(const char *debugStr) {
    char buffer[MAX_APP_NAME_LENGTH + 1];
    char *envStr;
    uint32_t loglevel =0;

    loglevel = covert2debugLevel(debugStr);
    FILE *file = fopen("/oem/debug_conf", "r");
    if (file != NULL) {
        size_t bytesRead = fread(buffer, 1, MAX_APP_NAME_LENGTH, file);
        fclose(file);

        loglevel = MAX(loglevel, covert2debugLevel(buffer));
    }

    os_env_get_str("loglevel", &envStr, "warn");
    //ALOGW("%s buffer:%s level:%d\n", __func__, envStr, loglevel);
    set_pbox_log_level(MAX(loglevel, covert2debugLevel(envStr)));
}

static const char short_options[] = "c:l:";
static const struct option long_options[] = {{"config", required_argument, NULL, 'c'},
                                             {"loglevel", required_argument, NULL, 'l'},
                                             {"help", no_argument, NULL, 'h'},
                                             {0, 0, 0, 0}};

static void usage_tip(FILE *fp, int argc, char **argv) {
    fprintf(fp,
            "Usage: %s [options]\n"
            "Version %s\n"
            "Options:\n"
            "-c | --config      partybox ini file, default is "
            "/userdata/rkpartybox.ini, need to be writable\n"
            "-l | --loglevel   loglevel [error/warn/info/debug], default is debug\n"
            "-h | --help        for help \n\n"
            "\n",
            argv[0], "v1.0");
    pbox_version_print();
}

void pbox_get_opt(int argc, char *argv[]) {
	for (;;) {
		int idx;
		int c;
		c = getopt_long(argc, argv, short_options, long_options, &idx);
		if (-1 == c)
			break;
		switch (c) {
		case 0: /* getopt_long() flag */
			break;
		case 'c':
			pbox_ini_path = optarg;
			break;
		case 'l':
			log_level_str = optarg;
			break;
		case 'h':
			usage_tip(stdout, argc, argv);
			exit(EXIT_SUCCESS);
		default:
			usage_tip(stderr, argc, argv);
			exit(EXIT_FAILURE);
		}
	}
}

void main(int argc, char **argv) {
    int max_fd, i;
    int pbox_fds[PBOX_MAIN_NUM] = {0};
    pthread_setname_np(pthread_self(), "party_main");
    signal(SIGINT, sigterm_handler);
    pbox_version_print();
    pbox_get_opt(argc, argv);
    pbox_debug_init(log_level_str);

    pbox_app_ui_init(pbox_ini_path);
    pbox_app_ui_load();

#if !ENABLE_USE_SOCBT
    pbox_init_background();
#endif

    for (i = 0; i< PBOX_SOCKPAIR_NUM; i++) {
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100*1000;

        if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0 , pbox_pipe_fds[i].fd) == -1) {
            ALOGE("Couldn't create pbox_fds[%d]: %s", i, strerror(errno));
            goto pbox_main_exit;
        }
        ALOGD("main: pbox_pipe_fds[%d]={%d, %d}\n", i, pbox_pipe_fds[i].fd[0], pbox_pipe_fds[i].fd[1]);
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
#if ENABLE_SARAADC
    pbox_create_KeyadcSaraTask();
#else
    pbox_create_KeyScanTask();
    pbox_create_KeyProcessTask();
#endif
    pbox_create_usb_task();
    #if ENABLE_USE_SOCBT
    pbox_create_btsoc_task();
    #else
    pbox_create_bttask();
    #endif
#if ENABLE_RK_LED_EFFECT
    pbox_app_led_startup_effect();
#endif

    fd_set read_fds;
    FD_ZERO(&read_fds);
    for (i= 0, max_fd = pbox_fds[0]; i < ARRAYSIZE(pbox_fds); i++) {
        FD_SET(pbox_fds[i], &read_fds);
        if (max_fd < pbox_fds[i])
            max_fd = pbox_fds[i];
        ALOGD("pbox_fds[%i]=%d, maxfd=%d\n", i, pbox_fds[i], max_fd);
    }

    start_fd_timer(pbox_fds[PBOX_MAIN_FD_TIMER], 2, PBOX_TIMER_INTERVAL, true); //every 10ms a timer.
    while (main_loop) {
        int ret;
        fd_set read_set = read_fds;

        int result = select(max_fd+1, &read_set, NULL, NULL, NULL);
        if ((result == 0) || (result < 0 && (errno != EINTR))) {
            ALOGW("select timeout");
            continue;
        }

        if(result < 0) {
            break;
        }

        //ALOGD("%s result:%d\n", __func__, result);
        for (int i = 0; i < ARRAYSIZE(pbox_fds); i++) {
            if((ret = FD_ISSET(pbox_fds[i], &read_set)) == 0)
                continue;
            maintask_read_event(i , pbox_fds[i]);
        }
    }

pbox_main_exit:
    pbox_app_data_deinit();
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
            ALOGW("%s: Connection closed\n", __func__);
        } else if (errno != EINTR) {
            perror("recvfrom");
        }
        return;
    }

    msTimePassed += PBOX_TIMER_INTERVAL;
    //ALOGD("working time:%llu\n", msTimePassed);

    if (0 == msTimePassed%20) {
        //every 10ms send command to reflash lvgl ui.
        pbox_app_lcd_dispplayReflash();
    }

    if (0 == msTimePassed%100) {
        pbox_app_data_save();
    }

    if((0 == msTimePassed%50) && (pboxUIdata->play_status == PLAYING)) {
        //send commamd to get engery.
        pbox_app_rockit_get_player_energy(pboxData->inputDevice);
    }

    if ((0 == msTimePassed%1000) && (pboxUIdata->play_status == PLAYING)) {
        //every one second send command to refresh position
        #if !ENABLE_EXT_MCU_USB
        if(pboxData->inputDevice == SRC_USB)
            pbox_app_rockit_get_music_current_postion(SRC_USB);
        #endif
    }

    if((isPoweron == false) /*&& (0 == msTimePassed%100)*/) {
        isPoweron = true;
        pbox_app_usb_pollState();
        #if ENABLE_USE_SOCBT
        pbox_app_btsoc_init();
        #else
        pbox_app_music_mics_init(DISP_All);
        #endif
        pbox_app_music_init();
    }
}
