#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <RkBtBase.h>
#include <RkBtSink.h>
#include "pbox_common.h"
#include "rk_btsink.h"
#include "pbox_btsink_app.h"
#include "pbox_socket.h"
#include "pbox_socketpair.h"
#include "pbox_rockit_app.h"
#include "pbox_multi_echo.h"
#include "pbox_app.h"
#include "rk_utils.h"

int unix_socket_btsink_send(void *info, int length)
{
    return unix_socket_send_cmd(PBOX_CHILD_BT, info, length);
}

void pbox_btsink_a2dp_stop(void) {
    pbox_bt_msg_t msg = {
        .type = RK_BT_CMD,
        .msgId = RK_BT_STOP,
    };

    unix_socket_btsink_send(&msg, sizeof(pbox_bt_msg_t));
}

// true: play
// false: pause
void pbox_btsink_playPause(bool play) {
    pbox_bt_msg_t msg = {
        .type = RK_BT_CMD,
        .msgId = RK_BT_PLAY,
    };
    msg.msgId = play? RK_BT_PLAY:RK_BT_PAUSE;

    unix_socket_btsink_send(&msg, sizeof(pbox_bt_msg_t));
}

void pbox_btsink_music_next(bool next) {
    pbox_bt_msg_t msg = {
        .type = RK_BT_CMD,
        .msgId = RK_BT_NEXT,
    };
    msg.msgId = next? RK_BT_NEXT:RK_BT_PREV;

    unix_socket_btsink_send(&msg, sizeof(pbox_bt_msg_t));
}

void pbox_btsink_volume_set(int volume) {
    pbox_bt_msg_t msg = {
        .type = RK_BT_CMD,
        .msgId = RK_BT_ABS_VOL,
    };
    msg.media_volume = volume;

    unix_socket_btsink_send(&msg, sizeof(pbox_bt_msg_t));
}

void pbox_btsink_volume_up(bool up) {
    pbox_bt_msg_t msg = {
        .type = RK_BT_CMD,
        .msgId = RK_BT_VOL_UP,
    };
    msg.msgId = up? RK_BT_VOL_UP:RK_BT_VOL_DOWN;

    unix_socket_btsink_send(&msg, sizeof(pbox_bt_msg_t));
}

void pbox_btsink_pair_enable(bool on) {
    pbox_bt_msg_t msg = {
        .type = RK_BT_CMD,
        .msgId = RK_BT_PAIRABLE,
    };
    msg.msgId = on? RK_BT_PAIRABLE:RK_BT_CONNECTABLE;

    unix_socket_btsink_send(&msg, sizeof(pbox_bt_msg_t));
}

void pbox_btsink_local_update(void) {
    pbox_bt_msg_t msg = {
        .type = RK_BT_CMD,
        .msgId = RK_BT_LOCAL_UPDATE,
    };

    unix_socket_btsink_send(&msg, sizeof(pbox_bt_msg_t));
}

void pbox_btsink_set_vendor_state(bool enable) {
    pbox_bt_msg_t msg = {
        .type = RK_BT_CMD,
        .msgId = RK_BT_START_BLUETOOTH,
    };
    msg.btinfo.enable = enable;

    unix_socket_btsink_send(&msg, sizeof(pbox_bt_msg_t));
}

void pbox_btsink_onoff(bool on) {
    pbox_bt_msg_t msg = {
        .type = RK_BT_CMD,
        .msgId = RK_BT_ON,
    };
    msg.msgId = on? RK_BT_ON:RK_BT_OFF;

    unix_socket_btsink_send(&msg, sizeof(pbox_bt_msg_t));
}

void pbox_btsink_start_only_aplay(bool only) {
    pbox_bt_msg_t msg = {
        .type = RK_BT_CMD,
        .msgId = RK_BT_START_BLUEALSA,
    };
    msg.msgId = only? RK_BT_START_BLUEALSA_APLAY:RK_BT_START_BLUEALSA;

    unix_socket_btsink_send(&msg, sizeof(pbox_bt_msg_t));
}

void pbox_btsink_start_only_bluealsa(void) {
    pbox_bt_msg_t msg = {
        .type = RK_BT_CMD,
        .msgId = RK_BT_START_BLUEALSA_ONLY,
    };

    unix_socket_btsink_send(&msg, sizeof(pbox_bt_msg_t));
}

int getBtDiscoverable (void) {
    return pboxBtSinkdata->discoverable;
}
btsink_state_t getBtSinkState(void) {
    return pboxBtSinkdata->btState;
}

void setBtSinkState(btsink_state_t state) {
    pboxBtSinkdata->btState = state;
}

char *getBtRemoteName(void) {
    return pboxBtSinkdata->remote_name;
}

void setBtRemoteName(char *name) {
    if (!name)
        return;

    if(0 != strcmp(name, pboxBtSinkdata->remote_name))
        strcpy(pboxBtSinkdata->remote_name, name);
}

bool isBtConnected(void)
{
    if(getBtSinkState()== APP_BT_CONNECTED)
        return true;

    return false;
}

bool isBtA2dpConnected(void)
{
    if(pboxBtSinkdata->btState==APP_BT_CONNECTED && (pboxBtSinkdata->a2dpState >= A2DP_CONNECTED))
        return true;
    
    return false;
}

bool isBtA2dpStreaming(void)
{
    if(pboxBtSinkdata->btState==APP_BT_CONNECTED && (pboxBtSinkdata->a2dpState == A2DP_STREAMING))
        return true;
    return false;
}

void update_bt_karaoke_playing_status(bool playing)
{
    ALOGD("%s :%d\n", __func__, playing);
    if(playing) {
        pbox_app_rockit_set_player_volume(pboxData->inputDevice, MIN_MAIN_VOLUME);
        pbox_app_rockit_resume_player(pboxData->inputDevice);
        pbox_app_resume_volume_later(350);
    } else {
        pbox_app_rockit_pause_player(pboxData->inputDevice);
    }
    pboxUIdata->play_status = playing ? PLAYING:_PAUSE;
    pbox_app_echo_playingStatus(playing, DISP_All);
}

void update_music_track_info(char *title, char *artist) {
    ALOGD("%s track:[%s]-[%s]\n", __func__, title, artist);
    pbox_app_echo_tack_info(title, artist, DISP_All);
}

void update_music_positions(uint32_t current, uint32_t total) {
    static uint32_t  prev_total = 0;
    ALOGD("%s position:[%d]-[%d](%d)\n", __func__, current, total, prev_total);
    pbox_app_echo_track_position(false, current, total, DISP_All);

    if(prev_total != total) {
        prev_total = total;
        pbox_app_restart_passive_player(SRC_CHIP_BT, true, DISP_All);
    }
}

void update_bt_music_volume(int volumeLevel ,display_t policy)
{
    float volumeLevelMapp = 0;

    if (volumeLevel > 127)
        volumeLevel = 127;
    if (volumeLevel < 0)
        volumeLevel = 0;

    volumeLevelMapp = volumeLevel * 100 / 127;

    ALOGD("%s bt volume :%d (0-127)mapping to %f (0-100)\n", __func__, volumeLevel, volumeLevelMapp);
    //covert to real db volume.
    volumeLevelMapp = PERCENT2TARGET(volumeLevelMapp, MIN_MAIN_VOLUME, MAX_MAIN_VOLUME);
    volumeLevelMapp = volumeLevelMapp> MAX_MAIN_VOLUME?MAX_MAIN_VOLUME:volumeLevelMapp;
    volumeLevelMapp = volumeLevelMapp< MIN_MAIN_VOLUME?MIN_MAIN_VOLUME:volumeLevelMapp;

    if(volumeLevelMapp != pboxUIdata->musicVolumeLevel)
    pbox_app_music_set_music_volume((float)volumeLevelMapp, policy);

    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) 
        pbox_app_music_resume(policy);
}

void bt_sink_data_recv(pbox_bt_msg_t *msg) {
    switch (msg-> msgId) {
        case BT_SINK_STATE: {
            ALOGD("%s btState:[%d -> %d]\n", __func__, pboxBtSinkdata->btState, msg->btinfo.state);
            if(pboxBtSinkdata->btState == msg->btinfo.state) {
                break;
            }
            setBtSinkState(msg->btinfo.state);
            ALOGD("%s pboxBtSinkdata->btState:[%d]\n", __func__, pboxBtSinkdata->btState);
            switch (pboxBtSinkdata->btState) {
                case APP_BT_INIT_ON: {
                    pbox_app_restart_btsink(false, DISP_All);
                    pbox_app_bt_pair_enable(true, DISP_All);
                    pbox_app_bt_local_update(DISP_All);
                } break;
                case APP_BT_DISCONNECT: {
                    pbox_app_bt_pair_enable(true, DISP_All);
                    if(is_input_source_selected(SRC_CHIP_BT, AUTO)) {
                        pbox_app_autoswitch_next_input_source(SRC_CHIP_BT, DISP_All);
                    }
                } break;
                case APP_BT_CONNECTED: {
                    setBtRemoteName(msg->btinfo.remote_name);
                    pbox_app_bt_pair_enable(false, DISP_All);

                    if(is_dest_source_auto_switchable(SRC_CHIP_BT))
                        pbox_app_switch_to_input_source(SRC_CHIP_BT, DISP_All);
                    if(is_input_source_selected(SRC_CHIP_BT, ANY)) {
                        pbox_app_restart_passive_player(SRC_CHIP_BT, true, DISP_All);
                    }
                } break;
                case APP_BT_NONE: {
                    ALOGD("%s recv msg: btsink state: OFF\n", __func__);
                } break;
            }

            if(is_input_source_selected(SRC_CHIP_BT, ANY))
                pbox_app_echo_bt_state(pboxBtSinkdata->btState, DISP_All);
        } break;

        case BT_SINK_NAME: {
            ALOGD("%s remote name: %s\n", __func__, msg->btinfo.remote_name);
            setBtRemoteName(msg->btinfo.remote_name);
        } break;
        case BT_SINK_A2DP_STATE: {
            btsink_ad2p_state_t a2dpState = msg->btinfo.a2dpState;
            ALOGD("%s recv msg: a2dpsink state: %d -> [%d]\n", __func__, pboxBtSinkdata->a2dpState, a2dpState);
            if(pboxBtSinkdata->a2dpState != a2dpState) {
                pboxBtSinkdata->a2dpState = a2dpState;
                if(!is_input_source_selected(SRC_CHIP_BT, ANY))
                    break;

                if(pboxBtSinkdata->a2dpState == A2DP_STREAMING) {
                    update_bt_karaoke_playing_status(true);
                } else {
                    update_bt_karaoke_playing_status(false);
                    //if (getBtSinkState() == APP_BT_DISCONNECT)
                    //    pbox_app_music_stop(DISP_All);
                }
            }
        } break;

        case BT_SINK_MUSIC_FORMAT: {
            int freq = msg->btinfo.audioFormat.sampingFreq;
            int channel = msg->btinfo.audioFormat.channel;
            ALOGD("%s recv msg: a2dpmusic format: freq:%d channel: %d\n", __func__, freq, channel);
            if ((freq != pboxBtSinkdata->pcmSampeFreq) || (channel != pboxBtSinkdata->pcmChannel)) {
                pboxBtSinkdata->pcmSampeFreq = freq;
                pboxBtSinkdata->pcmChannel = channel;
                if(pboxBtSinkdata->a2dpState == A2DP_STREAMING) {
                    pbox_app_restart_passive_player(SRC_CHIP_BT, false, DISP_All);
                }
            }
            ALOGD("%s update: pbox_data.btsink: freq:%d channel: %d\n", __func__, pboxBtSinkdata->pcmSampeFreq, pboxBtSinkdata->pcmChannel);
        } break;

        case BT_SINK_MUSIC_TRACK: {
            char *title = msg->btinfo.track.title;
            char *artist = msg->btinfo.track.artist;
            ALOGD("%s recv msg rack: %s %s\n", __func__, title, artist);
            if(is_input_source_selected(SRC_CHIP_BT, ANY))
                update_music_track_info(title, artist);
        } break;

        case BT_SINK_MUSIC_POSITIONS: {
            uint32_t current = msg->btinfo.positions.current;
            uint32_t total = msg->btinfo.positions.total;

            if(is_input_source_selected(SRC_CHIP_BT, ANY))
                update_music_positions(current, total);
        } break;

        case BT_SINK_ADPTER_INFO: {
            switch (msg->btinfo.adpter.adpter_id) {
                case BT_SINK_ADPTER_DISCOVERABLE:
                    pboxBtSinkdata->discoverable = msg->btinfo.adpter.discoverable;
                    if (!pboxBtSinkdata->discoverable && (pboxBtSinkdata->btState != APP_BT_CONNECTED)) {
                        pbox_app_bt_pair_enable(true, DISP_All);
                    }
                    break;

                default:
                    break;
            }
        } break;

        case RK_BT_ABS_VOL: {
            //when seperate function is enable, don't set bt volume to musicVolume.
            if ((!pboxUIdata->vocalSplit) && is_input_source_selected(SRC_CHIP_BT, ANY))
                update_bt_music_volume(msg->media_volume, DISP_All|DISP_FS);
        } break;

        case BT_SINK_VENDOR_EVT: {
            pbox_app_set_vendor_state(msg->btinfo.enable, DISP_All);
        }

        default:
        ALOGD("%s recv msg: msId: %02x not handled\n", __func__, msg->msgId);
        break;
    }
}

void maintask_bt_fd_process(int fd) {
    char buff[sizeof(pbox_bt_msg_t)] = {0};
    int ret = recv(fd, buff, sizeof(buff), 0);
    if ((ret == 0) || (ret < 0 && (errno != EINTR))) {
        ALOGD("%s ret:%d , error:%d\n", __func__, ret, errno);
        return;
    }
    pbox_bt_msg_t *msg = (pbox_bt_msg_t *)buff;
    ALOGD("%s sock recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

    if (msg->type != (bt_msg_t)PBOX_EVT)
        return;

    bt_sink_data_recv(msg);

    return;
}

void *btsink_watcher(void *arg) {
    ALOGD("%s thread: %lu\n", __func__, (unsigned long)pthread_self());

    ALOGD("%s \n", __func__);
    bool btsinkWatcher_track = false;
    pbox_bt_opcode_t btCmd_prev = 0;
    while (1) {
        if((getBtSinkState() == APP_BT_NONE) && (btCmd_prev == RK_BT_OFF)) {
            pbox_app_bt_sink_onoff(true, DISP_All);
            btCmd_prev= RK_BT_ON;
            goto next_round;
        } else if (getBtSinkState() == APP_BT_NONE) {
            goto next_round;
        } else if ((!pboxBtSinkdata->discoverable) && \
                    ((getBtSinkState() == APP_BT_INIT_ON) || (getBtSinkState() == APP_BT_DISCONNECT)))
        {
            pbox_btsink_pair_enable(true);
        } else if (btsinkWatcher_track == false) {
            btsinkWatcher_track = true;
            goto next_round;
        }

        if(!get_ps_pid_new("bluetoothd")) {
            btCmd_prev = RK_BT_OFF;
            btsinkWatcher_track = false;
            pbox_app_bt_sink_onoff(true, DISP_All);
            setBtSinkState(APP_BT_TURNING_TRUNNING_OFF);
            goto next_round;
        }

        if(!get_ps_pid_new("bluealsa")) {
            pbox_app_restart_btsink(false, DISP_All);
            goto next_round;
        }

        if (!(get_ps_pid_new("bluealsa-aplay"))) {
            pbox_app_restart_btsink(true, DISP_All);
            goto next_round;
        }

next_round:
        sleep(2);
    }
}
