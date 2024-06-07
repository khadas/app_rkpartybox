#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/select.h>
#include <sys/prctl.h>
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
#include "pbox_keyscan_app.h"
#include "pbox_keyscan.h"
#include "pbox_hotplug.h"
#include "pbox_hotplug_app.h"
#include "pbox_light_effect_app.h"
#include "pbox_light_effect.h"
#include "pbox_soc_bt_app.h"
#include "pbox_store_app.h"
#include "slog.h"
#include "rk_utils.h"
#include "pbox_soc_bt.h"
#include "os_task.h"

#define RKPARTYBOX_CMD_PATH "/tmp/rkpartybox_cmd"
#define RKPARTYBOX_CMD_SIZE 64

void maintask_timer_fd_process(int timer_fd);
void pbox_tasks_stop(void);
void pbox_pipes_close(int pipe_fd[], int size);
void pbox_main_user_cmd(void);

static int main_loop = 1;
int main_sch_quit = 0;
#if ENABLE_LCD_DISPLAY
#define PBOX_TIMER_INTERVAL 20
#else
#define PBOX_TIMER_INTERVAL 10
#endif

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
        #if ENABLE_EXT_BT_MCU
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

        case PBOX_MAIN_HOTPLUG: {
            maintask_hotplug_fd_process(fd);
        } break;

        case PBOX_MAIN_FD_TIMER: {
            maintask_timer_fd_process(fd);
        } break;
    }
    return 0;
}

static void pbox_main_check_quit(void) {
    if (main_sch_quit == 1) {
        main_sch_quit = 2;
        #if ENABLE_EXT_BT_MCU
        main_loop = 0;
        #else
        pbox_app_bt_sink_onoff(false, DISP_All);
        #endif
    } else if(main_sch_quit == 2) {
        if(getBtSinkState() == APP_BT_NONE) {
            main_loop = 0;
        }
    }

    return;
}

static void sigterm_handler(int sig)
{
    ALOGW("signal recv:%d\n", sig);
    main_sch_quit = 1;
}

static const char *log_level_str = "warn";
static const char *pbox_ini_path = "/data/rkpartybox.ini";
static void pbox_debug_init(const char *debugStr) {
    char buffer[MAX_APP_NAME_LENGTH + 1];
    const char *envStr;
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

static const char short_options[] = "c:l:v:";
static const struct option long_options[] = {{"config", required_argument, NULL, 'c'},
                                             {"loglevel", required_argument, NULL, 'l'},
                                             {"init-volume", required_argument, NULL, 'v'},
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
            "-v | --init-volume        init volume \n"
            "-h | --help        for help \n\n"
            "\n",
            argv[0], "v1.0");
    pbox_version_print();
}

void pbox_get_opt(int argc, char *argv[]) {
    const char *envStr;
    extern char **environ;
    char **env = environ;

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
        case 'v':
            os_env_set_str("init_vol", optarg);
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
    prctl(PR_SET_NAME, "party_main");
    signal(SIGINT, sigterm_handler);
    pbox_version_print();
    pbox_get_opt(argc, argv);
    pbox_debug_init(log_level_str);

    pbox_app_set_favor_source_order();
    pbox_app_ui_init(pbox_ini_path);
    pbox_app_ui_load();

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
    pbox_fds[PBOX_MAIN_HOTPLUG] = get_client_socketpair_fd(PBOX_SOCKPAIR_HOTPLUG);
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
    pbox_create_hotplug_dev_task();
    #if ENABLE_EXT_BT_MCU
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

    pbox_tasks_stop();
    os_task_deint_all();
    pbox_pipes_close(pbox_fds, ARRAYSIZE(pbox_fds));
pbox_main_exit:
    pbox_app_data_deinit();
    ALOGW("rkpartybox the last exiting code...bye...\n");
}

void pbox_pipes_close(int pipe_fd[], int size) {
    //pbox_pipe_t pbox_pipe_fds
    for(int i =0; i< size; i++) {
        if(i == PBOX_MAIN_FD_TIMER) {
            close(pipe_fd[i]);
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

void pbox_tasks_stop(void) {
#if ENABLE_EXT_BT_MCU
    pbox_stop_btsoc_task();
#else
    pbox_stop_bttask();
#endif
    pbox_stop_hotplug_dev_task();
    pbox_stop_KeyScanTask();
#if ENABLE_RK_LED_EFFECT
    pbox_stop_light_effect();
#endif
#if ENABLE_RK_ROCKIT
    pbox_stop_rockitTask();
#endif
#if ENABLE_LCD_DISPLAY
    pbox_stop_lvglTask();
#endif
}

static uint64_t msTimePassed = 0;
static bool isPoweron = false;
static bool isTunningOn = false;
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

    //40ms, 25frames..
    if (0 == msTimePassed%(PBOX_TIMER_INTERVAL*2)) {
        //every 10ms send command to reflash lvgl ui.
        pbox_app_lcd_dispplayReflash();
    }

    if (0 == msTimePassed%100) {
        pbox_app_data_save();
        pbox_main_user_cmd();
        pbox_main_check_quit();
    }

    if((0 == msTimePassed%(PBOX_TIMER_INTERVAL*5)) && (pboxUIdata->play_status == PLAYING)) {
        //send commamd to get engery.
        pbox_app_rockit_get_player_energy(pboxData->inputDevice);
    }

    if ((pboxUIdata->play_status == PLAYING)) {
        //every one second send command to refresh position
        uint32_t scenes = 0;
        if(pboxData->inputDevice == SRC_CHIP_USB && (0 == msTimePassed%1000))
            scenes |= BIT(ENV_POSITION);
        if(0 == msTimePassed%320)
            scenes |= BIT(ENV_GENDER);

        //pbox_app_rockit_get_music_current_postion(SRC_CHIP_USB);
        pbox_app_post_get_sence_value(pboxData->inputDevice, scenes);
    }

    if((isPoweron == false) /*&& (0 == msTimePassed%100)*/) {
        isPoweron = true;
        pbox_app_usb_pollState();
        #if ENABLE_EXT_BT_MCU
        pbox_app_btsoc_init();
        #else
        pbox_app_music_mics_init(DISP_All);
        #endif
        pbox_app_music_init();
    }

    if(0 == msTimePassed%(PBOX_TIMER_INTERVAL*5)) {
        #if !ENABLE_EXT_BT_MCU
        extern int pbox_app_scene_state_machine(void);
        pbox_app_scene_state_machine();
        #endif
    }

    if(isTunningOn == false && (0 == msTimePassed%5000)) {
        pbox_app_tunning_init(DISP_All);
        isTunningOn = true;
    }

    if(pboxData->volume_resume_time > 0) {
        pboxData->volume_resume_time -= PBOX_TIMER_INTERVAL;
        if(pboxData->volume_resume_time <= 0) {
            //pboxData->volume_resume_time = -1;
            pbox_app_music_set_music_volume(pboxUIdata->musicVolumeLevel, DISP_All);
            pbox_app_music_set_main_volume(pboxUIdata->mainVolumeLevel, DISP_All);
        }
    }
}

typedef struct {
    const char *cmd;
    void (*action)(char *data);
    const char *desc;
} pb_command_t;

void pbox_main_exit(char *data) {
    main_loop = 0;
    ALOGW("Main Exiting...\n");
}

static pb_command_t pb_command_table[] = {
    {"quit", pbox_main_exit, "Exit the application"},
};

static void show_help_cmd(void) {
    ALOGW("[Usage]: echo xxx > %s\n", RKPARTYBOX_CMD_PATH);
    for (unsigned int i = 0; i < sizeof(pb_command_table) / sizeof(pb_command_t); i++) {
        ALOGW("%s\t%s\n", pb_command_table[i].cmd, pb_command_table[i].desc);
    }
}

static void handle_command(const char *cmd, char *arg) {
    if (strcmp("help", cmd) == 0 || strcmp("h", cmd) == 0) {
        show_help_cmd();
        return;
    }

    for (unsigned int i = 0; i < sizeof(pb_command_table) / sizeof(pb_command_t); i++) {
        if (strcmp(pb_command_table[i].cmd, cmd) == 0) {
            if (pb_command_table[i].action != NULL) {
                pb_command_table[i].action(arg);
            }
            return;
        }
    }

    ALOGE("Unknown command: %s\n", cmd);
}

void pbox_main_user_cmd(void) {
    char buff[RKPARTYBOX_CMD_SIZE] = {0};
    if (access(RKPARTYBOX_CMD_PATH, F_OK) != 0) {
        return;
    }

    FILE *fp = fopen(RKPARTYBOX_CMD_PATH, "r");
    if (fp == NULL) {
        ALOGE("Failed to open command file: %s (errno: %d)\n", strerror(errno), errno);
        return;
    }

    ssize_t bytesRead = fread(buff, 1, sizeof(buff) - 1, fp);
    fclose(fp);
    remove(RKPARTYBOX_CMD_PATH);

    if (bytesRead < 2 || buff[bytesRead - 1] != '\n') {
        return;
    }

    buff[bytesRead - 1] = '\0';

    char *arg = strchr(buff, ' ');
    if (arg != NULL) {
        *arg = '\0';  // Split the command and argument
        arg++;
    }

    handle_command(buff, arg);
}
