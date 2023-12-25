#include <stdint.h>
#include <stdbool.h>
#include "pbox_app.h"
#include "pbox_multi_display.h"
#include "pbox_btsink_app.h"
#include "pbox_rockit_app.h"

pbox_data_t pbox_data = {
    .btsink = {
        .pcmSampeFreq = 44100,
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
    },
    .track= {
        .track_num = 0,
        .track_id = 0,
    }
};

struct _pbox_btsink *const pboxBtSinkdata  = &(pbox_data.btsink);
struct _pbox_ui *const pboxUIdata  = &(pbox_data.ui);
struct _pbox_track *const pboxTrackdata  = &(pbox_data.track);
usb_disk_info_t *const pboxUsbdata  = &(pbox_data.usbDisk);

void pbox_app_music_pause(display_t policy)
{
    if(pboxUIdata->play_status != PLAYING)
        return;

    if(isBtA2dpConnected()) {
        pbox_btsink_playPause(false);
    }

    pbox_app_rockit_pause_player();
    pbox_multi_displayIsPlaying(false, policy);
    pboxUIdata->play_status = _PAUSE;
}

void pbox_app_music_trackid(uint32_t id, display_t policy) {
    pboxTrackdata->track_id = id;
}

void pbox_app_music_start(display_t policy) {
    char *track_name = NULL;
    char track_uri[256];
    if (!isBtA2dpConnected()) {
        printf("pboxTrackdata->track_id:%d, track_num:%d\n", pboxTrackdata->track_id, pboxTrackdata->track_num);
        for (int i=0 ; i< pboxTrackdata->track_num; i++) {
            printf("pboxTrackdata->track_list[%d]:%s\n", i, pboxTrackdata->track_list[i].title);
        }
        track_name = pbox_app_usb_get_title(pboxTrackdata->track_id);
        sprintf(track_uri, MUSIC_PATH"%s", track_name);
        printf("play track [%s]\n", track_uri);
        pbox_app_rockit_set_datasource(track_uri, NULL);
        pbox_multi_displayTrackInfo(track_name, NULL, policy);
    }
    pbox_app_rockit_start_player();
    pbox_multi_displayIsPlaying(true, policy);
}

void pbox_app_music_resume(display_t policy)
{
    if(isBtA2dpConnected()) {
        pbox_btsink_playPause(true);
    }

    if (pboxUIdata->play_status == IDLE || pboxUIdata->play_status == _STOP) {
        pbox_app_music_start(policy);
    }

    pbox_app_rockit_resume_player();
    pbox_app_rockit_get_player_duration();
    pbox_multi_displayIsPlaying(true, policy);
    pboxUIdata->play_status = PLAYING;
}

void pbox_app_music_stop(display_t policy)
{
    if(isBtA2dpConnected()) {
        pbox_btsink_a2dp_stop();
    }

    pbox_app_rockit_stop_player();
    pbox_multi_displayIsPlaying(false, policy);
    pboxUIdata->play_status = _STOP;
}

void pbox_app_music_set_volume(uint32_t volume, display_t policy) {
    pbox_app_rockit_set_player_volume(volume);
    pbox_multi_displayMainVolumeLevel(volume, policy);
}

void pbox_app_music_album_next(bool next, display_t policy)
{
    uint32_t *const pId = &(pboxTrackdata->track_id);

    printf("%s, next:%d\n", __func__, next);
    if (isBtA2dpConnected()) {
        if(next) {
            pbox_btsink_music_next(true);
        }
        else {
            pbox_btsink_music_next(false);;
        }
    }
    else {
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
    }

    if(pboxUIdata->play_status == PLAYING) {
        if(isBtA2dpConnected()) {
            pbox_app_music_pause(policy);
        } else {
            pbox_app_music_stop(policy);
        }
        pbox_app_music_resume(policy);
    }
    else {
	    if(!isBtA2dpConnected()) {
            pbox_app_music_stop(policy);
        }
    }
}

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

    pboxUIdata->mMusicLevel = volume;
    pbox_app_rockit_set_player_seperate(seperate, hlevel, volume, rlevel);
    pbox_multi_displayMusicSeparateSwitch(seperate, hlevel, volume, rlevel, policy);
}

void pbox_app_music_set_human_music_level(uint32_t volume, display_t policy) {
    uint32_t mlevel = pboxUIdata->mMusicLevel;
    //uint32_t hlevel = pboxUIdata->mHumanLevel;
    uint32_t rlevel = pboxUIdata->mReservLevel;
    bool seperate = pboxUIdata->mVocalSeperateEnable;

    pboxUIdata->mHumanLevel = volume;
    pbox_app_rockit_set_player_seperate(seperate, volume, mlevel, rlevel);
    pbox_multi_displayMusicSeparateSwitch(seperate, volume, mlevel, rlevel, policy);
}

void pbox_app_music_set_reserv_music_level(uint32_t volume, display_t policy) {
    uint32_t mlevel = pboxUIdata->mMusicLevel;
    uint32_t hlevel = pboxUIdata->mHumanLevel;
    //uint32_t rlevel = pboxUIdata->mReservLevel;
    bool seperate = pboxUIdata->mVocalSeperateEnable;

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

void pbox_app_music_volume_up(display_t policy) {
    uint32_t *const volume = &pboxUIdata->mVolumeLevel;

    if (*volume <= 5)
        *volume += 5;
    else if (*volume <= 10)
        *volume += 15;
    else if (*volume <= 75)
        *volume += 25;

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

    printf("%s volume down:%d\n", __func__, *volume);
    pbox_app_music_set_volume(*volume, policy);

    if ((pboxUIdata->play_status == _PAUSE) && (pboxUIdata->play_status_prev == PLAYING)) 
        pbox_app_music_resume(policy);
}