#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "pbox_app.h"
#include "pbox_multi_echo.h"
#include "pbox_btsink_app.h"
#include "pbox_rockit_app.h"
#include "pbox_hotplug_app.h"
#include "hal_partybox.h"

pbox_data_t pbox_data = {
    .btsink = {
        .pcmSampeFreq = DEFAULT_SAMPLE_FREQ,
        .pcmChannel = 2,
    },
    .ui = {
        .mainVolumeLevel = MAX_MAIN_VOLUME,
        .musicVolumeLevel = DEFAULT_VOLUME,
        .accomLevel = 100,
        #if ENABLE_LCD_DISPLAY
        .humanLevel = 15,
        #else
        .humanLevel = 5,
        #endif
        .reservLevel = 5,
        .vocalSplit = false,
        .vocaltype = VOCAL_HUMAN,
        .play_status = IDLE,
        .play_status_prev = IDLE,
        .autoSource = true,
    },
    .track= {
        .track_num = 0,
        .track_id = 0,
    },
    .uac = {
        .state = false,
        .freq = 48000,
    },
#if ENABLE_EXT_BT_MCU
    .inputDevice = SRC_EXT_BT,
#else
    .inputDevice = SRC_CHIP_USB,
#endif
    .volume_resume_time = -1,
    .stereo_shake = -1,
    .inout_shake = -1,
    .gender_sence = -1,
};

pbox_data_t *const pboxData  = &(pbox_data);
struct _pbox_btsink *const pboxBtSinkdata  = &(pbox_data.btsink);
struct _pbox_ui *const pboxUIdata  = &(pbox_data.ui);
struct _pbox_track *const pboxTrackdata  = &(pbox_data.track);
usb_disk_info_t *const pboxUsbdata  = &(pbox_data.usbDisk);
struct _pbox_uac *const pboxUacdata = &(pbox_data.uac);

favor_input_order_t input_order_config[SRC_NUM];
void pbox_app_init_favor_source_order(void) {
    pboxData->avail_srcs = 0;
    //const input_source_t input_priority[SRC_NUM] = FAVOR_SRC_ORDER;
    for (int i = 0; i < SRC_NUM; i++) {
        input_order_config[i].source = hal_get_favor_source_order(i);
        input_order_config[i].enable = hal_get_supported_sources()&(1 << hal_get_favor_source_order(i));

        if(input_order_config[i].enable) {
            ALOGW("%s, source:%d, %s\n", __func__, input_order_config[i].source, getInputSourceString(input_order_config[i].source));
            pboxData->avail_srcs++;
        }
    }
};

bool is_input_source_configed(input_source_t source) {
    for (int i = 0; i < SRC_NUM; i++) {
        if(!input_order_config[i].enable) {
            continue;
        }

        if(input_order_config[i].source == source) {
            return true;
        }
    }
    return false;
}

void pbox_app_init_mic_mux_matrix(void) {
    uint32_t matrix = hal_get_supported_mic_matrix();
    uint32_t mux;
    for (int i = 0; i < hal_get_mic_guitar_num(); i++) {
        mux = matrix&BIT(i);
        pboxUIdata->micData[i].micMux = mux? MIC_IN:MIC_GT;
        ALOGW("%s matrix:0x%02x, mic[%d]=%s\n", __func__, matrix, i, mux? CSTR(MIC_IN):CSTR(MIC_GT));
    }
}

void pbox_app_echo_track_position(bool durationOnly, uint32_t current, uint32_t duration, display_t policy) {
    //nothing to notify rockit
    pbox_multi_echoTrackPosition(durationOnly, current, duration, policy);
}

void pbox_app_echo_gender_info(gender_t gender, display_t policy) {
    //nothing to notify rockit
    pbox_multi_echoGender(gender, policy);
}

void pbox_app_reset_gender_info(display_t policy) {
    //nothing to notify rockit
    pbox_multi_echoGender(GENDER_TBD, policy);
    pbox_app_rockit_reset_gender(pboxData->inputDevice);
}

void pbox_app_echo_tack_info(char *title, char *artist, display_t policy) {
    //nothing to notify rockit
    pbox_multi_echoTrackInfo(title, artist, policy);
}

void pbox_app_echo_bt_state(btsink_state_t state, display_t policy) {
    //nothing to notify rockit
    pbox_multi_echobtState(state, policy);
}

void pbox_app_resume_volume_later(int32_t msdelay) {
    pboxData->volume_resume_time = msdelay;
}

void pbox_app_echo_playingStatus(bool play, display_t policy) {
    //nothing to notify rockit
    pbox_multi_echoIsPlaying(play, policy);
}

void pbox_app_restart_passive_player(input_source_t source, bool restart, display_t policy) {
    ALOGW("%s, source:%d, restart:%d\n", __func__, source, restart);
    if(!is_input_source_selected(source, ANY)) {
        return;
    }

    pbox_app_rockit_set_player_volume(source, MIN_MAIN_VOLUME);
    if(restart) {
        pbox_app_rockit_stop_player(source);
    }

    switch(source) {
        case SRC_CHIP_BT: {
            pbox_app_rockit_start_audiocard_player(source, pboxBtSinkdata->pcmSampeFreq, pboxBtSinkdata->pcmChannel, hal_get_audio_card(SRC_CHIP_BT));
        } break;

        case SRC_EXT_BT: {
            pbox_app_rockit_start_audiocard_player(source, pboxBtSinkdata->pcmSampeFreq, pboxBtSinkdata->pcmChannel, hal_get_audio_card(SRC_EXT_BT));
        } break;

        case SRC_CHIP_UAC: {
            pbox_app_rockit_start_audiocard_player(SRC_CHIP_UAC, pboxUacdata->freq, 2, hal_get_audio_card(SRC_CHIP_UAC));
        } break;

        case SRC_EXT_USB: {
            pbox_app_rockit_start_audiocard_player(SRC_EXT_USB, 48000, 2, hal_get_audio_card(SRC_EXT_USB));
        } break;
        case SRC_EXT_AUX: {
            pbox_app_rockit_start_audiocard_player(SRC_EXT_AUX, 48000, 2, hal_get_audio_card(SRC_EXT_AUX));
        } break;
    }

    pbox_app_music_original_singer_open(!pboxUIdata->vocalSplit, policy);
    pbox_app_resume_volume_later(650);
}

//like BT, uac, or Usb connected with extern MCU, these are passive input source.
//for the audio stream are streamed to rockchips ics.
//another way, USB connected to rockchips was not passive source. for we play USB locally and actively.
void pbox_app_drive_passive_player(input_source_t source, play_status_t status, display_t policy) {
    ALOGD("%s, play status [%d->%d]\n", __func__, pboxUIdata->play_status, status);
    if(pboxUIdata->play_status == status) {
        return;
    }
    switch(status) {
        case PLAYING: {
            pbox_app_restart_passive_player(source, true, policy);
        } break;

        case _STOP: {
            pbox_app_music_stop(policy);
        } break;
    }
    pboxUIdata->play_status = status;
    pbox_app_echo_playingStatus((status==PLAYING) ? true: false, policy);
}

void pbox_app_record_start(input_source_t source, bool start, display_t policy) {
    if (start && (pboxData->inputDevice == SRC_CHIP_UAC))
        pbox_app_rockit_start_recorder(source, 48000, 2, NULL);
    else
        pbox_app_rockit_stop_recorder(source);
}

void pbox_app_bt_pair_enable(bool enable, display_t policy) {
    pbox_btsink_pair_enable(enable);
    //no ui display now
}

void pbox_app_bt_local_update(display_t policy) {
    pbox_btsink_local_update();
    //no ui display now
}

void pbox_app_set_vendor_state(bool enable, display_t policy) {
    pbox_btsink_set_vendor_state(enable);
    //no ui display now
}

void pbox_app_bt_sink_onoff(bool on, display_t policy) {
    pbox_btsink_onoff(on);
    //no ui display now
}

void pbox_app_start_bluealsa_only(display_t policy) {
        //start bluealsa only
        pbox_btsink_start_only_bluealsa();
}

void pbox_app_restart_btsink(bool only, display_t policy) {
        pbox_btsink_start_only_aplay(only);
        //no ui display now
}

void pbox_app_start_uac_poll(display_t policy) {
    //nothing
    //no ui display now
}

bool is_input_source_automode(void) {
    return pboxUIdata->autoSource;
}
bool is_input_source_selected(input_source_t source, switch_source_t mode) {
    if (mode == MANUAL) {
        return (pboxUIdata->autoSource==false) && (pboxData->inputDevice == source);
    } else if(mode == AUTO) {
        return (pboxUIdata->autoSource==true) && (pboxData->inputDevice == source);
    } else {
        return (pboxData->inputDevice == source);
    }
}

void pbox_app_autoswitch_next_input_source(input_source_t currentSource, display_t policy) {
    input_source_t dest;
    int index = -1;
    if (pboxUIdata->autoSource != true)
        return;

    for(int i = 0; i< SRC_NUM; i++) {
        if(!input_order_config[i].enable) {
            continue;
        }

        if(input_order_config[i].source == currentSource) {
            index = i;
            break;
        }
    }

    if(index == -1)
        return;

    for(int i= index+1; i < SRC_NUM; i++) {
        if(!input_order_config[i].enable) {
            continue;
        }

        if(input_order_config[i].source == SRC_CHIP_UAC) {
            pbox_app_switch_to_input_source(SRC_CHIP_UAC, policy);
            return;
        }

        if(is_dest_source_auto_switchable(input_order_config[i].source)) {
            pbox_app_switch_to_input_source(input_order_config[i].source, policy);
            return;
        }
    }
}

bool isInputSourceConnected(input_source_t source) {
    switch(source) {
        case SRC_CHIP_BT: {
            return isBtConnected();
        } break;

        case SRC_CHIP_USB: {
            return isUsbDiskConnected();
        } break;

        case SRC_CHIP_UAC: {
            return pboxUacdata->state;
        } break;

        case SRC_EXT_BT:
        case SRC_EXT_USB:
        case SRC_EXT_AUX: {
            return true;//tmp code
        } break;

        default: {
            break;
        }
    }

    return false;
}

bool is_dest_source_auto_switchable(input_source_t destSource) {
    int index = -1;

    if (pboxUIdata->autoSource == false) {
        return false;
    }

    for(int i= 0; i< SRC_NUM; i++) {
        if(!input_order_config[i].enable) {
            continue;
        }
        if(input_order_config[i].source == destSource) {
            index = i;
            break;
        }
    }

    if(index == -1)
        return false;

    for(int i = 0; i < index; i++) {
        if(!input_order_config[i].enable) {
            continue;
        }
        if(isInputSourceConnected(input_order_config[i].source)) {
            return false;//a higher source is connected..
        }
    }

    if(isInputSourceConnected(destSource)) {
        return true;
    }

    return false;
}

//this means we switch source actively...
void pbox_app_switch_to_input_source(input_source_t source, display_t policy) {
    ALOGD("%s, source: [%s->%s]\n", __func__, getInputSourceString(pboxData->inputDevice), getInputSourceString(source));
    //if(pboxData->inputDevice == source)
    //    return;
    pbox_app_music_stop(policy);
    pboxData->inputDevice = source;
    switch(source) {
        case SRC_CHIP_BT:
        case SRC_EXT_BT: {
            pbox_app_echo_bt_state(pboxBtSinkdata->btState, policy);
            if(isInputSourceConnected(source)) {
                pbox_app_restart_passive_player(source, false, policy);
            } else {
                pbox_app_echo_tack_info(" ", " ", policy);
            }
        } break;
        case SRC_EXT_AUX: {
            pbox_app_restart_passive_player(SRC_EXT_AUX, false, policy);
        } break;
        case SRC_EXT_USB: {
            pbox_app_restart_passive_player(SRC_EXT_USB, false, policy);
        } break;
        case SRC_CHIP_USB: {
            pbox_app_show_usb_state(pboxUsbdata->usbState, policy);
            if(isInputSourceConnected(SRC_CHIP_USB)) {
                //pbox_app_usb_list_update(pboxTrackdata->track_id, policy);
                pbox_app_echo_tack_info(pbox_app_usb_get_title(pboxTrackdata->track_id), " ", policy);
            } else {
                pbox_app_echo_tack_info(" ", " ",  policy);
            }
        } break;

        case SRC_CHIP_UAC: {
            pboxUIdata->play_status = pboxUacdata->state;
            pbox_app_echo_playingStatus(pboxUacdata->state, policy);
            pbox_multi_echoUacState(UAC_ROLE_SPEAKER, pboxUacdata->state, policy);
            pbox_app_echo_tack_info(" ", " ",  policy);
            pbox_app_restart_passive_player(SRC_CHIP_UAC, false, policy);

            //if you awlays want disable uac recored when swith source, enable this code..
            if(pboxUacdata->record_state)
                pbox_app_record_start(SRC_CHIP_UAC, pboxUacdata->record_state, policy);
        } break;
    }
    //no ui display now
    pbox_app_reset_gender_info(policy);
}

void pbox_app_music_pause(display_t policy)
{
    if(pboxUIdata->play_status != PLAYING)
        return;
    switch (pboxData->inputDevice) {
        case SRC_CHIP_BT: {
            if (isBtA2dpConnected()) {
                pbox_btsink_playPause(false);
            }
        } break;

        case SRC_CHIP_USB: {
        } break;

        case SRC_CHIP_UAC: {
        } break;
    }

    pbox_app_rockit_pause_player(pboxData->inputDevice);
    pbox_multi_echoIsPlaying(false, policy);
    pboxUIdata->play_status = _PAUSE;
}

void pbox_app_music_trackid(uint32_t id, display_t policy) {
    ALOGD("%s, id:%d\n", __func__, id);
    pboxTrackdata->track_id = id;
}

void pbox_app_music_start(display_t policy) {
    switch (pboxData->inputDevice) {
        case SRC_CHIP_BT: {
            if (isBtA2dpConnected()) {
                pbox_btsink_playPause(true);
            }
        } break;

        case SRC_CHIP_USB: {
            char track_uri[MAX_MUSIC_NAME_LENGTH+1];
            ALOGD("pboxTrackdata->track_id:%d, track_num:%d\n", pboxTrackdata->track_id, pboxTrackdata->track_num);
            for (int i=0 ; i< pboxTrackdata->track_num; i++) {
                ALOGD("pboxTrackdata->track_list[%d]:%s\n", i, pboxTrackdata->track_list[i].title);
            }
            if(pboxTrackdata->track_num == 0) {
                return;
            }
            char *track_name = pbox_app_usb_get_title(pboxTrackdata->track_id);
            sprintf(track_uri, MUSIC_PATH"%s", track_name);
            ALOGW("play track [%s]\n", track_uri);
            pbox_app_rockit_start_local_player(track_uri, NULL);
            pbox_app_music_set_music_volume(pboxUIdata->musicVolumeLevel, policy);
            pbox_app_music_original_singer_open(!pboxUIdata->vocalSplit, policy);
            pbox_app_echo_tack_info(track_name, NULL, policy);
            pbox_app_reset_gender_info(policy);
        } break;

        case SRC_CHIP_UAC: {
            pbox_app_echo_tack_info("", NULL, policy);
            pbox_app_reset_gender_info(policy);
            pbox_app_restart_passive_player(SRC_CHIP_UAC, true, policy);
        } break;
        default:
            break;
    }
}

void pbox_app_music_resume(display_t policy) {
    //pbox_app_music_stop(policy);
    switch (pboxData->inputDevice) {
        case SRC_CHIP_BT: {
            if (isBtA2dpConnected()&&(!isBtA2dpStreaming())) {
                pbox_btsink_playPause(true);
            }
        } break;

        case SRC_CHIP_USB: {
            if (pboxTrackdata->track_num == 0) {
                return;
            }
            if(pboxUIdata->play_status != _PAUSE || pboxData->trackchanged) {
                pboxData->trackchanged = false;
                pbox_app_music_start(policy);
            }
            else {
                pbox_app_rockit_resume_player(SRC_CHIP_USB);
            }

            pbox_app_rockit_get_player_duration(SRC_CHIP_USB);
        } break;

        case SRC_CHIP_UAC: {
            pbox_app_music_start(policy);
        } break;
    }
    pboxUIdata->play_status = PLAYING;
    pbox_multi_echoIsPlaying(true, policy);
}

void pbox_app_music_stop(display_t policy)
{
     ALOGD("%s\n", __func__);
    if (pboxUIdata->play_status == IDLE || pboxUIdata->play_status == _STOP) {
        //return;
    }
    switch (pboxData->inputDevice) {
        case SRC_CHIP_BT: {
            if (isBtA2dpConnected())
                pbox_btsink_a2dp_stop();
            pbox_app_rockit_stop_player(SRC_CHIP_BT);
        } break;

        case SRC_EXT_AUX: {
            pbox_app_rockit_stop_player(SRC_EXT_AUX);
        } break;

        case SRC_CHIP_USB: {
            pbox_app_rockit_stop_player(SRC_CHIP_USB);
        } break;

        case SRC_EXT_USB: {
            pbox_app_rockit_stop_player(SRC_EXT_USB);
        } break;

        case SRC_CHIP_UAC: {
            pbox_app_rockit_stop_player(SRC_CHIP_UAC);
            //enable the code if u not nedd uac recored when switch to other source.
            pbox_app_rockit_stop_recorder(SRC_CHIP_UAC);
        } break;

        default:
        break;
    }

    pbox_multi_echoIsPlaying(false, policy);
    pboxUIdata->play_status = _STOP;
}

void pbox_app_music_set_main_volume(float volume, display_t policy) {
    ALOGD("%s main volume: %f\n", __func__, volume);
    pboxUIdata->mainVolumeLevel = volume;
    pbox_app_rockit_set_player_volume(pboxData->inputDevice, volume);
    pbox_multi_echoMainVolumeLevel(volume, policy);
}

void pbox_app_music_set_music_volume(float volume, display_t policy) {
    ALOGW("%s music volume: %f\n", __func__, volume);
    pboxUIdata->musicVolumeLevel = volume;
    pbox_app_rockit_set_music_volume(pboxData->inputDevice, volume);
    pbox_multi_echoMusicVolumeLevel(volume, policy);
}

void pbox_app_music_album_next(bool next, display_t policy)
{
    ALOGD("%s, next:%d, inputDevice:%d\n", __func__, next, pboxData->inputDevice);
    switch (pboxData->inputDevice) {
        case SRC_CHIP_BT: {
            if (isBtA2dpConnected()) {
                pbox_app_rockit_set_player_volume(SRC_CHIP_BT, MIN_MAIN_VOLUME);
                if(next) {
                    pbox_btsink_music_next(true);
                }
                else {
                    pbox_btsink_music_next(false);;
                }
                //resume the volume in the pistion update...
                if(pboxUIdata->play_status == PLAYING) {
                    //pbox_app_music_pause(policy);
                    //pbox_app_music_resume(policy);
                }
            }
        } break;

        case SRC_CHIP_USB: {
            uint32_t *const pId = &(pboxTrackdata->track_id);
            if(pboxTrackdata->track_num == 0) {
                return;
            }

            if(next) {
                (*pId)++;
                if(*pId >= pboxTrackdata->track_num) *pId = 0;
            }
            else {
                if(*pId == 0) {
                    *pId = pboxTrackdata->track_num - 1;
                }
                else {
                    (*pId)--;
                }
            }

            if(*pId < pboxTrackdata->track_num) {
                printf("%s pId:%d total:%d\n", __func__, *pId, pboxTrackdata->track_num);
                pbox_app_echo_tack_info(pboxTrackdata->track_list[*pId].title, NULL,  DISP_All);
                pbox_app_reset_gender_info(policy|DISP_LCD);
            }

            if(pboxUIdata->play_status == PLAYING) {
                pbox_app_music_stop(policy);
                pbox_app_music_resume(policy);
            } else {
                pboxData->trackchanged = true;
            }
        } break;

        case SRC_CHIP_UAC: {
            char text[32] = {0};
            snprintf(text, 31, "UAC NO SUPPORT:%s !!!", next? "NEXT":"PREV");
            pbox_app_echo_tack_info(text, NULL,  DISP_All|DISP_LCD);
            pbox_app_reset_gender_info(policy|DISP_LCD);
        } break;

        default:
        break;
    }
}

void pbox_app_set_dest_vocal_lib(vocal_lib_t vocalib, display_t policy) {
    pbox_vocal_t vocal = {
        .enable = pboxUIdata->vocalSplit,
        .humanLevel = pboxUIdata->humanLevel,
        .accomLevel = pboxUIdata->accomLevel,
        .reservLevel = pboxUIdata->reservLevel,
    };

    vocal.vocaltype = pboxUIdata->vocaltype = (VOCAL_HUMAN == vocalib)? VOCAL_HUMAN: VOCAL_GUITAR;
    pbox_app_rockit_set_player_seperate(pboxData->inputDevice, vocal);
    pbox_multi_echoGuitarVocalib(vocal, policy);
}

void pbox_app_switch_vocal_lib(display_t policy) {//(bool vocalib) {
    pbox_vocal_t vocal = {
        .enable = pboxUIdata->vocalSplit,
        .humanLevel = pboxUIdata->humanLevel,
        .accomLevel = pboxUIdata->accomLevel,
        .reservLevel = pboxUIdata->reservLevel,
    };

    vocal.vocaltype = pboxUIdata->vocaltype = (VOCAL_GUITAR == pboxUIdata->vocaltype)? VOCAL_HUMAN: VOCAL_GUITAR;
    pbox_app_rockit_set_player_seperate(pboxData->inputDevice, vocal);
    pbox_multi_echoGuitarVocalib(vocal, policy);
}

void pbox_app_music_original_singer_open(bool orignal, display_t policy)
{
    pbox_vocal_t vocal = {
        .enable = !orignal,
        .humanLevel = pboxUIdata->humanLevel,
        .accomLevel = pboxUIdata->accomLevel,
        .reservLevel = pboxUIdata->reservLevel,
        .vocaltype = pboxUIdata->vocaltype,
    };

    pboxUIdata->vocalSplit = !orignal;
    pbox_app_rockit_set_player_seperate(pboxData->inputDevice, vocal);
    pbox_multi_echoVocalFadeoutSwitch(vocal.enable , vocal.humanLevel, vocal.accomLevel, vocal.reservLevel, policy);
}

void pbox_app_music_vocal_seperate_open(bool seperate, display_t policy)
{
    pbox_vocal_t vocal = {
        .enable = seperate,
        .humanLevel = pboxUIdata->humanLevel,
        .accomLevel = pboxUIdata->accomLevel,
        .reservLevel = pboxUIdata->reservLevel,
        .vocaltype = pboxUIdata->vocaltype,
    };

    pboxUIdata->vocalSplit = seperate;
    pbox_app_rockit_set_player_seperate(pboxData->inputDevice, vocal);
    pbox_multi_echoVocalFadeoutSwitch(vocal.enable , vocal.humanLevel, vocal.accomLevel, vocal.reservLevel, policy);
}

//album mode: shuffle, sequence, repeat, repeat one.....
void pbox_app_music_album_loop(uint32_t mode, display_t policy) {

}

void pbox_app_music_seek_position(uint32_t dest, uint32_t duration, display_t policy) {
    if (isBtA2dpConnected())
        return;

    pbox_app_rockit_set_player_seek(pboxData->inputDevice, dest);
    pbox_multi_echoTrackPosition(false, dest, duration, policy);
}

void pbox_app_music_set_mic_all(uint32_t index, mic_state_t micdata, display_t policy) {
    if(index >= hal_get_mic_guitar_num())
        return;
    pboxUIdata->micData[index] = micdata;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_ALL, micdata);
    pbox_multi_echoMicVolumeLevel(index, micdata.micVolume, policy);
    pbox_multi_echoMicMux(index, micdata.micMux, policy);
    pbox_multi_echoMicMute(index, micdata.micmute, policy);
    pbox_multi_echoRevertMode(index, micdata.reverbMode, policy);
    pbox_multi_echoEcho3A(index, micdata.echo3a, policy);
    pbox_multi_echoMicBass(index, micdata.micBass, policy);
    pbox_multi_echoMicTreble(index, micdata.micTreble, policy);
    pbox_multi_echoMicReverb(index, micdata.micReverb, policy);
}

void pbox_app_music_set_mic_volume(uint32_t index, float volume, display_t policy) {
    if(index >= hal_get_mic_guitar_num())
        return;
    pboxUIdata->micData[index].micVolume = volume;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_VOLUME, pboxUIdata->micData[index]);
    pbox_multi_echoMicVolumeLevel(index, volume, policy);
}

void pbox_app_music_set_mic_mute(uint8_t index, bool mute, display_t policy){
    if(index >= hal_get_mic_guitar_num())
        return;
    pboxUIdata->micData[index].micmute = mute;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_MUTE, pboxUIdata->micData[index]);
    pbox_multi_echoMicMute(index, mute, policy);
}

void pbox_app_music_init(void) {
    pbox_app_music_set_main_volume(pboxUIdata->mainVolumeLevel, DISP_All);
    pbox_app_music_set_music_volume(pboxUIdata->musicVolumeLevel, DISP_All);
    //pbox_app_music_set_accomp_music_level(pboxUIdata->accomLevel, DISP_All);
    //pbox_app_music_set_human_music_level(pboxUIdata->humanLevel, DISP_All);
    pbox_app_music_original_singer_open(!(pboxUIdata->vocalSplit), DISP_All);
    pbox_app_music_set_eq_mode(pboxUIdata->eqmode, DISP_All);
    #if ENABLE_EXT_BT_MCU
        pbox_app_music_set_placement(pboxUIdata->placement, DISP_All);
        pbox_app_music_set_stereo_mode(pboxUIdata->stereo, DISP_All);
        pbox_app_switch_to_input_source(SRC_EXT_BT, DISP_All);
    #endif
}

void pbox_app_music_mics_init(display_t policy) {
    for (int index = 0; index < hal_get_mic_guitar_num(); index++) {
        //varaible init func have moved to pbox_app_ui_load()
        pbox_app_music_set_mic_all(index, pboxUIdata->micData[index], policy);
    }
}

void pbox_app_music_set_mic_mux(uint8_t index, mic_mux_t mux, display_t policy) {
    if(index >= hal_get_mic_guitar_num())
        return;
    pboxUIdata->micData[index].micMux = mux;
    //pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_MUX, pboxUIdata->micData[index]);
    pbox_multi_echoMicMux(index, mux, policy);
}

void pbox_app_music_set_mic_treble(uint8_t index, float treble, display_t policy) {
    if(index >= hal_get_mic_guitar_num())
        return;
    pboxUIdata->micData[index].micTreble = treble;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_TREBLE, pboxUIdata->micData[index]);
    pbox_multi_echoMicTreble(index, treble, policy);
}

void pbox_app_music_set_mic_bass(uint8_t index, float bass, display_t policy) {
    if(index >= hal_get_mic_guitar_num())
        return;
    pboxUIdata->micData[index].micBass = bass;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_BASS, pboxUIdata->micData[index]);
    pbox_multi_echoMicBass(index, bass, policy);
}

void pbox_app_music_set_mic_reverb(uint8_t index, float reverb, display_t policy) {
    if(index >= hal_get_mic_guitar_num())
        return;
    pboxUIdata->micData[index].micReverb = reverb;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_REVERB, pboxUIdata->micData[index]);
    pbox_multi_echoMicReverb(index, reverb, policy);
}

void pbox_app_music_set_accomp_music_level(uint32_t volume, display_t policy) {
    pbox_vocal_t vocal = {
        .enable = pboxUIdata->vocalSplit,
        .humanLevel = pboxUIdata->humanLevel,
        .accomLevel = volume,
        .reservLevel = pboxUIdata->reservLevel,
        .vocaltype = pboxUIdata->vocaltype,
    };
    pboxUIdata->accomLevel = volume;

    ALOGW("%s alevel: %d, lib:%d, seperate:%d\n", __func__, volume, pboxUIdata->vocaltype, vocal.enable);
    pbox_app_rockit_set_player_seperate(pboxData->inputDevice, vocal);

    pbox_multi_echoAccompMusicLevel(volume, policy);
}

void pbox_app_music_play_notice_number(uint8_t number, display_t policy) {
    pbox_app_rockit_play_notice_number(number);
}

void pbox_app_music_set_human_music_level(uint32_t volume, display_t policy) {
    pbox_vocal_t vocal = {
        .enable = pboxUIdata->vocalSplit,
        .humanLevel = volume,
        .accomLevel = pboxUIdata->accomLevel,
        .reservLevel = pboxUIdata->reservLevel,
        .vocaltype = pboxUIdata->vocaltype,
    };
    pboxUIdata->humanLevel = volume;

    ALOGW("%s hlevel: %d, lib:%d, seperate:%d\n", __func__, volume, pboxUIdata->vocaltype, pboxUIdata->vocalSplit);
    pbox_app_rockit_set_player_seperate(pboxData->inputDevice, vocal);
    pbox_multi_echoHumanMusicLevel(volume, policy);
}

void pbox_app_music_set_reserv_music_level(uint32_t volume, display_t policy) {
    pbox_vocal_t vocal = {
        .enable = pboxUIdata->vocalSplit,
        .humanLevel = pboxUIdata->humanLevel,
        .accomLevel = pboxUIdata->accomLevel,
        .reservLevel = volume,
        .vocaltype = pboxUIdata->vocaltype,
    };
    pboxUIdata->reservLevel = volume;
    ALOGW("%s rlevel:%d, lib:%d, seperate:%d\n", __func__, volume, pboxUIdata->vocaltype, pboxUIdata->vocalSplit);
    pbox_app_rockit_set_player_seperate(pboxData->inputDevice, vocal);
    pbox_multi_echoReservLevel(volume, policy);
}

void pbox_app_music_set_echo_3a(uint8_t index, bool enable, display_t policy) {
    if(index >= hal_get_mic_guitar_num())
        return;
    pboxUIdata->micData[index].echo3a = enable;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_ECHO_3A, pboxUIdata->micData[index]);
    pbox_multi_echoEcho3A(index, enable, policy);
}

void pbox_app_music_set_recoder_revert(uint8_t index, pbox_revertb_t reverbMode, display_t policy) {
    if(index >= hal_get_mic_guitar_num())
        return;
    pboxUIdata->micData[index].reverbMode = reverbMode;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_REVERB_MODE, pboxUIdata->micData[index]);
    pbox_multi_echoRevertMode(index, reverbMode, policy);
}

void pbox_app_music_set_stereo_mode(stereo_mode_t stereo, display_t policy) {
    ALOGD("%s :%d\n", __func__, stereo);
    pboxUIdata->stereo = stereo;
    pbox_app_rockit_set_stereo_mode(pboxData->inputDevice, stereo);
    pbox_multi_echoStereoMode(stereo, policy);
}

void pbox_app_music_set_outdoor_mode(inout_door_t outdoor, display_t policy) {
    ALOGD("%s :%d\n", __func__, outdoor);
    pboxUIdata->outdoor = outdoor;
    pbox_app_rockit_set_outdoor_mode(pboxData->inputDevice, outdoor);
    pbox_multi_echoOutdoorMode(outdoor, policy);
}

void pbox_app_music_set_placement(placement_t place, display_t policy) {
    ALOGD("%s :%d\n", __func__, place);
    pboxUIdata->placement = place;
    pbox_app_rockit_set_placement(pboxData->inputDevice, place);
    pbox_multi_echoPlacement(place, policy);
}

void pbox_app_music_set_eq_mode(equalizer_t mode, display_t policy) {
    ALOGD("%s :%d\n", __func__, mode);
    pboxUIdata->eqmode = mode;
    pbox_app_rockit_set_eq_mode(pboxData->inputDevice, mode);
    pbox_multi_echoEqMode(mode, policy);
}

void pbox_app_light_bar_set_mode(uint8_t mode, display_t policy) {
    ALOGD("%s: %d\n", __func__, mode);
    pbox_multi_echo_lightbar_mode(mode, policy);
}

void pbox_app_light_bar_set_power_onoff(uint8_t poweron, display_t policy) {
    ALOGD("%s: %d\n", __func__, poweron);
    pbox_multi_echo_lightbar_power_onoff(poweron, policy);
}

void pbox_app_light_strobe_ctrl(uint8_t strobe, display_t policy) {
    ALOGD("%s: %d\n", __func__, strobe);
    pbox_multi_echo_strobe_ctrl(strobe, policy);
}

void pbox_app_party_light_enable(uint8_t party, display_t policy) {
    ALOGD("%s: %d\n", __func__, party);
    pbox_multi_echo_party_light_mode(party, policy);
}

void pbox_app_eq_set_bassboost_onoff(uint8_t bass, display_t policy) {
    ALOGD("%s: %d\n", __func__, bass);
    pbox_multi_echo_eq_bass_onoff(bass, policy);
    pbox_app_rockit_set_bassboost(pboxData->inputDevice, bass);
}


void pbox_app_tunning_set(bool enable, display_t policy) {
    ALOGW("%s %d\n", __func__, enable);
    pbox_app_rockit_set_tunning(enable);
}

void pbox_app_main_volume_up(display_t policy) {
    float volume = pboxUIdata->mainVolumeLevel;
    volume += (MAX_MAIN_VOLUME-MIN_MAIN_VOLUME)/10;
    volume = volume> MAX_MAIN_VOLUME?MAX_MAIN_VOLUME:volume;
    volume = volume< MIN_MAIN_VOLUME?MIN_MAIN_VOLUME:volume;

    ALOGD("%s volume up:%f, mainVol:%f\n", __func__, volume, pboxUIdata->mainVolumeLevel);
    if(pboxUIdata->mainVolumeLevel != volume)
    pbox_app_music_set_main_volume(volume, policy);

    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) 
        pbox_app_music_resume(policy);
}

void pbox_app_main_volume_down(display_t policy) {
    float volume = pboxUIdata->mainVolumeLevel;
    volume -= (MAX_MAIN_VOLUME-MIN_MAIN_VOLUME)/10;
    volume = volume> MAX_MAIN_VOLUME?MAX_MAIN_VOLUME:volume;
    volume = volume< MIN_MAIN_VOLUME?MIN_MAIN_VOLUME:volume;

    ALOGD("%s volume down:%f\n", __func__, volume);
    pbox_app_music_set_main_volume(volume, policy);

    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) 
        pbox_app_music_resume(policy);
}

void pbox_app_music_volume_up(display_t policy) {
    float volume = pboxUIdata->musicVolumeLevel;
    volume += (MAX_MAIN_VOLUME-MIN_MAIN_VOLUME)/20;
    volume = volume> MAX_MAIN_VOLUME?MAX_MAIN_VOLUME:volume;
    volume = volume< MIN_MAIN_VOLUME?MIN_MAIN_VOLUME:volume;

    ALOGD("%s :[%f]->[%f]\n", __func__, pboxUIdata->musicVolumeLevel, volume);
    if(pboxUIdata->musicVolumeLevel != volume)
        pbox_app_music_set_music_volume(volume, policy);

    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) 
        pbox_app_music_resume(policy);
}

void pbox_app_music_volume_down(display_t policy) {
    float volume = pboxUIdata->musicVolumeLevel;
    volume -= (MAX_MAIN_VOLUME-MIN_MAIN_VOLUME)/20;
    volume = volume> MAX_MAIN_VOLUME?MAX_MAIN_VOLUME:volume;
    volume = volume< MIN_MAIN_VOLUME?MIN_MAIN_VOLUME:volume;

    ALOGD("%s [%f]->[%f]\n", __func__, volume, pboxUIdata->musicVolumeLevel, volume);
    if(pboxUIdata->musicVolumeLevel != volume)
        pbox_app_music_set_music_volume(volume, policy);

    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) 
        pbox_app_music_resume(policy);
}

int vocal_table[] = {5, 100}; //keep the last as max value
void pbox_app_music_human_vocal_level_cycle(display_t policy) {
    static uint8_t vocal = 0;
    uint8_t id = 0;
    int table_size = sizeof(vocal_table)/sizeof(int);

    if(pboxUIdata->vocalSplit == false) {
        id = sizeof(vocal_table)/sizeof(int) - 1;
    } else {
        for (int i = 0; i < table_size; i++) {
            if(vocal_table[i] == vocal) {
                id = i;
                break;
            }
        }
    }

    id = (id + 1)%table_size;
    pboxUIdata->reservLevel = pboxUIdata->humanLevel = vocal = vocal_table[id];

    //pboxUIdata->reservLevel = 100;
    if(id == 0) {
        pbox_app_music_vocal_seperate_open(true, policy);
    } else if (id == (table_size-1)) {
        pbox_app_music_vocal_seperate_open(false, policy);
    } else {
        pbox_app_music_set_human_music_level(vocal_table[id], policy);
        pbox_app_music_play_notice_number(id, policy);
    }
}

int guitar_table[] = {0, 100};
void pbox_app_music_guitar_vocal_level_cycle(display_t policy) {
    static uint8_t guitar = 0;
    uint8_t id = 0;
    int table_size = sizeof(guitar_table)/sizeof(int);

    if(pboxUIdata->vocalSplit == false) {
        id = sizeof(guitar_table)/sizeof(int) - 1;
    } else {
        int table_size = sizeof(guitar_table)/sizeof(int);
        for (int i = 0; i < table_size; i++) {
            if(guitar_table[i] == guitar) {
                id = i;
                break;
            }
        }
    }

    id = (id + 1)%table_size;
    pboxUIdata->humanLevel = pboxUIdata->reservLevel = guitar = guitar_table[id];

    //pboxUIdata->humanLevel = 100;
    if(id == 0) {
        pbox_app_music_vocal_seperate_open(true, policy);
    } else if (id == (table_size-1)) {
        pbox_app_music_vocal_seperate_open(false, policy);
    } else {
        pbox_app_music_set_human_music_level(guitar_table[id], policy);
        pbox_app_music_play_notice_number(id, policy);
    }
}

void pbox_version_print(void) {
    int day, year, mon;
    char month[4];
    const char *dateString = __DATE__;

    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    sscanf(dateString, "%s %d %d", month, &day, &year);

    for (mon = 0; mon < 12; mon++) {
        if (strcasecmp(month, months[mon]) == 0) {
            break;
        }
    }

    ALOGW("%s: %04d-%02d-%02d %s\n", __func__, year, mon + 1, day, __TIME__);
}

void pbox_app_uac_state_change(uac_role_t role, bool start, display_t policy) {
    ALOGD("%s\n", __func__);
    switch (role) {
        case UAC_ROLE_SPEAKER: {
            if(pboxUacdata->state == start)
                    return;
            ALOGD("%s player start=%d\n", __func__, start);
            pboxUacdata->state = start;

            if (start && is_dest_source_auto_switchable(SRC_CHIP_UAC)) {
                pbox_app_switch_to_input_source(SRC_CHIP_UAC, policy);
            }

            if(!is_input_source_selected(SRC_CHIP_UAC, ANY)) {
                return;
            }

            //pboxUIdata->play_status = start;
            pbox_app_drive_passive_player(SRC_CHIP_UAC, start? PLAYING:_STOP, policy);
            pbox_app_echo_playingStatus(pboxUacdata->state, policy);
            pbox_multi_echoUacState(role, start, policy);
        } break;

        case UAC_ROLE_RECORDER: {
            if(pboxUacdata->record_state == start)
                return;
            ALOGD("%s recorder start=%d\n", __func__, start);
            pboxUacdata->record_state = start;

            pbox_app_record_start(SRC_CHIP_UAC, start, policy);
        }

        default: break;
    }
}

void pbox_app_uac_freq_change(uac_role_t role, uint32_t freq, display_t policy) {
    if(!is_input_source_selected(SRC_CHIP_UAC, ANY)) {
        return;
    }

    ALOGD("%s %s freq:%d\n", __func__, (role==UAC_ROLE_SPEAKER)?"spk":"rec", freq);
    if(pboxUacdata->freq != freq) {
        pboxUacdata->freq = freq;
        pbox_app_rockit_start_audiocard_player(SRC_CHIP_UAC, freq, 2, hal_get_audio_card(SRC_CHIP_UAC));
    }
}

void pbox_app_uac_volume_change(uac_role_t role, uint32_t volume, display_t policy) {
    if(!is_input_source_selected(SRC_CHIP_UAC, ANY)) {
        return;
    }

    pboxUIdata->musicVolumeLevel = PERCENT2TARGET((float)volume, MIN_MAIN_VOLUME, MAX_MAIN_VOLUME);
    ALOGD("%s volume:%d->%f\n", __func__, volume, pboxUIdata->musicVolumeLevel);
    if((role == UAC_ROLE_SPEAKER)) {
        pbox_app_rockit_set_uac_volume(role, pboxUIdata->musicVolumeLevel);
        pbox_multi_echoUacVolume(role, volume, policy);
    }
}

void pbox_app_uac_mute_change(uac_role_t role, bool mute, display_t policy) {
    if(!is_input_source_selected(SRC_CHIP_UAC, ANY)) {
        return;
    }

    pbox_app_rockit_set_mute(role, mute);
    pbox_multi_echoUacMute(role, mute, policy);
}

void pbox_app_uac_ppm_change(uac_role_t role, int32_t ppm, display_t policy) {
    if(!is_input_source_selected(SRC_CHIP_UAC, ANY)) {
        return;
    }

    pbox_app_rockit_set_ppm(role, ppm);
    pbox_multi_echoUacPpm(role, ppm, policy);
}

void pbox_app_adb_connection(bool connect, display_t policy) {
    pbox_app_tunning_set(connect, policy);
}

void pbox_app_usb_start_scan(display_t policy) {
    pbox_app_usb_startScan();
    //nothing to display now
}

void pbox_app_show_usb_state(usb_state_t state, display_t policy) {
    //nothing to notify rockit
    pbox_multi_echoUsbState(state, policy);
}

void pbox_app_usb_list_update(uint32_t trackId, display_t policy) {
    //nothing to notify rockit
    pbox_multi_echoUsbListupdate(trackId, policy);
}

void pbox_app_btsoc_init(void) {
    pbox_app_echo_poweron_status(DISP_All);
    pbox_app_echo_music_volume(DISP_All);
}

void pbox_app_echo_dsp_version(display_t policy) {
    //pbox_app_btsoc_echo_dsp_version("v1.00");
    pbox_multi_echo_dsp_version("v1.00", policy);
    //nothing to notify rockit
    //nothing to do with ui
}

void pbox_app_echo_main_volume(display_t policy) {
    pbox_multi_echoMainVolumeLevel(pboxUIdata->mainVolumeLevel, policy);
}

void pbox_app_echo_music_volume(display_t policy) {
    pbox_multi_echoMusicVolumeLevel(pboxUIdata->musicVolumeLevel, policy);
}

void pbox_app_echo_placement(display_t policy) {
    pbox_multi_echoPlacement(pboxUIdata->placement, policy);
}

void pbox_app_echo_inout_door(display_t policy) {
    pbox_multi_echoOutdoorMode(pboxUIdata->outdoor, policy);
}

void pbox_app_echo_micMux(uint8_t index, display_t policy) {
    if(index >= hal_get_mic_guitar_num())
        return;
    pbox_multi_echoMicMux(index, pboxUIdata->micData[index].micMux, policy);
}

void pbox_app_echo_poweron_status(display_t policy) {
    //it mustbe powered on
    pbox_multi_echoPoweron_status(true, policy);
}

void pbox_app_echo_stereo_mode(display_t policy) {
    pbox_multi_echoStereoMode(pboxUIdata->stereo, policy);
}

void pbox_app_echo_voice_fadeout_mode(display_t policy) {
    uint32_t hlevel = pboxUIdata->humanLevel;
    uint32_t alevel = pboxUIdata->accomLevel;
    uint32_t rlevel = pboxUIdata->reservLevel;
    bool seperate = pboxUIdata->vocalSplit;

    pbox_multi_echoVocalFadeoutSwitch(seperate , hlevel, alevel, rlevel, policy);
}

void pbox_app_echo_input_source(display_t policy) {
    pbox_multi_echoInputSource(pboxData->inputDevice, pboxUIdata->play_status, policy);
}

void pbox_app_echo_micdata(uint8_t index, mic_set_kind_t kind, display_t policy) {
    if(index >= hal_get_mic_guitar_num())
        return;
    switch (kind) {
        case MIC_SET_DEST_ECHO_3A: {
            pbox_multi_echoEcho3A(index, pboxUIdata->micData[index].echo3a, policy);
        } break;
        case MIC_SET_DEST_MUTE: {
            pbox_multi_echoMicMute(index, pboxUIdata->micData[index].micmute, policy);
        } break;
        case MIC_SET_DEST_MUX: {
            pbox_multi_echoMicMux(index, pboxUIdata->micData[index].micMux, policy);
        } break;
        case MIC_SET_DEST_REVERB_MODE: {
            pbox_multi_echoRevertMode(index, pboxUIdata->micData[index].reverbMode, policy);
        } break;
        case MIC_SET_DEST_VOLUME: {
            pbox_multi_echoMicVolumeLevel(index, pboxUIdata->micData[index].micVolume, policy);
        } break;
        case MIC_SET_DEST_TREBLE: {
            pbox_multi_echoMicTreble(index, pboxUIdata->micData[index].micTreble, policy);
        } break;
        case MIC_SET_DEST_BASS: {
            pbox_multi_echoMicBass(index, pboxUIdata->micData[index].micBass, policy);
        } break;
        case MIC_SET_DEST_REVERB: {
            pbox_multi_echoMicReverb(index, pboxUIdata->micData[index].micReverb, policy);
        } break;
        default: break;
    }
}

//micMux: bit0-bit7: mic0-mic7
//guitarMux: bit0-bit7: guitar0-guitar7
void pbox_app_get_energyinfo(uint8_t destMux, input_source_t source, uint8_t micMux, uint8_t guitarMux) {
    pbox_app_rockit_get_player_energy(destMux, source, micMux, guitarMux);
}

void pbox_app_btsoc_set_volume(float volume, display_t policy) {
    if(pboxUIdata->mainVolumeLevel == volume)
        return;
    pbox_app_music_set_main_volume(volume, policy);
}

void pbox_app_btsoc_set_music_volume(float volume, display_t policy) {
    if(pboxUIdata->musicVolumeLevel == volume)
        return;
    pbox_app_music_set_music_volume(volume, policy);
}

void pbox_app_btsoc_set_placement(placement_t placement, display_t policy) {
    if(pboxUIdata->placement == placement)
        return;
    pbox_app_music_set_placement(placement, policy);
}

void pbox_app_btsoc_set_outdoor_mode(inout_door_t inout, display_t policy) {
    if(pboxUIdata->outdoor == inout)
        return;
    pbox_app_music_set_outdoor_mode(inout, policy);
}

void pbox_app_btsoc_set_stereo_mode(stereo_mode_t mode, display_t policy) {
    if(pboxUIdata->stereo == mode)
        return;
    pbox_app_music_set_stereo_mode(mode, policy);
}

void pbox_app_btsoc_set_mic_data(mic_data_t data, display_t policy) {
    uint8_t index = data.index;
    mic_set_kind_t kind = data.kind;

    if(index >= hal_get_mic_guitar_num())
        return;
    switch (data.kind) {
        case MIC_SET_DEST_ECHO_3A: {
            if(pboxUIdata->micData[index].echo3a != data.micState.echo3a)
                pbox_app_music_set_echo_3a(index, data.micState.echo3a, policy);
        } break;
        case MIC_SET_DEST_MUTE: {
            if(pboxUIdata->micData[index].micmute != data.micState.micmute)
                pbox_app_music_set_mic_mute(index, data.micState.micmute, policy);
        } break;
        case MIC_SET_DEST_MUX: {
            if(pboxUIdata->micData[index].micMux != data.micState.micMux)
                pbox_app_music_set_mic_mux(index, data.micState.micMux, policy);
        } break;
        case MIC_SET_DEST_REVERB_MODE: {
            if(pboxUIdata->micData[index].reverbMode != data.micState.reverbMode)
                pbox_app_music_set_recoder_revert(index, data.micState.reverbMode, policy);
        } break;
        case MIC_SET_DEST_VOLUME: {
            if(pboxUIdata->micData[index].micVolume != data.micState.micVolume)
                pbox_app_music_set_mic_volume(index, data.micState.micVolume, policy);
        } break;
        case MIC_SET_DEST_TREBLE: {
            if(pboxUIdata->micData[index].micTreble != data.micState.micTreble)
                pbox_app_music_set_mic_treble(index, data.micState.micTreble, policy);
        } break;
        case MIC_SET_DEST_BASS: {
            if(pboxUIdata->micData[index].micBass != data.micState.micBass)
                pbox_app_music_set_mic_bass(index, data.micState.micBass, policy);
        } break;
        case MIC_SET_DEST_REVERB: {
            if(pboxUIdata->micData[index].micReverb != data.micState.micReverb)
                pbox_app_music_set_mic_reverb(index, data.micState.micReverb, policy);
        } break;
        default: break;
    }
}

void pbox_app_btsoc_set_human_voice_fadeout(bool fadeout, display_t policy) {
    if(pboxUIdata->vocalSplit == fadeout)
        return;
    pbox_app_music_original_singer_open(fadeout? false: true, policy);
}

void pbox_app_btsoc_set_human_vocal_level(uint32_t level, display_t policy) {
    if(pboxUIdata->humanLevel == level)
        return;
    pbox_app_music_set_human_music_level(level, policy);
}

void pbox_app_btsoc_set_accomp_vocal_level(uint32_t level, display_t policy) {
    if(pboxUIdata->accomLevel == level)
        return;
    pbox_app_music_set_accomp_music_level(level, policy);
}

void pbox_app_btsoc_set_reserv_vocal_level(uint32_t level, display_t policy) {
    if(pboxUIdata->reservLevel == level)
        return;
    pbox_app_music_set_reserv_music_level(level, policy);
}

void pbox_app_btsoc_set_mic_mux(uint8_t index, mic_mux_t micMux, display_t policy) {
    if(index >= hal_get_mic_guitar_num())
        return;
    if(pboxUIdata->micData[index].micMux == micMux)
        return;
    pbox_app_music_set_mic_mux(index, micMux, policy);
}

void pbox_app_btsoc_set_input_source(input_source_t source, play_status_t status, display_t policy) {
    if(pboxData->inputDevice != source)
        pbox_app_switch_to_input_source(source, policy);

    if(pboxUIdata->play_status != status)
        pbox_app_drive_passive_player(source, status, policy);
}

void pbox_app_post_inout_detect(display_t policy) {
    ALOGW("%s\n", __func__);
    pbox_app_rockit_start_inout_detect();
}

void pbox_app_post_stereo_detect(int agent_role) {
    ALOGW("%s role:%d\n", __func__, agent_role);
    pbox_app_rockit_start_doa_detect(agent_role);//partner:to play detect tone, agent: to detect left/right
 }

void pbox_app_post_stop_detect(void) {
    ALOGW("%s\n", __func__);
    pbox_app_rockit_stop_env_detect();
}

void pbox_app_post_get_sence_value(input_source_t source, size_t scenes) {
   //ALOGW("%s source:%s, scene:0x%02x\n", __func__, getInputSourceString(source), scenes);
   pbox_app_rockit_post_sence_detect(source, scenes);
}

int pbox_app_get_and_clear_inout_value(void) {
    int res = pboxData->inout_shake;
    if (-1 != res)
    ALOGW("%s res:%d %s\n", __func__, res, res == INDOOR? CSTR(INDOOR): (res == OUTDOOR? CSTR(OUTDOOR): "invalid"));
    pboxData->inout_shake = -1;
    return res;
}

int pbox_app_get_and_clear_stereo_position(void) {
    int res = pboxData->stereo_shake;
    ALOGW("%s res:%d %s\n", __func__, res, res == DOA_L? CSTR(DOA_L): (res == DOA_R? CSTR(DOA_R): "invalid"));
    pboxData->stereo_shake = -1;
    return res;
}