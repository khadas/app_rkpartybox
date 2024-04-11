#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "pbox_app.h"
#include "pbox_multi_display.h"
#include "pbox_btsink_app.h"
#include "pbox_rockit_app.h"
#include "pbox_usb_app.h"
#include "pbox_soc_bt_app.h"
#include "board.h"

pbox_data_t pbox_data = {
    .btsink = {
        .pcmSampeFreq = DEFAULT_SAMPLE_FREQ,
        .pcmChannel = 2,
    },
    .ui = {
        .mainVolumeLevel = DEFAULT_MAIN_VOLUME,
        .musicVolumeLevel = 0,
        .accomLevel = 100,
        #if ENABLE_LCD_DISPLAY
        .humanLevel = 15,
        #else
        .humanLevel = 5,
        #endif
        .reservLevel = 100,
        .vocalSplit = false,
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
    .inputDevice = SRC_USB,
    .volume_resume_time = -1,
};

pbox_data_t *const pboxData  = &(pbox_data);
struct _pbox_btsink *const pboxBtSinkdata  = &(pbox_data.btsink);
struct _pbox_ui *const pboxUIdata  = &(pbox_data.ui);
struct _pbox_track *const pboxTrackdata  = &(pbox_data.track);
usb_disk_info_t *const pboxUsbdata  = &(pbox_data.usbDisk);
struct _pbox_uac *const pboxUacdata = &(pbox_data.uac);
static bool play_command_sended = false;
#if ENABLE_UAC
const input_source_t input_priority[SRC_NUM] = {SRC_BT, SRC_USB, SRC_UAC};
#else
const input_source_t input_priority[SRC_NUM] = {SRC_BT, SRC_USB};
#endif

static bool check_dest_source(input_source_t* destSource);
void pbox_app_show_track_position(bool durationOnly, uint32_t current, uint32_t duration, display_t policy) {
    //nothing to notify rockit
    pbox_multi_displayTrackPosition(durationOnly, current, duration, policy);
}

void pbox_app_show_tack_info(char *title, char *artist, display_t policy) {
    //nothing to notify rockit
    pbox_multi_displayTrackInfo(title, artist, policy);
}

void pbox_app_show_bt_state(btsink_state_t state, display_t policy) {
    //nothing to notify rockit
    pbox_multi_displaybtState(state, policy);
}

void pbox_app_resume_volume_later(int32_t msdelay) {
    pboxData->volume_resume_time = msdelay;
}

void pbox_app_show_playingStatus(bool play, display_t policy) {
    //nothing to notify rockit
    pbox_multi_displayIsPlaying(play, policy);
}

void pbox_app_restart_passive_player(input_source_t source, bool restart, display_t policy) {
    ALOGD("%s, source:%d, restart:%d\n", __func__, source, restart);
    if(!is_input_source_selected(source, ANY)) {
        return;
    }

    pbox_app_rockit_set_player_volume(source, MIN_MAIN_VOLUME);
    if(restart) {
        pbox_app_rockit_stop_player(source);
    }

    switch(source) {
        case SRC_BT: {
            pbox_app_rockit_start_audiocard_player(SRC_BT, pboxBtSinkdata->pcmSampeFreq, pboxBtSinkdata->pcmChannel, AUDIO_CARD_BT);
        } break;
#if ENABLE_UAC
        case SRC_UAC: {
            pbox_app_rockit_start_audiocard_player(SRC_UAC, pboxUacdata->freq, 2, AUDIO_CARD_UAC);
        }
#endif
#if ENABLE_EXT_MCU_USB
        case SRC_USB: {
            pbox_app_rockit_start_audiocard_player(SRC_BT, 48000, 2, AUDIO_CARD_USB);
        }
#endif
#if ENABLE_AUX
        case SRC_AUX: {
            pbox_app_rockit_start_audiocard_player(SRC_BT, 48000, 2, AUDIO_CARD_AUX);
        }
#endif
    }

    //pbox_app_music_set_volume(pboxUIdata->mainVolumeLevel, policy);
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
    pbox_app_show_playingStatus((status==PLAYING) ? true: false, policy);
}

void pbox_app_record_start(input_source_t source, bool start, display_t policy) {
    if (start)
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

bool is_dest_source_switchable(input_source_t source, switch_source_t mode) {
    bool result;
    input_source_t dest = source;

    result = check_dest_source(&source);
    result =  result && (source == dest);

    switch (mode) {
        case MANUAL: {
            return result && (pboxUIdata->autoSource == false);
        } break;

        case AUTO: {
            return result && (pboxUIdata->autoSource == true);
        } break;

        case ANY: {
            return result;
        } break;
    }

    return false;
}

void pbox_app_autoswitch_next_input_source(input_source_t currentSource, display_t policy) {
    input_source_t dest;
    int index = -1;
    if (pboxUIdata->autoSource != true)
        return;

    for(int i = 0; i< SRC_NUM; i++) {
        if(input_priority[i] == currentSource) {
            index = i;
            break;
        }
    }

    if(index == -1)
        return;

    for(int i= index+1; i < SRC_NUM; i++) {
#if ENABLE_UAC
        if(input_priority[i] == SRC_UAC) {
            pbox_app_switch_to_input_source(SRC_UAC, policy);
            return;
        }
#endif
        {
            if(is_dest_source_switchable(input_priority[i], AUTO)) {
                pbox_app_switch_to_input_source(input_priority[i], policy);
                return;
            }
        }
    }
}

#ifdef ENABLE_EXT_BT_MCU
bool isInputSourceConnected(input_source_t source) {
    return true;
}
#else
bool isInputSourceConnected(input_source_t source) {
    switch(source) {
        case SRC_BT: {
            return isBtConnected();
        } break;

        case SRC_USB: {
            return isUsbDiskConnected();
        } break;
#if ENABLE_UAC
        case SRC_UAC: {
            return pboxUacdata->state;
        } break;
#endif
        default: {
            break;
        }
    }

    return false;
}
#endif

bool check_dest_source(input_source_t* destSource) {
    int index = -1;

    if (pboxUIdata->autoSource == false) {
        return true;
    }

    for(int i= 0; i< SRC_NUM; i++) {
        if(input_priority[i] == *destSource) {
            index = i;
            break;

        }
    }

    if(index == -1)
        return false;

    for(int i = 0; i < index; i++) {
        if(isInputSourceConnected(input_priority[i])) {
            *destSource = input_priority[i];
            return true;
        }
    }

    if(isInputSourceConnected(*destSource)) {
        return true;
    }

    return false;
}

//this means we switch source actively...
void pbox_app_switch_to_input_source(input_source_t source, display_t policy) {
    ALOGD("%s, source: [%d->%d]\n", __func__, pboxData->inputDevice, source);
    if(pboxData->inputDevice == source)
        return;
    pbox_app_music_stop(policy);
    pboxData->inputDevice = source;
    switch(source) {
        case SRC_BT: {
            pbox_app_show_bt_state(pboxBtSinkdata->btState, policy);
            if(isInputSourceConnected(SRC_BT)) {
                pbox_app_restart_passive_player(SRC_BT, false, policy);
            } else {
                pbox_app_show_tack_info(" ", " ", policy);
            }
        } break;
#if ENABLE_AUX
        case SRC_AUX: {
            pbox_app_restart_passive_player(SRC_AUX, false, policy);
        } break;
#endif
#if ENABLE_EXT_MCU_USB
        case SRC_USB: {
            pbox_app_restart_passive_player(SRC_USB, false, policy);
        } break;
#else
        case SRC_USB: {
            pbox_app_show_usb_state(pboxUsbdata->usbState, policy);
            if(isInputSourceConnected(SRC_USB)) {
                pbox_app_usb_list_update(pboxTrackdata->track_id, policy);
            } else {
                pbox_app_show_tack_info(" ", " ",  policy);
            }
        } break;
#endif
#if ENABLE_UAC
        case SRC_UAC: {
            pboxUIdata->play_status = pboxUacdata->state;
            pbox_app_show_playingStatus(pboxUacdata->state, policy);
            pbox_multi_displayUacState(UAC_ROLE_SPEAKER, pboxUacdata->state, policy);
            pbox_app_show_tack_info(" ", " ",  policy);
            //pbox_app_uac_restart();
            pbox_app_restart_passive_player(SRC_UAC, false, policy);
        } break;
#endif
    }
    //no ui display now
}

void pbox_app_music_pause(display_t policy)
{
    if(pboxUIdata->play_status != PLAYING)
        return;
    switch (pboxData->inputDevice) {
        case SRC_BT: {
            if (isBtA2dpConnected()) {
                pbox_btsink_playPause(false);
            }
        } break;

        case SRC_USB: {
        } break;
#if ENABLE_UAC
        case SRC_UAC: {
        } break;
#endif
    }

    pbox_app_rockit_pause_player(pboxData->inputDevice);
    pbox_multi_displayIsPlaying(false, policy);
    pboxUIdata->play_status = _PAUSE;
}

void pbox_app_music_trackid(uint32_t id, display_t policy) {
    ALOGD("%s, id:%d\n", __func__, id);
    pboxTrackdata->track_id = id;
}

void pbox_app_music_start(display_t policy) {
    switch (pboxData->inputDevice) {
        case SRC_BT: {
            if (isBtA2dpConnected()) {
                pbox_btsink_playPause(true);
            }
        } break;

        case SRC_USB: {
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
            pbox_app_music_set_volume(pboxUIdata->mainVolumeLevel, policy);
            pbox_multi_displayTrackInfo(track_name, NULL, policy);
        } break;
#if ENABLE_UAC
        case SRC_UAC: {
            pbox_multi_displayTrackInfo("", NULL, policy);
            pbox_app_restart_passive_player(SRC_UAC, true, policy);
        } break;
        default:
#endif
            break;
    }
}

void pbox_app_music_resume(display_t policy) {
    //pbox_app_music_stop(policy);
    switch (pboxData->inputDevice) {
        case SRC_BT: {
            if (isBtA2dpConnected()&&(!isBtA2dpStreaming())) {
                pbox_btsink_playPause(true);
            }
        } break;

        case SRC_USB: {
            if (pboxTrackdata->track_num == 0) {
                return;
            }
            pbox_app_music_start(policy);
            pbox_app_rockit_get_player_duration(SRC_USB);
        } break;
#if ENABLE_UAC
        case SRC_UAC: {
            pbox_app_music_start(policy);
        } break;
#endif
    }
    pboxUIdata->play_status = PLAYING;
    pbox_multi_displayIsPlaying(true, policy);
}

void pbox_app_music_stop(display_t policy)
{
     ALOGD("%s\n", __func__);
    if (pboxUIdata->play_status == IDLE || pboxUIdata->play_status == _STOP) {
        return;
    }
    switch (pboxData->inputDevice) {
        case SRC_BT: {
            if (isBtA2dpConnected())
                pbox_btsink_a2dp_stop();
            pbox_app_rockit_stop_player(SRC_BT);
        } break;
#if ENABLE_AUX
        case SRC_AUX: {
            pbox_app_rockit_stop_player(SRC_BT);
        } break;
#endif
        case SRC_USB: {
#if ENABLE_EXT_MCU_USB
            pbox_app_rockit_stop_player(SRC_BT);
#else
            pbox_app_rockit_stop_player(SRC_USB);
#endif
        } break;
#if ENABLE_UAC
        case SRC_UAC: {
            pbox_app_rockit_stop_player(SRC_UAC);
        } break;
#endif
        default:
        break;
    }

    pbox_multi_displayIsPlaying(false, policy);
    pboxUIdata->play_status = _STOP;
}

void pbox_app_music_set_volume(float volume, display_t policy) {
    ALOGD("%s main volume: %f\n", __func__, volume);
    pboxUIdata->mainVolumeLevel = volume;
    pbox_app_rockit_set_player_volume(pboxData->inputDevice, volume);
    pbox_multi_displayMainVolumeLevel(volume, policy);
}

void pbox_app_music_set_music_volume(float volume, display_t policy) {
    ALOGD("%s music volume: %f\n", __func__, volume);
    pboxUIdata->musicVolumeLevel = volume;
    pbox_app_rockit_set_music_volume(pboxData->inputDevice, volume);
    pbox_multi_displayMusicVolumeLevel(volume, policy);
}

void pbox_app_music_album_next(bool next, display_t policy)
{
    ALOGD("%s, next:%d, inputDevice:%d\n", __func__, next, pboxData->inputDevice);
    switch (pboxData->inputDevice) {
        case SRC_BT: {
            if (isBtA2dpConnected()) {
                pbox_app_rockit_set_player_volume(SRC_BT, MIN_MAIN_VOLUME);
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

        case SRC_USB: {
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
                pbox_multi_displayTrackInfo(pboxTrackdata->track_list[*pId].title, NULL,  DISP_All);
            }

            if(pboxUIdata->play_status == PLAYING) {
                pbox_app_music_stop(policy);
                pbox_app_music_resume(policy);
            }

        } break;
#if ENABLE_UAC
        case SRC_UAC: {
            char text[32] = {0};
            snprintf(text, 31, "UAC NO SUPPORT:%s !!!", next? "NEXT":"PREV");
            pbox_multi_displayTrackInfo(text, NULL,  DISP_All);
        } break;
#endif
        default:
        break;
    }
}

int nomal_mode_volume = 0;

void pbox_app_music_original_singer_open(bool orignal, display_t policy)
{
    bool seperate = !orignal;
    uint32_t hlevel = pboxUIdata->humanLevel;
    uint32_t alevel = pboxUIdata->accomLevel;
    uint32_t rlevel = pboxUIdata->reservLevel;

    pboxUIdata->vocalSplit = !orignal;
    pbox_app_rockit_set_player_seperate(pboxData->inputDevice, seperate , hlevel, alevel, rlevel);
    pbox_multi_displayMusicSeparateSwitch(seperate , hlevel, alevel, rlevel, policy);
}

//album mode: shuffle, sequence, repeat, repeat one.....
void pbox_app_music_album_loop(uint32_t mode, display_t policy) {

}

void pbox_app_music_seek_position(uint32_t dest, uint32_t duration, display_t policy) {
    if (isBtA2dpConnected())
        return;

    pbox_app_rockit_set_player_seek(pboxData->inputDevice, dest);
    pbox_multi_displayTrackPosition(false, dest, duration, policy);
}

void pbox_app_music_set_mic_all(uint32_t index, mic_state_t micdata, display_t policy) {
    pboxUIdata->micData[index] = micdata;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_ALL, micdata);
    pbox_multi_displayMicVolumeLevel(index, micdata.micVolume, policy);
    pbox_multi_displayMicMux(index, micdata.micMux, policy);
    pbox_multi_displayMicMute(index, micdata.micmute, policy);
    pbox_multi_displayRevertMode(index, micdata.reverbMode, policy);
    pbox_multi_displayEcho3A(index, micdata.echo3a, policy);
    pbox_multi_displayMicBass(index, micdata.micBass, policy);
    pbox_multi_displayMicTreble(index, micdata.micTreble, policy);
    pbox_multi_displayMicReverb(index, micdata.micReverb, policy);
}

void pbox_app_music_set_mic_volume(uint32_t index, float volume, display_t policy) {
    pboxUIdata->micData[index].micVolume = volume;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_VOLUME, pboxUIdata->micData[index]);
    pbox_multi_displayMicVolumeLevel(index, volume, policy);
}

void pbox_app_music_set_mic_mute(uint8_t index, bool mute, display_t policy){
    pboxUIdata->micData[index].micmute = mute;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_MUTE, pboxUIdata->micData[index]);
    pbox_multi_displayMicMute(index, mute, policy);
}

void pbox_app_music_init(void) {
    pbox_app_music_set_volume(pboxUIdata->mainVolumeLevel, DISP_All);
    pbox_app_music_set_music_volume(pboxUIdata->musicVolumeLevel, DISP_All);
    pbox_app_music_set_accomp_music_level(pboxUIdata->accomLevel, DISP_All);
    pbox_app_music_set_human_music_level(pboxUIdata->humanLevel, DISP_All);
    pbox_app_music_original_singer_open(!(pboxUIdata->vocalSplit), DISP_All);
    #if ENABLE_USE_SOCBT
        pbox_app_music_set_placement(pboxUIdata->placement, DISP_All);
        pbox_app_music_set_stereo_mode(pboxUIdata->stereo, DISP_All);
        pbox_app_switch_to_input_source(SRC_BT, DISP_All);
    #endif
}

void pbox_app_music_mics_init(display_t policy) {
    for (int index = 0; index < MIC_NUM; index++) {
        //varaible init func have moved to pbox_app_ui_load()
        pbox_app_music_set_mic_all(index, pboxUIdata->micData[index], policy);
    }
}

void pbox_app_music_set_mic_mux(uint8_t index, mic_mux_t mux, display_t policy) {
    pboxUIdata->micData[index].micMux = mux;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_MUX, pboxUIdata->micData[index]);
    pbox_multi_displayMicMux(index, mux, policy);
}

void pbox_app_music_set_mic_treble(uint8_t index, float treble, display_t policy) {
    pboxUIdata->micData[index].micTreble = treble;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_TREBLE, pboxUIdata->micData[index]);
    pbox_multi_displayMicTreble(index, treble, policy);
}

void pbox_app_music_set_mic_bass(uint8_t index, float bass, display_t policy) {
    pboxUIdata->micData[index].micBass = bass;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_BASS, pboxUIdata->micData[index]);
    pbox_multi_displayMicBass(index, bass, policy);
}

void pbox_app_music_set_mic_reverb(uint8_t index, float reverb, display_t policy) {
    pboxUIdata->micData[index].micReverb = reverb;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_REVERB, pboxUIdata->micData[index]);
    pbox_multi_displayMicReverb(index, reverb, policy);
}

void pbox_app_music_set_accomp_music_level(uint32_t volume, display_t policy) {
    //uint32_t alevel = pboxUIdata->accomLevel;
    uint32_t hlevel = pboxUIdata->humanLevel;
    uint32_t rlevel = pboxUIdata->reservLevel;
    bool seperate = pboxUIdata->vocalSplit;

    ALOGD("%s hlevel: %d, alevel: %d, seperate:%d\n", __func__, hlevel, volume, seperate);
    pboxUIdata->accomLevel = volume;
    pbox_app_rockit_set_player_seperate(pboxData->inputDevice, seperate, hlevel, volume, rlevel);
    //pbox_multi_displayMusicSeparateSwitch(seperate, hlevel, volume, rlevel, policy);
    pbox_multi_displayAccompMusicLevel(volume, policy);
}

void pbox_app_music_set_human_music_level(uint32_t volume, display_t policy) {
    uint32_t alevel = pboxUIdata->accomLevel;
    //uint32_t hlevel = pboxUIdata->humanLevel;
    uint32_t rlevel = pboxUIdata->reservLevel;
    bool seperate = pboxUIdata->vocalSplit;

    ALOGD("%s hlevel: %d, alevel: %d, seperate:%d\n", __func__, volume, alevel, seperate);
    pboxUIdata->humanLevel = volume;
    pbox_app_rockit_set_player_seperate(pboxData->inputDevice, seperate, volume, alevel, rlevel);
    //pbox_multi_displayMusicSeparateSwitch(seperate, volume, alevel, rlevel, policy);
    pbox_multi_displayHumanMusicLevel(volume, policy);
}

void pbox_app_music_set_reserv_music_level(uint32_t volume, display_t policy) {
    uint32_t alevel = pboxUIdata->accomLevel;
    uint32_t hlevel = pboxUIdata->humanLevel;
    //uint32_t rlevel = pboxUIdata->reservLevel;
    bool seperate = pboxUIdata->vocalSplit;

    ALOGD("%s hlevel: %d, alevel: %d, rlevel:%d, seperate:%d\n", __func__, hlevel, alevel, volume, seperate);
    pboxUIdata->reservLevel = volume;
    pbox_app_rockit_set_player_seperate(pboxData->inputDevice, seperate, hlevel, alevel, volume);
    //pbox_multi_displayMusicSeparateSwitch(seperate, hlevel, alevel, volume, policy);
    pbox_multi_displayReservLevel(volume, policy);
}

void pbox_app_music_set_echo_3a(uint8_t index, bool enable, display_t policy) {
    pboxUIdata->micData[index].echo3a = enable;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_ECHO_3A, pboxUIdata->micData[index]);
    pbox_multi_displayEcho3A(index, enable, policy);
}

void pbox_app_music_set_recoder_revert(uint8_t index, pbox_revertb_t reverbMode, display_t policy) {
    pboxUIdata->micData[index].reverbMode = reverbMode;
    pbox_app_rockit_set_mic_data(index, MIC_SET_DEST_REVERB_MODE, pboxUIdata->micData[index]);
    pbox_multi_displayRevertMode(index, reverbMode, policy);
}

void pbox_app_music_set_stereo_mode(stereo_mode_t stereo, display_t policy) {
    ALOGD("%s :%d\n", __func__, stereo);
    pboxUIdata->stereo = stereo;
    pbox_app_rockit_set_stereo_mode(pboxData->inputDevice, stereo);
    pbox_multi_displayMusicStereoMode(stereo, policy);
}

void pbox_app_music_set_outdoor_mode(inout_door_t outdoor, display_t policy) {
    ALOGD("%s :%d\n", __func__, outdoor);
    pboxUIdata->outdoor = outdoor;
    pbox_app_rockit_set_outdoor_mode(pboxData->inputDevice, outdoor);
    pbox_multi_displayMusicOutdoorMode(outdoor, policy);
}

void pbox_app_music_set_placement(placement_t place, display_t policy) {
    ALOGD("%s :%d\n", __func__, place);
    pboxUIdata->placement = place;
    pbox_app_rockit_set_placement(pboxData->inputDevice, place);
    pbox_multi_displayMusicPlaceMode(place, policy);
}

void pbox_app_tunning_init(display_t policy) {
    pbox_app_rockit_init_tunning();
}

void pbox_app_music_volume_up(display_t policy) {
    float volume = pboxUIdata->mainVolumeLevel;
    volume += (MAX_MAIN_VOLUME-MIN_MAIN_VOLUME)/10;
    volume = volume> MAX_MAIN_VOLUME?MAX_MAIN_VOLUME:volume;
    volume = volume< MIN_MAIN_VOLUME?MIN_MAIN_VOLUME:volume;

    ALOGD("%s volume up:%f, mainVol:%f\n", __func__, volume, pboxUIdata->mainVolumeLevel);
    if(pboxUIdata->mainVolumeLevel != volume)
    pbox_app_music_set_volume(volume, policy);

    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) 
        pbox_app_music_resume(policy);
}

void pbox_app_music_volume_down(display_t policy) {
    float volume = pboxUIdata->mainVolumeLevel;
    volume -= (MAX_MAIN_VOLUME-MIN_MAIN_VOLUME)/10;
    volume = volume> MAX_MAIN_VOLUME?MAX_MAIN_VOLUME:volume;
    volume = volume< MIN_MAIN_VOLUME?MIN_MAIN_VOLUME:volume;

    ALOGD("%s volume down:%f\n", __func__, volume);
    pbox_app_music_set_volume(volume, policy);

    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) 
        pbox_app_music_resume(policy);
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
#if ENABLE_UAC
    ALOGD("%s\n", __func__);
    switch (role) {
        case UAC_ROLE_SPEAKER: {
            if(pboxUacdata->state == start)
                    return;
            ALOGD("%s player start=%d\n", __func__, start);
            pboxUacdata->state = start;

            if (start && is_dest_source_switchable(SRC_UAC, AUTO)) {
                pbox_app_switch_to_input_source(SRC_UAC, policy);
            }

            if(!is_input_source_selected(SRC_UAC, ANY)) {
                return;
            }

            pboxUIdata->play_status = start;
            pbox_app_drive_passive_player(SRC_UAC, start? PLAYING:_STOP, policy);
            pbox_multi_displayUacState(role, start, policy);
        } break;

        case UAC_ROLE_RECORDER: {
            if(pboxUacdata->record_state == start)
                return;
            ALOGD("%s recorder start=%d\n", __func__, start);
            pboxUacdata->record_state = start;

            pbox_app_record_start(SRC_UAC, start, policy);
        }

        default: break;
    }
#endif
}

void pbox_app_uac_freq_change(uac_role_t role, uint32_t freq, display_t policy) {
#if ENABLE_UAC
    if(!is_input_source_selected(SRC_UAC, ANY)) {
        return;
    }

    ALOGD("%s %s freq:%d\n", __func__, (role==UAC_ROLE_SPEAKER)?"spk":"rec", freq);
    if(pboxUacdata->freq != freq) {
        pboxUacdata->freq = freq;
        pbox_app_rockit_start_audiocard_player(SRC_UAC, freq, 2, AUDIO_CARD_UAC);
    }
#endif
}

void pbox_app_uac_volume_change(uac_role_t role, uint32_t volume, display_t policy) {
#if ENABLE_UAC
    if(!is_input_source_selected(SRC_UAC, ANY)) {
        return;
    }

    pboxUIdata->mainVolumeLevel = PERCENT2TARGET((float)volume, MIN_MAIN_VOLUME, MAX_MAIN_VOLUME);
    ALOGD("%s volume:%d->%f\n", __func__, volume, pboxUIdata->mainVolumeLevel);
    if((role == UAC_ROLE_SPEAKER)) {
        pbox_app_rockit_set_uac_volume(role, pboxUIdata->mainVolumeLevel);
        pbox_multi_displayUacVolume(role, volume, policy);
    }
#endif
}

void pbox_app_uac_mute_change(uac_role_t role, bool mute, display_t policy) {
#if ENABLE_UAC
    if(!is_input_source_selected(SRC_UAC, ANY)) {
        return;
    }

    pbox_app_rockit_set_mute(role, mute);
    pbox_multi_displayUacMute(role, mute, policy);
#endif
}

void pbox_app_uac_ppm_change(uac_role_t role, int32_t ppm, display_t policy) {
#if ENABLE_UAC
    if(!is_input_source_selected(SRC_UAC, ANY)) {
        return;
    }

    pbox_app_rockit_set_ppm(role, ppm);
    pbox_multi_displayUacPpm(role, ppm, policy);
#endif
}

void pbox_app_usb_start_scan(display_t policy) {
    pbox_app_usb_startScan();
    //nothing to display now
}

void pbox_app_show_usb_state(usb_state_t state, display_t policy) {
    //nothing to notify rockit
    pbox_multi_displayUsbState(state, policy);
}

void pbox_app_usb_list_update(uint32_t trackId, display_t policy) {
    //nothing to notify rockit
    pbox_multi_displayUsbListupdate(trackId, policy);
}

void pbox_app_btsoc_get_dsp_version(display_t policy) {
    pbox_app_btsoc_reply_dsp_version("v1.00");
    //nothing to notify rockit
    //nothing to do with ui
}

void pbox_app_btsoc_get_volume(display_t policy) {
    pbox_app_btsoc_reply_main_volume(pboxUIdata->mainVolumeLevel);
    //nothing to notify rockit
    //nothing to do with ui
}

void pbox_app_btsoc_set_volume(float volume, display_t policy) {
    if(pboxUIdata->mainVolumeLevel != volume)
    pbox_app_music_set_volume(volume, policy);
}

void pbox_app_btsoc_set_music_volume(float volume, display_t policy) {
    if(pboxUIdata->musicVolumeLevel != volume)
    pbox_app_music_set_music_volume(volume, policy);
}

void pbox_app_btsoc_set_placement(placement_t placement, display_t policy) {
    if(pboxUIdata->placement != placement)
    pbox_app_music_set_placement(placement, policy);
}

void pbox_app_btsoc_get_placement(display_t policy) {
    pbox_app_btsoc_reply_placement(pboxUIdata->placement);
    //nothing to notify rockit
    //nothing to do with ui
}

void pbox_app_btsoc_get_mic1_state(display_t policy) {
}

void pbox_app_btsoc_get_mic2_state(display_t policy) {
}

void pbox_app_btsoc_set_outdoor_mode(inout_door_t inout, display_t policy) {
    if(pboxUIdata->outdoor != inout)
    pbox_app_music_set_outdoor_mode(inout, policy);
}

void pbox_app_btsoc_get_inout_door(display_t policy) {
    pbox_app_btsoc_reply_inout_door(0);
}

void pbox_app_btsoc_get_poweron(display_t policy) {
    pbox_app_btsoc_reply_poweron(true);
}

void pbox_app_btsoc_set_stereo_mode(stereo_mode_t mode, display_t policy) {
    if(pboxUIdata->stereo != mode)
    pbox_app_music_set_stereo_mode(mode, policy);
}

void pbox_app_btsoc_get_stereo_mode(display_t policy) {
    pbox_app_btsoc_reply_stereo_mode(MODE_STEREO);
}

void pbox_app_btsoc_get_human_voice_fadeout(display_t policy) {
    //pbox_app_btsoc_reply_human_voice_fadeout(pboxUIdata->fadeout);
}

void pbox_app_btsoc_set_mic_data(mic_data_t data, display_t policy) {
    uint8_t index = data.index;
    mic_set_kind_t kind = data.kind;
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
    if(pboxUIdata->vocalSplit != fadeout)
    pbox_app_music_original_singer_open(fadeout? false: true, policy);
}

void pbox_app_btsoc_set_mic_mux(uint8_t index, mic_mux_t micMux, display_t policy) {
    if(pboxUIdata->micData[index].micMux != micMux)
    pbox_app_music_set_mic_mux(index, micMux, policy);
}

void pbox_app_btsoc_get_input_source(display_t policy) {
    pbox_app_btsoc_reply_input_source_with_playing_status(pboxData->inputDevice, pboxUIdata->play_status);
}

void pbox_app_btsoc_set_input_source(input_source_t source, play_status_t status, display_t policy) {
    if(pboxData->inputDevice != source)
    pbox_app_switch_to_input_source(source, policy);

    if(pboxUIdata->play_status != status)
    pbox_app_drive_passive_player(source, status, policy);
}

void pbox_app_music_get_music_volume(display_t policy) {
    pbox_app_btsoc_reply_music_volume(pboxUIdata->musicVolumeLevel);
}