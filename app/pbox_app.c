#include <stdint.h>
#include <stdbool.h>
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
        .mVolumeLevel = 50,
        .mMicVolumeLevel = 100,
        .mMusicLevel = 100,
        .mHumanLevel = 15,
        .mReservLevel = 100,
        .mEchoReductionEnable = true,
        .mVocalSeperateEnable = false,
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
    //if (state == BT_DISCONNECT) {
    //    pbox_multi_displayUsbListupdate(pboxTrackdata->track_id, policy);
    //}
}

void pbox_app_show_playingStatus(bool play, display_t policy) {
    //nothing to notify rockit
    pbox_multi_displayIsPlaying(play, policy);
}

void pbox_app_restart_passive_player(input_source_t source, bool restart, display_t policy) {
    if(!is_input_source_selected(source, ANY)) {
        return;
    }

    if(restart) {
        pbox_app_rockit_stop_BTplayer();
    }

    switch(source) {
        case SRC_BT: {
            pbox_app_rockit_start_BTplayer(pboxBtSinkdata->pcmSampeFreq, pboxBtSinkdata->pcmChannel, AUDIO_CARD_BT);
        } break;
#if ENABLE_UAC
        case SRC_UAC: {
            pbox_app_rockit_start_BTplayer(pboxUacdata->freq, 2, AUDIO_CARD_UAC);
        }
#endif
#if ENABLE_EXT_MCU_USB
        case SRC_USB: {
            pbox_app_rockit_start_BTplayer(48000, 2, AUDIO_CARD_USB);
        }
#endif
#if ENABLE_AUX
        case SRC_AUX: {
            pbox_app_rockit_start_BTplayer(48000, 2, AUDIO_CARD_AUX);
        }
#endif
    }
}

//like BT, uac, or Usb connected with extern MCU, these are passive input source.
//for the audio stream are streamed to rockchips ics.
//another way, USB connected to rockchips was not passive source. for we play USB locally and actively.
void pbox_app_drive_passive_player(input_source_t source, play_status_t status, display_t policy) {
    printf("%s, play status [%d->%d]\n", __func__, pboxUIdata->play_status, status);
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

void pbox_app_music_stop_bt_player(display_t policy) {
    pbox_app_rockit_stop_BTplayer();
    //nothing to do with ui
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
    printf("%s, source: [%d->%d]\n", __func__, pboxData->inputDevice, source);
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
            pbox_multi_displayUacState(UAC_ROLE_SPEAKER, false, policy);
            pbox_app_show_tack_info(" ", " ",  policy);
            pbox_app_uac_restart();
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

    pbox_app_rockit_pause_player();
    pbox_multi_displayIsPlaying(false, policy);
    pboxUIdata->play_status = _PAUSE;
}

void pbox_app_music_trackid(uint32_t id, display_t policy) {
    printf("%s, id:%d\n", __func__, id);
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
            printf("pboxTrackdata->track_id:%d, track_num:%d\n", pboxTrackdata->track_id, pboxTrackdata->track_num);
            for (int i=0 ; i< pboxTrackdata->track_num; i++) {
                printf("pboxTrackdata->track_list[%d]:%s\n", i, pboxTrackdata->track_list[i].title);
            }
            if(pboxTrackdata->track_num == 0) {
                return;
            }
            char *track_name = pbox_app_usb_get_title(pboxTrackdata->track_id);
            sprintf(track_uri, MUSIC_PATH"%s", track_name);
            printf("play track [%s]\n", track_uri);
            pbox_app_rockit_set_datasource(track_uri, NULL);
            pbox_multi_displayTrackInfo(track_name, NULL, policy);
            pbox_app_rockit_start_player();
        } break;
#if ENABLE_UAC
        case SRC_UAC: {
            pbox_multi_displayTrackInfo("", NULL, policy);
            //pbox_app_rockit_stop_BTplayer();
        } break;
        default:
#endif
            break;
    }
}

void pbox_app_music_resume(display_t policy) {
    //pbox_app_music_stop(policy);
    //pbox_app_music_stop_bt_player(policy);
    switch (pboxData->inputDevice) {
        case SRC_BT: {
            if (isBtA2dpConnected()) {
                pbox_btsink_playPause(true);
            }
        } break;

        case SRC_USB: {
            if (pboxTrackdata->track_num == 0) {
                return;
            }
            pbox_app_music_start(policy);
            pbox_app_rockit_get_player_duration();
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
    if (pboxUIdata->play_status == IDLE || pboxUIdata->play_status == _STOP) {
        return;
    }
    switch (pboxData->inputDevice) {
        case SRC_BT: {
            if (isBtA2dpConnected())
                pbox_btsink_a2dp_stop();
            pbox_app_rockit_stop_BTplayer();
        } break;
#if ENABLE_AUX
        case SRC_AUX: {
            pbox_app_rockit_stop_BTplayer();
        } break;
#endif
        case SRC_USB: {
#if ENABLE_EXT_MCU_USB
            pbox_app_rockit_stop_BTplayer();
#else
            pbox_app_rockit_stop_player();
#endif
        } break;
#if ENABLE_UAC
        case SRC_UAC: {
            pbox_app_rockit_stop_BTplayer();
        } break;
#endif
        default:
        break;
    }

    pbox_multi_displayIsPlaying(false, policy);
    pboxUIdata->play_status = _STOP;
}

void pbox_app_music_set_volume(uint32_t volume, display_t policy) {
    printf("%s main volume: %d\n", __func__, volume);
    pboxUIdata->mVolumeLevel = volume;
    pbox_app_rockit_set_player_volume(volume);
    pbox_multi_displayMainVolumeLevel(volume, policy);
}

void pbox_app_music_album_next(bool next, display_t policy)
{
    printf("%s, next:%d, inputDevice:%d\n", __func__, next, pboxData->inputDevice);
    switch (pboxData->inputDevice) {
        case SRC_BT: {
            if (isBtA2dpConnected()) {
                if(next) {
                    pbox_btsink_music_next(true);
                }
                else {
                    pbox_btsink_music_next(false);;
                }
                if(pboxUIdata->play_status == PLAYING) {
                    pbox_app_music_pause(policy);
                    pbox_app_music_resume(policy);
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
    uint32_t hlevel = pboxUIdata->mHumanLevel;
    uint32_t mlevel = pboxUIdata->mMusicLevel;
    uint32_t rlevel = pboxUIdata->mReservLevel;

    pboxUIdata->mVocalSeperateEnable = !orignal;
    pbox_app_rockit_set_player_seperate(seperate , hlevel, mlevel, rlevel);
    pbox_multi_displayMusicSeparateSwitch(seperate , hlevel, mlevel, rlevel, policy);
}

//album mode: shuffle, sequence, repeat, repeat one.....
void pbox_app_music_album_loop(uint32_t mode, display_t policy) {

}
//switch mode: usb/bt/others
uint32_t pbox_app_music_switch_mode(uint32_t mode, display_t policy) {

}

void pbox_app_music_seek_position(uint32_t dest, uint32_t duration, display_t policy) {
    if (isBtA2dpConnected())
        return;

    pbox_app_rockit_set_player_seek(dest);
    pbox_multi_displayTrackPosition(false, dest, duration, policy);
}

void pbox_app_music_set_mic_volume(uint32_t volume, display_t policy) {
    pboxUIdata->mMicVolumeLevel = volume;
    pbox_app_rockit_set_recoder_volume(volume);
    pbox_multi_displayMicVolumeLevel(volume, policy);
}

void pbox_app_music_set_mic_mute(bool mute, display_t policy){
    uint32_t volume;
    pboxUIdata->mMute = mute;

    pbox_app_rockit_set_recoder_mute(mute);
    pbox_multi_displayMicMute(mute, policy);
}

void pbox_app_music_set_accomp_music_level(uint32_t volume, display_t policy) {
    //uint32_t mlevel = pboxUIdata->mMusicLevel;
    uint32_t hlevel = pboxUIdata->mHumanLevel;
    uint32_t rlevel = pboxUIdata->mReservLevel;
    bool seperate = pboxUIdata->mVocalSeperateEnable;

    printf("%s hlevel: %d, mlevel: %d, seperate:%d\n", __func__, hlevel, volume, seperate);
    pboxUIdata->mMusicLevel = volume;
    pbox_app_rockit_set_player_seperate(seperate, hlevel, volume, rlevel);
    pbox_multi_displayMusicSeparateSwitch(seperate, hlevel, volume, rlevel, policy);
}

void pbox_app_music_set_human_music_level(uint32_t volume, display_t policy) {
    uint32_t mlevel = pboxUIdata->mMusicLevel;
    //uint32_t hlevel = pboxUIdata->mHumanLevel;
    uint32_t rlevel = pboxUIdata->mReservLevel;
    bool seperate = pboxUIdata->mVocalSeperateEnable;

    printf("%s hlevel: %d, mlevel: %d, seperate:%d\n", __func__, volume, mlevel, seperate);
    pboxUIdata->mHumanLevel = volume;
    pbox_app_rockit_set_player_seperate(seperate, volume, mlevel, rlevel);
    pbox_multi_displayMusicSeparateSwitch(seperate, volume, mlevel, rlevel, policy);
}

void pbox_app_music_set_reserv_music_level(uint32_t volume, display_t policy) {
    uint32_t mlevel = pboxUIdata->mMusicLevel;
    uint32_t hlevel = pboxUIdata->mHumanLevel;
    //uint32_t rlevel = pboxUIdata->mReservLevel;
    bool seperate = pboxUIdata->mVocalSeperateEnable;

    printf("%s hlevel: %d, mlevel: %d, rlevel:%d, seperate:%d\n", __func__, hlevel, mlevel, volume, seperate);
    pboxUIdata->mReservLevel = volume;
    pbox_app_rockit_set_player_seperate(seperate, hlevel, mlevel, volume);
    pbox_multi_displayMusicSeparateSwitch(seperate, hlevel, mlevel, volume, policy);
}

void pbox_app_music_set_echo_3a(bool enable, display_t policy) {
    pboxUIdata->echo3A = enable;
    pbox_app_rockit_set_recoder_3A(enable);
    pbox_multi_displayEcho3A(enable, policy);
}

void pbox_app_music_set_recoder_revert(pbox_revertb_t reverbMode, display_t policy) {
    pboxUIdata->reverbMode = reverbMode;
    pbox_app_rockit_set_recoder_revert(reverbMode);
    pbox_multi_displayRevertMode(reverbMode, policy);
}

void pbox_app_music_set_stereo_mode(stereo_mode_t stereo, display_t policy) {
    printf("%s :%d\n", __func__, stereo);
    pboxUIdata->stereo = stereo;
    pbox_app_rockit_set_stereo_mode(stereo);
    pbox_multi_displayMusicStereoMode(stereo, policy);
}

void pbox_app_music_set_outdoor_mode(inout_door_t outdoor, display_t policy) {
    printf("%s :%d\n", __func__, outdoor);
    pboxUIdata->outdoor = outdoor;
    pbox_app_rockit_set_outdoor_mode(outdoor);
    pbox_multi_displayMusicOutdoorMode(outdoor, policy);
}

void pbox_app_music_set_placement(placement_t place, display_t policy) {
    printf("%s :%d\n", __func__, place);
    pboxUIdata->placement = place;
    pbox_app_rockit_set_placement(place);
    pbox_multi_displayMusicPlaceMode(place, policy);
}

void pbox_app_music_volume_up(display_t policy) {
    uint32_t *const volume = &pboxUIdata->mVolumeLevel;

    if (*volume <= 5)
        *volume += 5;
    else if (*volume <= 10)
        *volume += 15;
    else if (*volume <= 75)
        *volume += 25;
    else
        *volume = 100;

    printf("%s volume up:%d\n", __func__, *volume);
    pbox_app_music_set_volume(*volume, policy);

    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) 
        pbox_app_music_resume(policy);
}

void pbox_app_music_volume_down(display_t policy) {
    uint32_t *const volume = &pboxUIdata->mVolumeLevel;

    if (*volume >= 50)
        *volume -= 25;
    else if (*volume >= 25) 
        *volume -= 15; 
    else if (*volume >= 5)
        *volume -= 5;
    else
        *volume = 0;

    printf("%s volume down:%d\n", __func__, *volume);
    pbox_app_music_set_volume(*volume, policy);

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

    printf("%s: %04d-%02d-%02d %s\n", __func__, year, mon + 1, day, __TIME__);
}

void pbox_app_uac_state_change(uac_role_t role, bool start, display_t policy) {
#if ENABLE_UAC
    printf("%s\n", __func__);
    if((role == UAC_ROLE_SPEAKER)) {
        if(&pboxUacdata->state == start)
            return;
        printf("%s start=%d\n", __func__, start);
        pboxUacdata->state = start;

        if (start && is_dest_source_switchable(SRC_UAC, AUTO)) {
            pbox_app_switch_to_input_source(SRC_UAC, policy);
        }

        if(!is_input_source_selected(SRC_UAC, ANY)) {
            return;
        }

        pbox_app_drive_passive_player(SRC_UAC, start? PLAYING:_STOP, policy);
        pbox_multi_displayUacState(role, start, policy);
    }
#endif
    ////pbox_app_rockit_set_uac_state(role, start);
}

void pbox_app_uac_freq_change(uac_role_t role, uint32_t freq, display_t policy) {
#if ENABLE_UAC
    if(!is_input_source_selected(SRC_UAC, ANY)) {
        return;
    }
    printf("%s freq:%d\n", __func__, freq);
    if(pboxUacdata->freq != freq) {
        pboxUacdata->freq = freq;
        pbox_app_rockit_start_BTplayer(freq, 2, AUDIO_CARD_UAC);
    }
#endif
}

void pbox_app_uac_volume_change(uac_role_t role, uint32_t volume, display_t policy) {
#if ENABLE_UAC
    if(!is_input_source_selected(SRC_UAC, ANY)) {
        return;
    }

    if((role == UAC_ROLE_SPEAKER)) {
        pbox_app_rockit_set_uac_volume(role, volume);
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
    pbox_app_btsoc_reply_main_volume(pboxUIdata->mVolumeLevel);
    //nothing to notify rockit
    //nothing to do with ui
}

void pbox_app_btsoc_set_placement(placement_t placement, display_t policy) {
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
    pbox_app_music_set_outdoor_mode(inout, policy);
}

void pbox_app_btsoc_get_inout_door(display_t policy) {
    pbox_app_btsoc_reply_inout_door(0);
}

void pbox_app_btsoc_get_poweron(display_t policy) {
    pbox_app_btsoc_reply_poweron(true);
}

void pbox_app_btsoc_set_stereo_mode(stereo_mode_t mode, display_t policy) {
    pbox_app_music_set_stereo_mode(mode, policy);
}

void pbox_app_btsoc_get_stereo_mode(display_t policy) {
    pbox_app_btsoc_reply_stereo_mode(MODE_STEREO);
}

void pbox_app_btsoc_get_human_split(display_t policy) {
    pbox_app_btsoc_reply_human_split(pboxUIdata->mHumanLevel);
}

void pbox_app_btsoc_set_human_split(uint32_t level, display_t policy) {
    printf("%s hlevel: %d\n", __func__, level);
    pbox_app_music_set_human_music_level(level, policy);
}

void pbox_app_btsoc_get_input_source(display_t policy) {
    pbox_app_btsoc_reply_input_source_with_playing_status(pboxData->inputDevice, pboxUIdata->play_status);
}

void pbox_app_btsoc_set_input_source(input_source_t source, play_status_t status, display_t policy) {
    pbox_app_switch_to_input_source(source, policy);
    pbox_app_drive_passive_player(source, status, policy);
}

void pbox_app_music_get_accom_level(display_t policy) {
    pbox_app_btsoc_reply_accom_level(pboxUIdata->mMusicLevel);
}