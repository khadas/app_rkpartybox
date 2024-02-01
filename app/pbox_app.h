#ifndef _PTBOX_APP_H_
#define _PTBOX_APP_H_

#include <stdint.h>
#include <stdbool.h>
#include "pbox_common.h"
#include "rk_btsink.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DISP_NONE = 0,
    DISP_LED = 1<<0,
    DISP_LCD = 1<<1,
    DISP_All = DISP_LED|DISP_LCD
} display_t;

typedef struct {
    char *title;
    char *artist;
    //char *genre;
    uint32_t duration;
} music_track_t;

typedef struct {
    struct _pbox_btsink {
        char localAddr[6];
        char remoteAddr[6];
        char remote_name[MAX_NAME_LENGTH + 1];
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
    usb_disk_info_t usbDisk;
    bool uac_state;
    input_source_t inputDevice;
    struct _pbox_ui {
        uint32_t mVolumeLevel;
        uint32_t mMicVolumeLevel;
        uint32_t mMusicLevel;
        uint32_t mHumanLevel;
        uint32_t mReservLevel;
        bool mEchoReductionEnable;
        bool mVocalSeperateEnable;
        bool echo3A;
        bool mMute;
        pbox_revertb_t reverbMode;
        play_status_t play_status;
        play_status_t play_status_prev;
    } ui;
} pbox_data_t;

extern pbox_data_t *const pboxData;
extern struct _pbox_btsink *const pboxBtSinkdata;
extern struct _pbox_ui *const pboxUIdata;
extern struct _pbox_track *const pboxTrackdata;
extern usb_disk_info_t *const pboxUsbdata;
void pbox_app_show_track_position(bool durationOnly, uint32_t current, uint32_t duration, display_t policy);
void pbox_app_show_tack_info(char *title, char *artist, display_t policy);
void pbox_app_show_bt_state(btsink_state_t state, display_t policy);
void pbox_app_show_playingStatus(bool play, display_t policy);
void pbox_app_restart_bt_player(bool restart, char *cardName, display_t policy);
void pbox_app_music_stop_bt_player(display_t policy);
void pbox_app_bt_pair_enable(bool enable, display_t policy);
void pbox_app_set_vendor_state(bool enable, display_t policy);
void pbox_app_restart_btsink(bool only, display_t policy);
void pbox_app_switch_to_input_source(input_source_t source, display_t policy);
void pbox_app_update_input_source(input_source_t source, display_t policy);
void pbox_app_switch_next_input_source(input_source_t source, display_t policy);
void pbox_app_music_pause(display_t policy);
void pbox_app_music_trackid(uint32_t id, display_t policy);
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
char* pbox_app_usb_get_title(uint32_t trackId);
void pbox_app_music_set_mic_volume(uint32_t volume, display_t policy);
void pbox_app_music_set_mic_mute(bool mute, display_t policy);
void pbox_app_music_set_accomp_music_level(uint32_t volume, display_t policy);
void pbox_app_music_set_human_music_level(uint32_t volume, display_t policy);
void pbox_app_music_set_reserv_music_level(uint32_t volume, display_t policy);
void pbox_app_music_set_echo_3a(bool enable, display_t policy);
void pbox_app_music_set_recoder_revert(pbox_revertb_t reverbMode, display_t policy);
void pbox_version_print(void);

void pbox_app_uac_state_change(uac_role_t role, bool start, display_t policy);
void pbox_app_uac_freq_change(uac_role_t role, uint32_t freq, display_t policy);
void pbox_app_uac_volume_change(uac_role_t role, uint32_t volume, display_t policy);
void pbox_app_uac_mute_change(uac_role_t role, bool mute, display_t policy);
void pbox_app_uac_ppm_change(uac_role_t role, int32_t ppm, display_t policy);
void pbox_app_restart_uac_player(bool restart, display_t policy);
void pbox_app_start_uac_poll(display_t policy);

void pbox_app_usb_list_update(uint32_t trackId, display_t policy);
void pbox_app_usb_state_change(usb_state_t state, display_t policy);
void pbox_app_usb_start_scan(display_t policy);
#ifdef __cplusplus
}
#endif

#endif
