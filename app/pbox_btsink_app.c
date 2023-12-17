#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <RkBtBase.h>
#include <RkBtSink.h>
#include "pbox_common.h"
#include "rk_btsink.h"
#include "pbox_btsink_app.h"
#include "pbox_socket.h"
#include "pbox_rockit_app.h"
#include "pbox_lvgl_app.h"

typedef struct {
	char localAddr[6];
	char remoteAddr[6];
	btsink_state_t btState;
	btsink_ad2p_state_t a2dpState;
    int pcmSampeFreq;
    int pcmChannel;
	int volume; //[0-10], map to 0~100 or 0~128
	int discoverable;
} rk_btsink_t;

rk_btsink_t rkbtsinkData = {
    .pcmSampeFreq = 44100,
    .pcmChannel = 2,
};

int getBtDiscoverable (void) {
    return rkbtsinkData.discoverable;
}
btsink_state_t getBtSinkState(void) {
    return rkbtsinkData.btState;
}

void setBtSinkState(btsink_state_t state) {
    rkbtsinkData.btState = state;
}

bool isBtA2dpConnected(void)
{
	if(rkbtsinkData.btState==BT_CONNECTED && (rkbtsinkData.a2dpState >= A2DP_CONNECTED))
		return true;
	
	return false;
}

bool isBtA2dpStreaming(void)
{
	if(rkbtsinkData.btState==BT_CONNECTED && (rkbtsinkData.a2dpState == A2DP_STREAMING))
		return true;
	return false;
}

int bt_sink_send_cmd(rk_bt_opcode_t command, char *data, int len)
{
    pbox_bt_msg_t msg = {0};

    msg.type = RK_BT_CMD;
    msg.msgId = command;
    if(data != NULL)
        memcpy(&msg.btcmd.data[0], data, len);

    unix_socket_send_cmd(PBOX_CHILD_BT, &msg, sizeof(pbox_bt_msg_t));
}

void update_bt_karaoke_playing_status(bool playing)
{
    printf("%s :%d\n", __func__, playing);
    /*play_status = playing ? PLAYING:_PAUSE;

    if(play_obj == NULL)
        return;

    if(playing) {
        lv_obj_add_state(play_obj, LV_STATE_CHECKED);
        lv_obj_invalidate(play_obj);
    } else {
        lv_obj_clear_state(play_obj, LV_STATE_CHECKED);
        lv_obj_invalidate(play_obj);
    }*/
}

void update_music_track_info(char *title, char *artist) {
    printf("%s track:[%s]-[%s]\n", __func__, title, artist);

    pbox_app_lcd_displayTrackInfo(title, artist);
}


void update_music_positions(unsigned current, unsigned total) {
    static int  prev_total = 0;
    printf("%s position:[%d]-[%d](%d)\n", __func__, current, total, prev_total);
    pbox_app_lcd_displayTrackPosition(current, total);
    if(prev_total != total) {
        prev_total = total;
        total = total/1000;
        //todo lv_slider_set_range(slider_obj, 0, total);
        //lv_slider_set_left_value(slider_obj, 0, LV_ANIM_OFF);
        //lv_label_set_text_fmt(duration_obj, "%"LV_PRIu32":%02"LV_PRIu32, total/ 60, total % 60);
    }

    //todo time_act = current = current/1000;
    //lv_label_set_text_fmt(time_obj, "%"LV_PRIu32":%02"LV_PRIu32, current / 60, current % 60);
    //lv_slider_set_value(slider_obj, current, LV_ANIM_ON);
}

void bt_sink_data_recv(pbox_bt_msg_t *msg) {
    switch (msg-> msgId) {
        case BT_SINK_STATE: {
            printf("%s btState:[%d -> %d]\n", __func__, rkbtsinkData.btState, msg->btinfo.state);
            if(rkbtsinkData.btState == msg->btinfo.state) {
                break;
            }
            setBtSinkState(msg->btinfo.state);
            if(rkbtsinkData.btState == BT_INIT_ON || rkbtsinkData.btState == BT_DISCONNECT) {
                if(rkbtsinkData.btState == BT_DISCONNECT) {
                    pbox_app_rockit_stop_BTplayer();
                    bt_sink_send_cmd(RK_BT_PAIRABLE, NULL, 0);
                }
                if(rkbtsinkData.btState == BT_INIT_ON) {
                    bt_sink_send_cmd(RK_BT_START_BLUEALSA, NULL, 0);
                    bt_sink_send_cmd(RK_BT_PAIRABLE, NULL, 0);
                }
            }
            else if(rkbtsinkData.btState == BT_CONNECTED) {
                bt_sink_send_cmd(RK_BT_CONNECTABLE, NULL, 0);
                //rk_demo_music_pause();
                pbox_app_rockit_start_BTplayer(rkbtsinkData.pcmSampeFreq, rkbtsinkData.pcmChannel);
            }
            else if(rkbtsinkData.btState == BT_NONE) {
                printf("%s recv msg: btsink state: OFF\n", __func__);
            }
        } break;

        case BT_SINK_A2DP_STATE: {
            btsink_ad2p_state_t a2dpState = msg->btinfo.a2dpState;
            printf("%s recv msg: a2dpsink state: %d <- prev[%d]\n", __func__, a2dpState, rkbtsinkData.a2dpState);
            if(rkbtsinkData.a2dpState != a2dpState) {
                rkbtsinkData.a2dpState = a2dpState;
                if(rkbtsinkData.a2dpState == A2DP_STREAMING) {
                    update_bt_karaoke_playing_status(true);
                    //notity ui player or rockit that bt is playing
                } else {
                    update_bt_karaoke_playing_status(false);
                }
            }
        } break;

        case BT_SINK_MUSIC_FORMAT: {
            int freq = msg->btinfo.audioFormat.sampingFreq;
            int channel = msg->btinfo.audioFormat.channel;
            printf("%s recv msg: a2dpmusic format: freq:%d channel: %d\n", __func__, freq, channel);
            if ((freq != rkbtsinkData.pcmSampeFreq) || (channel != rkbtsinkData.pcmChannel)) {
                rkbtsinkData.pcmSampeFreq = freq;
                rkbtsinkData.pcmChannel = channel;
                if(rkbtsinkData.a2dpState == A2DP_STREAMING) {
                    pbox_app_rockit_start_BTplayer(rkbtsinkData.pcmSampeFreq, rkbtsinkData.pcmChannel);
                }
            }
            printf("%s update: rkbtsinkData: freq:%d channel: %d\n", __func__, rkbtsinkData.pcmSampeFreq, rkbtsinkData.pcmChannel);
        } break;

        case BT_SINK_MUSIC_TRACK: {
            char *title = msg->btinfo.track.title;
            char *artist = msg->btinfo.track.artist;
            printf("%s recv msg rack: %s %s\n", __func__, title, artist);
            update_music_track_info(title, artist);
        } break;

        case BT_SINK_MUSIC_POSITIONS: {
            unsigned int current = msg->btinfo.positions.current;
            unsigned int total = msg->btinfo.positions.total;
            update_music_positions(current, total);
        } break;

        case BT_SINK_ADPTER_INFO: {
            switch (msg->btinfo.adpter.adpter_id) {
                case BT_SINK_ADPTER_DISCOVERABLE:
                    rkbtsinkData.discoverable = msg->btinfo.adpter.discoverable;
                    if (!rkbtsinkData.discoverable && (rkbtsinkData.btState != BT_CONNECTED)) {
                        bt_sink_send_cmd(RK_BT_PAIRABLE, NULL, 0);
                    }
                    break;

                default:
                    break;
            }
        } break;

        default:
        printf("%s recv msg: msId: %02x not handled\n", __func__, msg->msgId);
        break;
    }
}

void maintask_bt_fd_process(int fd) {
    char buff[sizeof(pbox_bt_msg_t)] = {0};
    int ret = recvfrom(fd, buff, sizeof(buff), 0, NULL, NULL);
    if ((ret == 0) || (ret < 0 && (errno != EINTR))) {
        printf("%s ret:%d , error:%d\n", __func__, ret, errno);
        return;
    }
    pbox_bt_msg_t *msg = (pbox_bt_msg_t *)buff;
    printf("%s sock recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

    if (msg->type != (bt_msg_t)PBOX_EVT)
        return;

    bt_sink_data_recv(msg);
    
    return;
}

void *btsink_watcher(void *arg) {
	pthread_setname_np(pthread_self(), "pbox_btwatch");
    printf("%s thread: %lu\n", __func__, (unsigned long)pthread_self());

    printf("%s \n", __func__);
    bool btsinkWatcher_track = false;
    pbox_bt_opcode_t btCmd_prev = 0;
    while (1) {
        if((getBtSinkState() == BT_NONE) && (btCmd_prev == RK_BT_OFF)) {
            bt_sink_send_cmd(RK_BT_ON, NULL, 0);
            btCmd_prev= RK_BT_ON;
            goto next_round;
        } else if (getBtSinkState() == BT_NONE) {
            goto next_round;
        } else if (btsinkWatcher_track == false) {
            btsinkWatcher_track = true;
            goto next_round;
        }

        if(get_ps_pid_new("bluetoothd") < 0) {
            btCmd_prev = RK_BT_OFF;
            btsinkWatcher_track = false;
            bt_sink_send_cmd(RK_BT_OFF, NULL, 0);
            setBtSinkState(BT_TURNING_TRUNNING_OFF);
            goto next_round;
        }

        if(get_ps_pid_new("bluealsa") < 0) {
            bt_sink_send_cmd(RK_BT_START_BLUEALSA, NULL, 0);
            goto next_round;
        }

        if (get_ps_pid_new("bluealsa-aplay") < 0) {
            bt_sink_send_cmd(RK_BT_START_BLUEALSA_APLAY, NULL, 0);
            goto next_round;
        }
next_round:
        sleep(2);
    }
}
