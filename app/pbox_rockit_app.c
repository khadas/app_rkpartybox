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
#include "pbox_socketpair.h"
#include "pbox_rockit.h"
#include "pbox_multi_echo.h"
#include "pbox_app.h"

int unix_socket_rockit_send(void *info, int length)
{
    int ret;
    #if ENABLE_RK_ROCKIT
    ret = unix_socket_send_cmd(PBOX_CHILD_ROCKIT, info, length);
    #endif
    return ret;
}

void pbox_app_rockit_create(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_CREATE,
    };
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_destroy(input_source_t source) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_DESTROY,
        .source = source,
    };
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_start_local_player(char *path, char *headers) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_START_LOCAL_PLAYER,
        .dataSource = {0},
    };

    if(path)
        strncpy(msg.dataSource.track_uri, path, MAX_MUSIC_NAME_LENGTH);
    if(headers)
        strncpy(msg.dataSource.headers, headers, MAX_APP_NAME_LENGTH);
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_start_audiocard_player(input_source_t source, int sampleFreq, int channel, const char *cardName) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_START_AUDIOCARD_PLAYER,
        .source = source,
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
    strncpy(msg.audioFormat.cardName, cardName, sizeof(msg.audioFormat.cardName));
    ALOGD("%s src:%d, sampleFreq:%d, channel:%d, cardname:%s \n", __func__, source, sampleFreq, channel, cardName);
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_pause_player(input_source_t source) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_PAUSE_PLAYER,
        .source = source,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_stop_player(input_source_t source) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_STOP_PLAYER,
        .source = source,
    };
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_resume_player(input_source_t source) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_RESUME_PLAYER,
        .source = source,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_get_music_current_postion(input_source_t source) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_GET_PLAYERCURRENTPOSITION,
        .source = source,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_get_player_duration(input_source_t source) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_GET_PLAYERDURATION,
        .source = source,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_player_loop(input_source_t source, bool loop) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_PLAYERLOOPING,
        .source = source,
    };

    msg.loop = loop;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_player_seek(input_source_t source, uint32_t mPosition) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_PLAYERSEEKTO,
        .source = source,
    };

    msg.mPosition = mPosition;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_player_volume(input_source_t source, float volume) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_MAINVOLUME,
        .source = source,
    };

    if(volume == MIN_MAIN_VOLUME) {
        volume = MIN_MAIN_VOLUME_MUTE;
    }
    msg.volume = volume;
    ALOGD("%s msg.vol:%f volume:%f\n", __func__, msg.volume, volume);
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_get_player_volume(input_source_t source) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_GET_MAINVOLUME,
        .source = source,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_music_volume(input_source_t source, float volume) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_MUSICVOLUME,
        .source = source,
    };

    msg.volume = volume;
    ALOGD("%s music vol:%f\n", __func__, msg.volume);
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}


void pbox_app_rockit_play_notice_number(uint32_t number) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_NOTICE_NUMBER,
    };

    msg.number = number;
    ALOGD("%s :%d\n", __func__, msg.number);
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_get_music_volume(input_source_t source) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_GET_MUSICVOLUME,
        .source = source,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_player_seperate(input_source_t source, pbox_vocal_t vocal) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_PLAYER_SEPERATE,
        .source = source,
    };

    msg.vocalSeperate = vocal;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_stereo_mode(input_source_t source, stereo_mode_t stereo) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_STEREO_MODE,
        .source = source,
    };

    msg.stereo = stereo;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_outdoor_mode(input_source_t source, inout_door_t outdoor) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_OUTDOOR_MODE,
        .source = source,
    };

    msg.outdoor = outdoor;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_placement(input_source_t source, placement_t place) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_PLACEMENT_MODE,
        .source = source,
    };

    msg.place = place;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_eq_mode(input_source_t source, equalizer_t mode) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_EQ_MODE,
        .source = source,
    };

    msg.eqmode = mode;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_bassboost(input_source_t source, uint8_t mode) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_BASS_BOOST_MODE,
        .source = source,
    };

    msg.value = mode;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}
//micMux: bit0-bit7: mic0-mic7
//guitarMux: bit0-bit7: guitar0-guitar7
void pbox_app_rockit_get_player_energy(uint8_t destMux, input_source_t source, uint8_t micMux, uint8_t guitarMux) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_GET_ENERGYLEVEL,
        .source = source,
    };

    msg.energy.energyDest = destMux;
    msg.energy.micMux = micMux;
    msg.energy.guitarMux = guitarMux;

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_start_recorder(input_source_t source, int sampleFreq, int channel, const char *cardName) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_START_RECORDER,
        .source = source,
    };

    if (0 == sampleFreq) {
        sampleFreq = 48000;
    }
    if (0 == channel) {
        channel = 2;
    }

    msg.audioFormat.sampingFreq = sampleFreq;
    msg.audioFormat.channel = channel;
    snprintf(msg.audioFormat.cardName, sizeof(msg.audioFormat.cardName), 
                "%s", hal_get_audio_card(SRC_CHIP_UAC));
    ALOGD("%s src:%d, sampleFreq:%d, channel:%d, cardname:%s\n", __func__, source, sampleFreq, channel, cardName);
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_stop_recorder(input_source_t source) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_STOP_RECORDER,
        .source = source,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_mic_data(uint8_t index, mic_set_kind_t kind, mic_state_t micState) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_MIC_STATE,
    };

    if(kind==MIC_SET_DEST_VOLUME) {
        if(micState.micVolume <= MIN_MIC_PHONE_VOLUME) {
            micState.micVolume = -100;
        }
    }

    msg.micdata.index = index;
    msg.micdata.kind = kind;
    msg.micdata.micState = micState;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_get_recoder_volume(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_GET_RECORDERVOLUME,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_tunning(bool enable) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_TUNNING_TOOL,
    };

    msg.enable = enable;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_reset_gender(input_source_t source) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_RESET_GENDER,
        .source = source,
    };
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_post_sence_detect(input_source_t source, size_t scenes) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_GET_SENCE,
        .source = source,
        .scene = scenes,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_start_inout_detect(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_START_INOUT_DETECT,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

//left, right
void pbox_app_rockit_start_doa_detect(int agent_role) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_START_DOA_DETECT,
        .agentRole = agent_role,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_stop_env_detect(void) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_STOP_ENV_DETECT,
    };

    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}
void pbox_app_rockit_set_uac_state(uac_role_t role, bool start) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_UAC_STATE,
        .source = SRC_CHIP_UAC,
    };
    msg.uac.uac_role = role;
    msg.uac.state = start;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_uac_freq(uac_role_t role, uint32_t freq) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_UAC_SAMPLE_RATE,
        .source = SRC_CHIP_UAC,
    };
    msg.uac.uac_role = role;
    msg.uac.sampleFreq = freq;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_uac_volume(uac_role_t role, float volume) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_UAC_VOLUME,
        .source = SRC_CHIP_UAC,
    };
    msg.uac.uac_role = role;
    msg.uac.volume = volume;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_mute(uac_role_t role, bool mute) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_UAC_MUTE,
        .source = SRC_CHIP_UAC,
    };
    msg.uac.uac_role = role;
    msg.uac.mute = mute;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

void pbox_app_rockit_set_ppm(uac_role_t role, int32_t ppm) {
    pbox_rockit_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_ROCKIT_SET_UAC_PPM,
        .source = SRC_CHIP_UAC,
    };
    msg.uac.uac_role = role;
    msg.uac.ppm = ppm;
    unix_socket_rockit_send(&msg, sizeof(pbox_rockit_msg_t));
}

char *get_scene_string(env_scene_t scene) {
    switch (scene) {
        case ENV_DOA: return CSTR(DOA);
        case ENV_REVERB: return CSTR(REVERB);
        case ENV_GENDER: return CSTR(GENDER);
        case ENV_POSITION: return CSTR(POSITION);
    }
    return NULL;
}

void pbox_app_handle_env_sences(struct _sense_res sense_res) {
    uint32_t scene = sense_res.scene;
    ALOGD("%s %s result:%d\n", __func__, get_scene_string(scene), sense_res.result);

    switch (scene) {
        case ENV_DOA: {
            pboxData->stereo_shake = sense_res.result;
        } break;
        case ENV_REVERB: {
            pboxData->inout_shake = sense_res.result;
        } break;
        case ENV_GENDER: {
            pboxData->gender_sence = sense_res.result;
            pbox_app_echo_gender_info(pboxData->gender_sence, DISP_All);
        } break;
    }
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
                ALOGD("%s freq[%05d]HZ energyData[%05d]db\n",
                                __func__, energy_data.energykeep[i].freq,
                                energy_data.energykeep[i].energy);
            }*/
            if((pboxData->dispMicEngery && (energy_data.dest == ENERGY_MIC && (energy_data.index == 0)))
                ||(pboxData->dispMicEngery==0) && (energy_data.dest == ENERGY_PLAYER)) {
                pbox_multi_echoEnergyInfo(energy_data, DISP_All);
            }
        } break;
        case PBOX_ROCKIT_MUSIC_POSITION_EVT: {
            //ALOGD("duration: %d", music_duration);
            music_position = msg->mPosition;
            if ((music_duration != 0) && (music_position !=0)) {
                pbox_multi_echoTrackPosition(false, music_position, music_duration, DISP_All);
            }
        } break;
        case PBOX_ROCKIT_MUSIC_DURATION_EVT: {
            music_duration = msg->duration;
            ALOGD("duration: %d\n", music_duration);
            if (music_duration != 0) {
                pbox_multi_echoTrackPosition(true, music_position, music_duration, DISP_All);
            }
        } break;
        case PBOX_ROCKIT_MUSIC_MAIN_VOLUME_EVT: {
            int32_t volume = msg->volume;
            ALOGD("volume: %d\n", volume);
            if(volume < MIN_MAIN_VOLUME)
                volume = MIN_MAIN_VOLUME;
            pboxUIdata->mainVolumeLevel = volume;
            pbox_multi_echoMainVolumeLevel(volume, DISP_All);
        } break;
        case PBOX_ROCKIT_MUSIC_CHANNEL_VOLUME_EVT: {
            int32_t volume = msg->volume;
            ALOGD("volume: %d\n", volume);
            pboxUIdata->musicVolumeLevel = volume;
            pbox_multi_echoMusicVolumeLevel(volume, DISP_All);
        } break;
        case PBOX_ROCKIT_PLAY_COMPLETED_EVT: {
            music_position = 0;
            pbox_app_music_album_next(true, DISP_All);
            if(pboxUIdata->play_status != PLAYING) {
                pbox_app_music_resume(DISP_All);
            }
        } break;
        case PBOX_ROCKIT_PLAY_ERROR_EVT: {
            music_position = 0;
        } break;

        case PBOX_ROCKIT_ENV_SENCE_EVT: {
            pbox_app_handle_env_sences(msg->sence_res);
        } break;

        case PBOX_ROCKIT_AWAKEN_EVT: {
            struct _wake_up mWakeUp = msg->wake_up;
            uint32_t mWakeCmd = mWakeUp.wakeCmd;

            ALOGD("%s WakeCmd:%d\n",__func__, mWakeUp);
            switch (mWakeCmd) {
                case RC_PB_WAKE_UP_CMD_RECIEVE: {
                    ALOGD("wakeup command receive\n");
                    pboxUIdata->play_status_prev= pboxUIdata->play_status;
                    pbox_app_music_pause(DISP_All);
                } break;
                case RC_PB_WAKE_UP_CMD_RECIEVE_BUT_NO_TASK: {
                        ALOGD("wakeup command receive but no task\n");
                        if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) {
                            pbox_app_music_resume(DISP_All);
                        }
                    } break;

                case RC_PB_WAKE_UP_CMD_VOLUME_UP: {
                    float *const volume = &pboxUIdata->musicVolumeLevel;

                    if (*volume <= 5)
                        *volume += 5;
                    else if (*volume <= 10)
                        *volume += 15;
                    else if (*volume <= 75)
                        *volume += 25;
                    ALOGD("%s volume up:%d\n", __func__, *volume);

                    pbox_app_music_set_music_volume(*volume, DISP_All);
                    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) {
                        pbox_app_music_resume(DISP_All);
                    }
                    } break;

                case RC_PB_WAKE_UP_CMD_VOLUME_DOWN: {
                    float *const volume = &pboxUIdata->musicVolumeLevel;

                    ALOGD("%s volume down:%d\n", __func__, *volume);

                    if (true) {
                        if (*volume >= 50)
                            *volume -= 25;
                        else if (*volume >= 25) 
                            *volume -= 15; 
                        else if (*volume >= 5)
                            *volume -= 5;

                        pbox_app_music_set_music_volume(*volume, DISP_All);
                    }
                    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) {
                        pbox_app_music_resume(DISP_All);
                    }
                } break;

                case RC_PB_WAKE_UP_CMD_PAUSE_PLARER: {
                    pbox_app_music_pause(DISP_All);
                } break;

                case RC_PB_WAKE_UP_CMD_START_PLAYER: {
                    pbox_app_music_resume(DISP_All);
                } break;

                case RC_PB_WAKE_UP_CMD_STOP_PLARER: {
                    pbox_app_music_stop(DISP_All);
                } break;

                case RC_PB_WAKE_UP_CMD_PREV: {
                    pbox_app_music_album_next(false, DISP_All);
                    if(pboxUIdata->play_status != PLAYING) {
                        pbox_app_music_resume(DISP_All);
                    }
                } break;

                case RC_PB_WAKE_UP_CMD_NEXT: {
                    pbox_app_music_album_next(true, DISP_All);
                    if(pboxUIdata->play_status != PLAYING) {
                        pbox_app_music_resume(DISP_All);
                    }
                } break;

                case RC_PB_WAKE_UP_CMD_ORIGINAL_SINGER_OPEN: {
                    pbox_app_music_original_singer_open(true, DISP_All);
                    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) {
                        pbox_app_music_resume(DISP_All);
                    }
                } break;

                case RC_PB_WAKE_UP_CMD_ORIGINAL_SINGER_CLOSE: {
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
    int ret = recv(fd, buff, sizeof(buff), 0);
    if (ret <= 0)
        return;

    pbox_rockit_msg_t *msg = (pbox_rockit_msg_t *)buff;
    //ALOGD("%s sock recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

    if (msg->type != PBOX_EVT)
        return;

    maintask_rcokit_data_recv(msg);
    return;
}
