#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "pbox_common.h"
#include "pbox_socket.h"
#include "pbox_rockit.h"
#include "pbox_multi_display.h"
#include "pbox_app.h"

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

    switch (sampleFreq) {
        case 16000:
        case 24000:
        case 32000:
        case 44100:
        case 48000:{
            //do nothing..
        } break;

        default: {
           sampleFreq = 44100;
        } break;
    }

    switch (channel) {
        case 0: {
            channel = 2;
        } break;

        default: {
        } break;
    }
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

void pbox_app_rockit_set_player_loop(bool loop) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SETPLAYERLOOPING,
    };

    msg.loop = loop;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_player_seek(uint32_t mPosition) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SETPLAYERSEEKTO,
    };

    msg.mPosition = mPosition;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_player_volume(uint32_t volume) {
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

void pbox_app_rockit_set_player_seperate(bool enable , uint32_t hlevel, uint32_t mlevel, uint32_t rlevel) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SETPLAYER_SEPERATE,
    };

    msg.vocalSeperate.enable = enable;
    msg.vocalSeperate.u32HumanLevel = hlevel;
    msg.vocalSeperate.u32OtherLevel = mlevel;
    msg.vocalSeperate.u32ReservLevel = rlevel;
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

void pbox_app_rockit_set_recoder_volume(uint32_t volume) {
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

void pbox_app_rockit_set_recoder_3A(bool echo3A_On) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_RECORDER_3A,
    };

    msg.echo3A_On = echo3A_On;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

int maintask_rcokit_data_recv(pbox_rockit_msg_t *msg)
{
    static uint32_t music_duration = 0;
    static uint32_t music_position = 0;
    assert(msg);
    switch (msg->msgId) {
        case PBOX_ROCKIT_ENERGY_EVT: {
            energy_info_t energy_data = msg->energy_data;
            int size = energy_data.size;
            /*
            for(int i = 0; i< energy_data.size; i++) {
                printf("%s freq[%05d]HZ energyData[%05d]db\n",
                                __func__, energy_data.energykeep[i].freq,
                                energy_data.energykeep[i].energy);
            }*/
            pbox_multi_displayEnergyInfo(energy_data, DISP_All);

        } break;
        case PBOX_ROCKIT_MUSIC_POSITION_EVT: {
            printf("duration: %d", music_duration);
            music_position = msg->mPosition;
            if ((music_duration != 0) && (music_position !=0)) {
                pbox_multi_displayTrackPosition(music_position, music_duration, DISP_All);
            }
        } break;
        case PBOX_ROCKIT_MUSIC_DURATION_EVT: {
            music_duration = msg->duration;
            printf("duration: %d", music_duration);
            if ((music_duration != 0) && (music_position !=0)) {
                pbox_multi_displayTrackPosition(music_position, music_duration, DISP_All);
            }
        } break;
        case PBOX_ROCKIT_MUSIC_VOLUME_EVT: {
            uint32_t volume = msg->volume;
            printf("volume: %d", volume);
            pboxUIdata->mVolumeLevel = volume;
            pbox_multi_displayMainVolumeLevel(volume, DISP_All);
        } break;
        case PBOX_ROCKIT_PLAY_COMPLETED_EVT: {
            music_position = 0;
        } break;
        case PBOX_ROCKIT_PLAY_ERROR_EVT: {
            music_position = 0;
        } break;

        case PBOX_ROCKIT_AWAKEN_EVT: {
            struct _wake_up mWakeUp = msg->wake_up;
            uint32_t mWakeCmd = mWakeUp.wakeCmd;

            printf("%s WakeCmd:%d\n",__func__, mWakeUp);
            switch (mWakeCmd) {
                case KARAOKE_WAKE_UP_CMD_RECIEVE: {
                    printf("wakeup command receive\n");
                    pboxUIdata->play_status_prev= pboxUIdata->play_status;
                    pbox_app_music_pause(DISP_All);
                } break;
                case KARAOKE_WAKE_UP_CMD_RECIEVE_BUT_NO_TASK: {
                        printf("wakeup command receive but no task\n");
                        if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) {
                            pbox_app_music_resume(DISP_All);
                        }
                    } break;

                case KARAOKE_WAKE_UP_CMD_VOLUME_UP: {
                    uint32_t *const volume = &pboxUIdata->mVolumeLevel;

                    if (*volume <= 5)
                        *volume += 5;
                    else if (*volume <= 10)
                        *volume += 15;
                    else if (*volume <= 75)
                        *volume += 25;
                    printf("%s volume up:%d\n", __func__, *volume);

                    pbox_app_music_set_volume(*volume, DISP_All);
                    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) {
                        pbox_app_music_resume(DISP_All);
                    }
                    } break;

                case KARAOKE_WAKE_UP_CMD_VOLUME_DOWN: {
                    uint32_t *const volume = &pboxUIdata->mVolumeLevel;

                    printf("%s volume down:%d\n", __func__, *volume);

                    if (true) {
                        if (*volume >= 50)
                            *volume -= 25;
                        else if (*volume >= 25) 
                            *volume -= 15; 
                        else if (*volume >= 5)
                            *volume -= 5;

                        pbox_app_music_set_volume(*volume, DISP_All);
                    }
                    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) {
                        pbox_app_music_resume(DISP_All);
                    }
                } break;

                case KARAOKE_WAKE_UP_CMD_PAUSE_PLARER: {
                    pbox_app_music_pause(DISP_All);
                } break;

                case KARAOKE_WAKE_UP_CMD_START_PLAYER: {
                    pbox_app_music_start(DISP_All);
                } break;

                case KARAOKE_WAKE_UP_CMD_STOP_PLARER: {
                    pbox_app_music_stop(DISP_All);
                } break;

                case KARAOKE_WAKE_UP_CMD_PREV: {
                    pbox_app_music_album_next(false, DISP_All);
                    if(pboxUIdata->play_status != PLAYING) {
                        pbox_app_music_resume(DISP_All);
                    }
                } break;

                case KARAOKE_WAKE_UP_CMD_NEXT: {
                    pbox_app_music_album_next(true, DISP_All);
                    if(pboxUIdata->play_status != PLAYING) {
                        pbox_app_music_resume(DISP_All);
                    }
                } break;

                case KARAOKE_WAKE_UP_CMD_ORIGINAL_SINGER_OPEN: {
                    pbox_app_music_original_singer_open(true, DISP_All);
                    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) {
                        pbox_app_music_resume(DISP_All);
                    }
                } break;

                case KARAOKE_WAKE_UP_CMD_ORIGINAL_SINGER_CLOSE: {
                    pbox_app_music_original_singer_open(false, DISP_All);
                    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) {
                        pbox_app_music_resume(DISP_All);
                    }
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
    //printf("%s sock recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

    if (msg->type != PBOX_EVT)
        return;

    maintask_rcokit_data_recv(msg);
    return;
}
