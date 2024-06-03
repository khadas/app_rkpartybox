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
    DISP_BTMCU = 1<<2,
    DISP_FS = 1<<3, //File system
    DISP_All = (DISP_LED|DISP_LCD|DISP_BTMCU),
    DISP_All_EXCLUDE_BTMCU = (DISP_All&(~DISP_BTMCU)),
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

    struct _pbox_uac {
        bool state;
        bool record_state;
        uint32_t freq;
    } uac;

    input_source_t inputDevice;
    uint32_t avail_srcs;
    int32_t volume_resume_time;//it mustbe 10 times of PBOX_TIMER_INTERVAL
    bool trackchanged;
    int stereo_shake;
    int inout_shake;
    int gender_sence;

    struct _pbox_ui {
        placement_t placement;
        stereo_mode_t stereo;
        inout_door_t outdoor;
        equalizer_t  eqmode;
        float mainVolumeLevel;//main volume
        float musicVolumeLevel;//music volume
        mic_state_t micData[MIC_NUM];//mMicVolumeLevel
        uint32_t accomLevel;
        uint32_t humanLevel;
        uint32_t reservLevel;
        bool vocalSplit;
        bool vocallib;
        play_status_t play_status;
        play_status_t play_status_prev;
        bool autoSource;
    } ui;
} pbox_data_t;

extern pbox_data_t *const pboxData;
extern struct _pbox_btsink *const pboxBtSinkdata;
extern struct _pbox_ui *const pboxUIdata;
extern struct _pbox_track *const pboxTrackdata;
extern usb_disk_info_t *const pboxUsbdata;
extern struct _pbox_uac *const pboxUacdata;
extern favor_input_order_t input_order_config[SRC_NUM];

void pbox_app_set_favor_source_order(void);
void pbox_app_restart_passive_player(input_source_t source, bool restart, display_t policy);
void pbox_app_bt_pair_enable(bool enable, display_t policy);
void pbox_app_bt_local_update(display_t policy);
void pbox_app_bt_sink_onoff(bool on, display_t policy);
void pbox_app_set_vendor_state(bool enable, display_t policy);
void pbox_app_restart_btsink(bool only, display_t policy);
void pbox_app_switch_to_input_source(input_source_t source, display_t policy);
void pbox_app_autoswitch_next_input_source(input_source_t source, display_t policy);
void pbox_app_music_pause(display_t policy);
void pbox_app_music_trackid(uint32_t id, display_t policy);
void pbox_app_music_start(display_t policy);
void pbox_app_music_resume(display_t policy);
void pbox_app_music_stop(display_t policy);
void pbox_app_main_volume_up(display_t policy);
void pbox_app_main_volume_down(display_t policy);
void pbox_app_music_volume_up(display_t policy);
void pbox_app_music_volume_down(display_t policy);
void pbox_app_music_album_next(bool next, display_t policy);
void pbox_app_music_original_singer_open(bool original, display_t policy);
void pbox_app_music_album_loop(uint32_t mode, display_t policy);
void pbox_app_music_seek_position(uint32_t dest, uint32_t duration, display_t policy);
char* pbox_app_usb_get_title(uint32_t trackId);
void pbox_app_music_mics_init(display_t policy);
void pbox_app_tunning_init(display_t policy);
void pbox_app_resume_volume_later(int32_t msdelay);

void pbox_app_music_set_main_volume(float volume, display_t policy);
void pbox_app_music_set_music_volume(float volume, display_t policy);
void pbox_app_music_set_accomp_music_level(uint32_t volume, display_t policy);
void pbox_app_music_set_human_music_level(uint32_t volume, display_t policy);
void pbox_app_music_set_reserv_music_level(uint32_t volume, display_t policy);
void pbox_app_music_set_stereo_mode(stereo_mode_t stereo, display_t policy);
void pbox_app_music_set_outdoor_mode(inout_door_t outdoor, display_t policy);
void pbox_app_music_set_placement(placement_t place, display_t policy);
void pbox_app_music_set_eq_mode(equalizer_t mode, display_t policy);
void pbox_app_music_human_vocal_level_cycle(display_t policy);
void pbox_app_music_guitar_vocal_level_cycle(display_t policy);
void pbox_app_switch_vocal_lib(bool vocalib);
void pbox_app_music_play_notice_number(uint8_t number, display_t policy);

void pbox_app_music_set_mic_volume(uint32_t index, float volume, display_t policy);
void pbox_app_music_set_mic_mute(uint8_t index, bool mute, display_t policy);
void pbox_app_music_set_mic_mux(uint8_t index, mic_mux_t mux, display_t policy);
void pbox_app_music_set_echo_3a(uint8_t index, bool enable, display_t policy);
void pbox_app_music_set_recoder_revert(uint8_t index, pbox_revertb_t reverbMode, display_t policy);
void pbox_app_music_set_mic_treble(uint8_t index, float treble, display_t policy);
void pbox_app_music_set_mic_bass(uint8_t index, float bass, display_t policy);
void pbox_app_music_set_mic_reverb(uint8_t index, float reverb, display_t policy);
void pbox_app_music_set_mic_all(uint32_t index, mic_state_t micdata, display_t policy);

void pbox_version_print(void);
void pbox_app_music_init(void);

void pbox_app_uac_state_change(uac_role_t role, bool start, display_t policy);
void pbox_app_uac_freq_change(uac_role_t role, uint32_t freq, display_t policy);
void pbox_app_uac_volume_change(uac_role_t role, uint32_t volume, display_t policy);
void pbox_app_uac_mute_change(uac_role_t role, bool mute, display_t policy);
void pbox_app_uac_ppm_change(uac_role_t role, int32_t ppm, display_t policy);
void pbox_app_start_uac_poll(display_t policy);

void pbox_app_usb_list_update(uint32_t trackId, display_t policy);
void pbox_app_show_usb_state(usb_state_t state, display_t policy);
void pbox_app_usb_start_scan(display_t policy);

void pbox_app_btsoc_init(void);
void pbox_app_btsoc_set_volume(float volume, display_t policy);
void pbox_app_btsoc_set_placement(placement_t placement, display_t policy);
void pbox_app_btsoc_set_outdoor_mode(inout_door_t inout, display_t policy);
void pbox_app_btsoc_set_stereo_mode(stereo_mode_t mode, display_t policy);
void pbox_app_btsoc_set_human_voice_fadeout(bool fadeout, display_t policy);
void pbox_app_btsoc_set_mic_mux(uint8_t index, mic_mux_t micMux, display_t policy);
void pbox_app_btsoc_set_mic_data(mic_data_t data, display_t policy);
void pbox_app_btsoc_set_input_source(input_source_t source, play_status_t status, display_t policy);
void pbox_app_btsoc_set_music_volume(float volume, display_t policy);

void pbox_app_echo_track_position(bool durationOnly, uint32_t current, uint32_t duration, display_t policy);
void pbox_app_echo_tack_info(char *title, char *artist, display_t policy);
void pbox_app_echo_bt_state(btsink_state_t state, display_t policy);
void pbox_app_echo_playingStatus(bool play, display_t policy);

void pbox_app_echo_dsp_version(display_t policy);
void pbox_app_echo_main_volume(display_t policy);
void pbox_app_echo_music_volume(display_t policy);
void pbox_app_echo_placement(display_t policy);
void pbox_app_echo_inout_door(display_t policy);
void pbox_app_echo_poweron_status(display_t policy);
void pbox_app_echo_stereo_mode(display_t policy);
void pbox_app_echo_voice_fadeout_mode(display_t policy);
void pbox_app_echo_micMux(uint8_t index, display_t policy);
void pbox_app_echo_micdata(uint8_t index, mic_set_kind_t kind, display_t policy);
void pbox_app_echo_input_source(display_t policy);


bool is_dest_source_auto_switchable(input_source_t source);
bool is_input_source_selected(input_source_t source, switch_source_t mode);
bool isInputSourceConnected(input_source_t source);
bool is_input_source_automode(void);
bool is_input_source_configed(input_source_t source);
int pbox_app_ui_load(void);

#ifdef __cplusplus
}
#endif

#endif
