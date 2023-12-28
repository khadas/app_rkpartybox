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
#include "pbox_socketpair.h"
#include "pbox_rockit_app.h"
#include "pbox_multi_display.h"
#include "pbox_app.h"

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
    strcpy(pboxBtSinkdata->remote_name, name);
}

bool isBtA2dpConnected(void)
{
	if(pboxBtSinkdata->btState==BT_CONNECTED && (pboxBtSinkdata->a2dpState >= A2DP_CONNECTED))
		return true;
	
	return false;
}

bool isBtA2dpStreaming(void)
{
	if(pboxBtSinkdata->btState==BT_CONNECTED && (pboxBtSinkdata->a2dpState == A2DP_STREAMING))
		return true;
	return false;
}

void update_bt_karaoke_playing_status(bool playing)
{
    printf("%s :%d\n", __func__, playing);
    pboxUIdata->play_status = playing ? PLAYING:_PAUSE;

    pbox_multi_displayIsPlaying(playing, DISP_All);
}

void update_music_track_info(char *title, char *artist) {
    printf("%s track:[%s]-[%s]\n", __func__, title, artist);

    pbox_multi_displayTrackInfo(title, artist, DISP_All);
}


void update_music_positions(uint32_t current, uint32_t total) {
    static uint32_t  prev_total = 0;
    printf("%s position:[%d]-[%d](%d)\n", __func__, current, total, prev_total);
    pbox_multi_displayTrackPosition(false, current, total, DISP_All);
    if(prev_total != total) {
        prev_total = total;
    }
}

void pbox_app_music_bt_volume_set(int mVolumeLevel ,display_t policy)
{
	int mVolumeLevelMapp = 0;

	if (mVolumeLevel > 127)
		mVolumeLevel = 127;
	if (mVolumeLevel < 0)
		mVolumeLevel = 0;

	mVolumeLevelMapp = mVolumeLevel * 100 / 127;

	printf("%s bt volume :%d (0-127)mapping to %d (0-100)\n", __func__, mVolumeLevel, mVolumeLevelMapp);

	pbox_app_music_set_volume(mVolumeLevelMapp, policy);

	if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) 
		pbox_app_music_resume(policy);
}

void bt_sink_data_recv(pbox_bt_msg_t *msg) {
    switch (msg-> msgId) {
        case BT_SINK_STATE: {
            printf("%s btState:[%d -> %d]\n", __func__, pboxBtSinkdata->btState, msg->btinfo.state);
            if(pboxBtSinkdata->btState == msg->btinfo.state) {
                break;
            }
            setBtSinkState(msg->btinfo.state);
            printf("%s pboxBtSinkdata->btState:[%d]\n", __func__, pboxBtSinkdata->btState);
            if(pboxBtSinkdata->btState == BT_INIT_ON || pboxBtSinkdata->btState == BT_DISCONNECT) {
                if(pboxBtSinkdata->btState == BT_DISCONNECT) {
                    pbox_app_rockit_stop_BTplayer();
                    pbox_app_music_stop(DISP_All);
                    pbox_btsink_pair_enable(true);
                }
                if(pboxBtSinkdata->btState == BT_INIT_ON) {
                    pbox_btsink_start_only_aplay(false);
                    pbox_btsink_pair_enable(true);;
                }
            }
            else if(pboxBtSinkdata->btState == BT_CONNECTED) {
                pbox_btsink_pair_enable(false);
                pbox_app_rockit_pause_player();
                pbox_app_rockit_start_BTplayer(pboxBtSinkdata->pcmSampeFreq, pboxBtSinkdata->pcmChannel);
            }
            else if(pboxBtSinkdata->btState == BT_NONE) {
                printf("%s recv msg: btsink state: OFF\n", __func__);
            }
            pbox_multi_displaybtState(pboxBtSinkdata->btState, DISP_All);
        } break;

        case BT_SINK_NAME: {
            printf("%s remote name: %s\n", __func__, msg->btinfo.remote_name);
            setBtRemoteName(msg->btinfo.remote_name);
        } break;
        case BT_SINK_A2DP_STATE: {
            btsink_ad2p_state_t a2dpState = msg->btinfo.a2dpState;
            printf("%s recv msg: a2dpsink state: %d -> [%d]\n", __func__, pboxBtSinkdata->a2dpState, a2dpState);
            if(pboxBtSinkdata->a2dpState != a2dpState) {
                pboxBtSinkdata->a2dpState = a2dpState;
                if(pboxBtSinkdata->a2dpState == A2DP_STREAMING) {
                    update_bt_karaoke_playing_status(true);
                } else {
                    update_bt_karaoke_playing_status(false);
                    if (getBtSinkState() == BT_DISCONNECT)
                        pbox_app_music_stop(DISP_All);
                }
            }
        } break;

        case BT_SINK_MUSIC_FORMAT: {
            int freq = msg->btinfo.audioFormat.sampingFreq;
            int channel = msg->btinfo.audioFormat.channel;
            printf("%s recv msg: a2dpmusic format: freq:%d channel: %d\n", __func__, freq, channel);
            if ((freq != pboxBtSinkdata->pcmSampeFreq) || (channel != pboxBtSinkdata->pcmChannel)) {
                pboxBtSinkdata->pcmSampeFreq = freq;
                pboxBtSinkdata->pcmChannel = channel;
                if(pboxBtSinkdata->a2dpState == A2DP_STREAMING) {
                    pbox_app_rockit_start_BTplayer(pboxBtSinkdata->pcmSampeFreq, pboxBtSinkdata->pcmChannel);
                }
            }
            printf("%s update: pbox_data.btsink: freq:%d channel: %d\n", __func__, pboxBtSinkdata->pcmSampeFreq, pboxBtSinkdata->pcmChannel);
        } break;

        case BT_SINK_MUSIC_TRACK: {
            char *title = msg->btinfo.track.title;
            char *artist = msg->btinfo.track.artist;
            printf("%s recv msg rack: %s %s\n", __func__, title, artist);
            update_music_track_info(title, artist);
        } break;

        case BT_SINK_MUSIC_POSITIONS: {
            uint32_t current = msg->btinfo.positions.current;
            uint32_t total = msg->btinfo.positions.total;
            update_music_positions(current, total);
        } break;

        case BT_SINK_ADPTER_INFO: {
            switch (msg->btinfo.adpter.adpter_id) {
                case BT_SINK_ADPTER_DISCOVERABLE:
                    pboxBtSinkdata->discoverable = msg->btinfo.adpter.discoverable;
                    if (!pboxBtSinkdata->discoverable && (pboxBtSinkdata->btState != BT_CONNECTED)) {
                        pbox_btsink_pair_enable(true);
                    }
                    break;

                default:
                    break;
            }
        } break;

        case RK_BT_ABS_VOL: {
		//when seperate function is enable, don't set bt volume to mainVolume.
		if (!pboxUIdata->mVocalSeperateEnable)
			pbox_app_music_bt_volume_set(msg->media_volume, DISP_All);
        } break;

        default:
        printf("%s recv msg: msId: %02x not handled\n", __func__, msg->msgId);
        break;
    }
}

void maintask_bt_fd_process(int fd) {
    char buff[sizeof(pbox_bt_msg_t)] = {0};
#if ENABLE_UDP_CONNECTION_LESS
    int ret = recvfrom(fd, buff, sizeof(buff), 0, NULL, NULL);
#else
    int ret = recv(fd, buff, sizeof(buff), 0);
#endif
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
            pbox_btsink_onoff(true);
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
            pbox_btsink_onoff(true);
            setBtSinkState(BT_TURNING_TRUNNING_OFF);
            goto next_round;
        }

        if(get_ps_pid_new("bluealsa") < 0) {
            pbox_btsink_start_only_aplay(false);
            goto next_round;
        }

        if (get_ps_pid_new("bluealsa-aplay") < 0) {
            pbox_btsink_start_only_aplay(true);
            goto next_round;
        }
next_round:
        sleep(2);
    }
}
