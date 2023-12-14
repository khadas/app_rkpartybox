#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/select.h>
#include <sys/time.h>
#include "pbox_common.h"
#include "pbox_rockit.h"
//#include "pbox_socket_server.h"
//#include "rk_btsink.h"
#include "rk_comm_karaoke.h"

typedef enum
{
    IDLE = 0,
    PLAYING, 
    _PAUSE,
    _STOP,
    PLAY_NUM
} play_status_t;

static play_status_t play_status = IDLE;
static play_status_t play_status_prev = IDLE;
int32_t mHumanLevel=15, mMusicLevel=100, mGuitarLevel = 100;
int32_t mVolumeLevel=50, mMicVolumeLevel=50;
bool mEchoReductionEnable = true;
bool mVocalSeperateEnable = false;

int maintask_rockit_fd_process(int fd) 
{
    char buff[sizeof(pbox_rockit_msg_t)] = {0};

    int ret = recvfrom(fd, buff, sizeof(buff), 0, NULL, NULL);
    if (ret <= 0)
        return ret;

    pbox_rockit_msg_t *msg = (pbox_rockit_msg_t *)buff;
    printf("%s sock recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

    if (msg->type != PBOX_EVT)
        return -1;

    switch (msg->msgId) {
        case PBOX_ROCKIT_ENERGY_EVT: {
            struct energy_info energy_data = msg->energy_data;
            int size = energy_data.size;
            for(int i = 0; i< energy_data.size; i++) {
    		    printf("freq[%05d]HZ energyData[%05d]db\n",
                                energy_data.energykeep[i].freq,
                                energy_data.energykeep[i].energy);
            }
            //send to ui
        } break;
        case PBOX_ROCKIT_MUSIC_POSITION_EVT: {
            //send to ui
        } break;
        case PBOX_ROCKIT_MUSIC_DURATION_EVT: {
            RK_S64 duration = msg->duration;
            printf("duration: %d", duration);
            //send to ui
        } break;
        case PBOX_ROCKIT_MUSIC_VOLUME_EVT: {
            RK_U32 volume = msg->volume;
            printf("volume: %d", volume);
            //send to ui
        } break;
        case PBOX_ROCKIT_PLAY_COMPLETED_EVT: {
            //send to ui
        } break;
        case PBOX_ROCKIT_PLAY_ERROR_EVT: {
            //send to ui
        } break;

        case PBOX_ROCKIT_AWAKEN_EVT: {
            struct _wake_up mWakeUp = msg->wake_up;
            RK_S32 mWakeCmd = mWakeUp.wakeCmd;

            printf("%s WakeCmd:%d\n",__func__, mWakeUp);
            switch (mWakeCmd) {
                case KARAOKE_WAKE_UP_CMD_RECIEVE: {
                    printf("wakeup command receive\n");
                    play_status_prev= play_status;
                    if (play_status == PLAYING) {
                            //rockit_pause_cmd();
                            //bt pause
                            //_lv_demo_music_pause(); //ui
                            // led ui update
                    }
                } break;
                case KARAOKE_WAKE_UP_CMD_RECIEVE_BUT_NO_TASK: {
                        printf("wakeup command receive but no task\n");
                        if ((play_status == _PAUSE) && (play_status_prev == PLAYING)) {
                            //_lv_demo_music_resume();
                        }
                    } break;

                case KARAOKE_WAKE_UP_CMD_VOLUME_UP: {
                    RK_U32 volume = mWakeUp.volume;
                    printf("volume up:%d\n", volume);

                    if (true) {
                        if (volume <= 5)
                        volume += 5;
                        else if (volume <= 10)
                        volume += 15;
                        else if (volume <= 75)
                        volume += 25;

                        //rockit volume up cmd
                        //ui update.
                    }
                    if ((play_status == _PAUSE) && (play_status_prev == PLAYING)) {
                        //_lv_demo_music_resume(); //rockit and ui update
                    }
                    } break;

                case KARAOKE_WAKE_UP_CMD_VOLUME_DOWN: {
                    RK_U32 volume = mWakeUp.volume;
                    printf("volume up:%d\n", volume);

                    if (true) {
                        if (volume >= 50)
                            volume -= 25;
                        else if (volume >= 25) 
                            volume -= 15; 
                        else if (volume >= 5)
                            volume -= 5;

                        //rockit volume down cmd
                        //ui update.
                    }
                    if ((play_status == _PAUSE) && (play_status_prev == PLAYING)) {
                        //_lv_demo_music_resume(); //rockit and ui update
                    }
                } break;

                case KARAOKE_WAKE_UP_CMD_PAUSE_PLARER: {
                    //_lv_demo_music_pause();
                } break;

                case KARAOKE_WAKE_UP_CMD_START_PLAYER: {
                    //_lv_demo_music_resume();
                } break;

                case KARAOKE_WAKE_UP_CMD_STOP_PLARER: {
                    /*if(isBtA2dpConnected()) {
                        bt_sink_send_cmd(RK_BT_STOP, NULL, 0);
                    }
                    lv_timer_pause(sec_counter_timer);
                    lv_obj_clear_state(play_obj, LV_STATE_CHECKED);
                    lv_obj_invalidate(play_obj);
                    rk_demo_music_stop(player_ctx);*/
                    play_status = _STOP;
                } break;

                case KARAOKE_WAKE_UP_CMD_PREV: {
                    uint32_t id;// = track_id;
                    printf("wakeup prev track command\n");
                    /*if(isBtA2dpConnected()) {
                        bt_sink_send_cmd(RK_BT_PREV, NULL, 0);
                        rk_demo_music_pause(player_ctx);
                        play_status = _PAUSE;
                    }
                    else {
                        id = track_id;
                        if(id == 0) {
                            id = track_num - 1;
                        }
                        else {
                            id--;
                        }
                        char *track_name = _lv_demo_music_get_title(id);
                        char track_uri[256];
                        sprintf(track_uri, MUSIC_PATH"%s", track_name);
                        printf("wakecmd play track [%s]\n", track_uri);
                        rk_demo_music_stop(player_ctx);
                        play_status = _STOP;
                    }
                    _lv_demo_music_play(id);*/
                } break;

                case KARAOKE_WAKE_UP_CMD_NEXT: {
                    uint32_t id;// = track_id;
                    printf("wakeup next track command\n");
                    /*if(isBtA2dpConnected()) {
                        bt_sink_send_cmd(RK_BT_NEXT, NULL, 0);
                        rk_demo_music_pause(player_ctx);
                        play_status = _PAUSE;
                    }
                    else {
                        id = track_id;
                        id++;
                        if(id >= track_num) id = 0;
                        char *track_name = _lv_demo_music_get_title(id);
                        char track_uri[256];
                        sprintf(track_uri, MUSIC_PATH"%s", track_name);
                        printf("wakecmd play track [%s]\n", track_uri);
                        rk_demo_music_stop(player_ctx);
                        play_status = _STOP;
                    }
                    _lv_demo_music_play(id);*/
                } break;

                case KARAOKE_WAKE_UP_CMD_ORIGINAL_SINGER_OPEN: {
                    mVocalSeperateEnable = false;
                    /*rk_demo_music_voice_seperate(mHumanLevel, mMusicLevel, mGuitarLevel);
                    lv_obj_add_state(accomp_slider, LV_STATE_DISABLED);
                    lv_obj_add_state(vocal_slider, LV_STATE_DISABLED);
                    lv_obj_add_state(guitar_slider, LV_STATE_DISABLED);
                    if((play_status == _PAUSE) && (play_status_prev == PLAYING))
                        _lv_demo_music_resume();
                        */
                } break;

                case KARAOKE_WAKE_UP_CMD_ORIGINAL_SINGER_CLOSE: {
                    mVocalSeperateEnable = true;
                    /*rk_demo_music_voice_seperate(mHumanLevel, mMusicLevel, mGuitarLevel);
                    lv_obj_clear_state(accomp_slider, LV_STATE_DISABLED);
                    lv_obj_clear_state(vocal_slider, LV_STATE_DISABLED);
                    lv_obj_clear_state(guitar_slider, LV_STATE_DISABLED);
                    if((play_status == _PAUSE) && (play_status_prev == PLAYING))
                        _lv_demo_music_resume();
                    */
                } break;
                default: break;
            } //end switch (mWakeCmd)
        } break;
        default: break;
    } //end switch (msg->msgId)

    return 0;
}


int maintask_lvgl_fd_process(int fd) {
    return 0;
}

int maintask_bt_fd_process(int fd) {
    return 0;
}

int maintask_keyscan_fd_process(int fd) {
    return 0;
}

int maintask_read_event(int source, int fd) {
    int result = 0;
    switch (source) {
        case PBOX_LVGL: {
            result = maintask_lvgl_fd_process(fd);
        } break;

        case PBOX_BT: {
            result = maintask_bt_fd_process(fd);
        } break;

        case PBOX_ROCKIT: {
            result = maintask_rockit_fd_process(fd);
        } break;

        case PBOX_KEYSCAN: {
            result = maintask_keyscan_fd_process(fd);
        } break;
    }

    return result;
}

void main(int argc, char **argv) {
    int max_fd;
    int pbox_fds[PBOX_NUM] = {-1, -1, -1, -1};
	pthread_setname_np(pthread_self(), "party_main");
	//signal(SIGINT, sigterm_handler);

    pbox_fds[PBOX_LVGL] = create_udp_socket(SOCKET_PATH_LVGL_CLINET);
    pbox_fds[PBOX_BT] = create_udp_socket(SOCKET_PATH_ROCKIT_CLINET);
    pbox_fds[PBOX_ROCKIT] = create_udp_socket(SOCKET_PATH_BTSINK_CLIENT);
    pbox_fds[PBOX_KEYSCAN] = create_udp_socket(SOCKET_PATH_BTSINK_CLIENT);
    //battery_fd, usb_fd;

    //pbox_create_lvglTask();
    pbox_create_rockitTask();
    //pbox_create_ledEffectTask();
    //pbox_create_KeyScanTask();
    //pbox_create_BtTask();

    fd_set read_fds;
    FD_ZERO(&read_fds);
    for (int i= 0, maxfd = pbox_fds[0]; i < sizeof(pbox_fds)/sizeof(int); i++) {
        FD_SET(pbox_fds[i], &read_fds);
        if (maxfd < pbox_fds[i])
            maxfd = pbox_fds[i];
    }

    while (true) {
        fd_set read_set = read_fds;
        int result = select(max_fd+1, &read_fds, NULL, NULL, NULL);
        if ((result == 0) || (result < 0 && (errno != EINTR))) {
            printf("select timeout");
            continue;
        }

        if(result < 0) {
            break;
        }

        for (int i = 0; i < sizeof(pbox_fds)/sizeof(int); i++) {
            if(FD_ISSET(pbox_fds[i], &read_fds) == 0)
                continue;
            maintask_read_event(i , pbox_fds[i]);
        }
    }
}
