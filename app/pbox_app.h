#ifndef _PTBOX_APP_H_
#define _PTBOX_APP_H_

#include <stdint.h>
#include <stdbool.h>
#include "pbox_common.h"
#include "rk_btsink.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TRACK_MAX_NUM 30

typedef enum {
    DISP_NONE = 0,
    DISP_LED = 1<<0,
    DISP_LCD = 1<<1,
    DISP_All = DISP_LED|DISP_LCD
} display_t;

typedef struct {
    char *title;
    char *artist;
    char *genre;
    uint32_t duration;
} music_track_t;

typedef struct {
    struct _pbox_btsink {
        char localAddr[6];
        char remoteAddr[6];
        btsink_state_t btState;
        btsink_ad2p_state_t a2dpState;
        int pcmSampeFreq;
        int pcmChannel;
        int volume; //[0-10], map to 0~100 or 0~128
        int discoverable;
    } btsink;

    struct _pbox_track {
        uint32_t track_num;
        uint32_t track_id;
        music_track_t track_list[TRACK_MAX_NUM];
    } track;

    struct _pbox_ui {
        uint32_t mVolumeLevel;
        uint32_t mMicVolumeLevel;
        uint32_t mMusicLevel;
        uint32_t mHumanLevel;
        uint32_t mReservLevel;
        bool mEchoReductionEnable;
        bool mVocalSeperateEnable;
        bool echo3A;
        pbox_revertb_t reverbMode;
        play_status_t play_status;
        play_status_t play_status_prev;
    } ui;
} pbox_data_t;

extern struct _pbox_btsink *const pboxBtSinkdata;
extern struct _pbox_ui *const pboxUIdata;

void pbox_app_music_pause(display_t policy);
void pbox_app_music_start(display_t policy);
void pbox_app_music_resume(display_t policy);
void pbox_app_music_stop(display_t policy);
void pbox_app_music_set_volume(uint32_t volume, display_t policy);
void pbox_app_music_volume_up(display_t policy);
void pbox_app_music_volume_down(display_t policy);
void pbox_app_music_album_next(bool next, display_t policy);
uint32_t pbox_app_music_switch_mode(uint32_t mode, display_t policy);
void pbox_app_music_original_singer_open(bool original, display_t policy);
void pbox_app_music_album_loop(uint32_t mode, display_t policy);
void pbox_app_music_seek_position(uint32_t dest, uint32_t duration, display_t policy);

#ifdef __cplusplus
}
#endif

#endif