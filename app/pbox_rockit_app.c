#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "pbox_common.h"
#include "pbox_socket.h"
#include "pbox_rockit.h"

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

int unix_socket_rockit_send(void *info, int length)
{
	return unix_socket_send_cmd(PBOX_CHILD_ROCKIT, info, length);
}

void pbox_app_rockit_create(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_CREATE,
    };
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_destroy(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_DESTROY,
    };
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_datasource(char *path, char *headers) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SETDATASOURCE,
        .dataSource = {0},
    };

    strncpy(msg.dataSource.track_uri, path, MAX_APP_NAME_LENGTH);
    strncpy(msg.dataSource.headers, headers, MAX_APP_NAME_LENGTH);
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_start_BTplayer(int sampleFreq, int channel) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_STARTBTPLAYER,
    };

    msg.audioFormat.sampingFreq = sampleFreq;
    msg.audioFormat.channel = channel;
    printf("zdm %s sampleFreq:%d, channel:%d \n", __func__, sampleFreq, channel);
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_stop_BTplayer(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_STOPBTPLAYER,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_start_player(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_STARTPLAYER,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_pause_player(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_PAUSEPLAYER,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_stop_player(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_STOPPLAYER,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_resume_player(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_RESUMEPLAYER,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_get_music_current_postion(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_GETPLAYERCURRENTPOSITION,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_get_player_duration(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_GETPLAYERDURATION,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_player_loop(RK_BOOL loop) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SETPLAYERLOOPING,
    };

    msg.loop = loop;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_player_seek(RK_S64 usecPosition) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SETPLAYERSEEKTO,
    };

    msg.usecPosition = usecPosition;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_player_volume(RK_U32 volume) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SETPLAYERVOLUME,
    };

    msg.volume = volume;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_get_player_volume(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_GETPLAYERVOLUME,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_player_seperate(bool enable , uint32_t hlevel, uint32_t mlevel, uint32_t glevel) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SETPLAYER_SEPERATE,
    };

    msg.vocal.enable = enable;
    msg.vocal.u32HumanLevel = hlevel;
    msg.vocal.u32OtherLevel = mlevel;
    msg.vocal.u32GuitarLevel = glevel;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_get_player_energy(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_GETPLAYERENERGYLEVEL,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_start_recorder(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_STARTRECORDER,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_stop_recorder(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_STOPRECORDER,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_recoder_volume(RK_U32 volume) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SETRECORDERVOLUME,
    };

    msg.volume = volume;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_get_recoder_volume(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_GETRECORDERVOLUME,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_recoder_revert(pbox_revertb_t reverbMode) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_RECORDER_REVERT,
    };

    msg.reverbMode = reverbMode;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_recoder_3A(RK_BOOL echo3A_On) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_RECORDER_3A,
    };

    msg.echo3A_On = echo3A_On;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

int maintask_rcokit_data_recv(pbox_rockit_msg_t *msg)
{
    assert(msg);
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

}

void maintask_rockit_fd_process(int fd) 
{
    char buff[sizeof(pbox_rockit_msg_t)] = {0};

    int ret = recvfrom(fd, buff, sizeof(buff), 0, NULL, NULL);
    if (ret <= 0)
        return;

    pbox_rockit_msg_t *msg = (pbox_rockit_msg_t *)buff;
    printf("%s sock recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

    if (msg->type != PBOX_EVT)
        return;

    maintask_rcokit_data_recv(msg);
    return;
}
