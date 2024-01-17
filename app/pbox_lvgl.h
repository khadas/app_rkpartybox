#ifndef _PBOX_LVGL_H
#define _PBOX_LVGL_H
#include <stdbool.h>
#include <stdint.h>
#include "pbox_common.h"
#include "rk_btsink.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    //command
    PBOX_LCD_DISP_PLAY_PAUSE = 1,
    PBOX_LCD_DISP_PREV_NEXT,
    PBOX_LCD_DISP_TRACK_INFO,
    PBOX_LCD_DISP_TRACK_POSITION,
    PBOX_LCD_DISP_USB_LIST_UPDATE,
    PBOX_LCD_DISP_USB_STATE,
    PBOX_LCD_DISP_BT_STATE,
    PBOX_LCD_DISP_UAC_STATE,
    PBOX_LCD_DISP_MAIN_VOL_LEVEL,
    PBOX_LCD_DISP_MIC_VOL_LEVEL,
    PBOX_LCD_DISP_MIC_MUTE,
    PBOX_LCD_DISP_ACCOMP_MUSIC_LEVEL,
    PBOX_LCD_DISP_HUMAN_MUSIC_LEVEL,
    PBOX_LCD_DISP_MUSIC_SEPERATE_SWITCH,
    PBOX_LCD_DISP_ECHO_3A_SWITCH,
    PBOX_LCD_DISP_REVERT_MODE,
    PBOX_LCD_DISP_LOOP_MODE,
    PBOX_LCD_DISP_ENERGY_INFO,
    PBOX_LCD_DISP_RESERV_LEVEL,
    PBOX_LCD_DISP_REFLASH,


    //event
    PBOX_LCD_PLAY_PAUSE_EVT = 0x100,
    PBOX_LCD_PLAY_STOP_EVT,
    PBOX_LCD_PLAY_TRACKID_EVT,
    PBOX_LCD_PREV_NEXT_EVT,
    PBOX_LCD_LOOP_MODE_EVT,//260
    PBOX_LCD_SEEK_POSITION_EVT,
    PBOX_LCD_MAIN_VOL_LEVEL_EVT,//262
    PBOX_LCD_MIC_VOL_LEVEL_EVT,
    PBOX_LCD_ACCOMP_MUSIC_LEVEL_EVT,//surroundings/environment sound
    PBOX_LCD_HUMAN_MUSIC_LEVEL_EVT,
    PBOX_LCD_SEPERATE_SWITCH_EVT,
    PBOX_LCD_ECHO_3A_EVT,
    PBOX_LCD_REVERT_MODE_EVT,
    PBOX_LCD_RESERV_MUSIC_LEVEL_EVT,
} pbox_lcd_opcode_t;

typedef struct {
    pbox_msg_t type;
    pbox_lcd_opcode_t msgId;
    union {
        bool play;
        bool next;
        bool loop;
        struct {
            char title[MAX_APP_NAME_LENGTH + 1];
            char artist[MAX_APP_NAME_LENGTH + 1];
        } track;
        uint32_t        trackId;
        uint32_t        mainVolume;
        uint32_t        micVolume;
        uint32_t        accomp_music_level;//surroundings/environment sound level
        uint32_t        human_music_level;
        uint32_t        reserv_music_level;
        pbox_revertb_t      reverbMode;
        pbox_vocal_t        vocalSeparate;
        bool                echo3A_On;
        bool                enable;
        bool                micmute;
        struct {
            bool onlyDuration;
            uint32_t mCurrent;
            uint32_t mDuration;
        } positions;
        energy_info_t energy_data;
        usb_state_t usbState;
        btsink_state_t btState;
        bool uac_start;
    };
} pbox_lcd_msg_t;

int pbox_create_lvglTask(void);
void lcd_pbox_notifyPlayPause(bool play);
void lcd_pbox_notifyPlayStop();
void lcd_pbox_notifyTrackid(uint32_t trackId);
void lcd_pbox_notifyPrevNext(bool next);
void lcd_pbox_notifyLoopMode(bool loop);
void lcd_pbox_notifySeekPosition(unsigned int mCurrent, unsigned int mDuration);
void lcd_pbox_notifyMainVolLevel(uint32_t mainVolume);
void lcd_pbox_notifyMicVolLevel(uint32_t micVolume);
void lcd_pbox_notifyAccompMusicLevel(uint32_t accomp_music_level);
void lcd_pbox_notifyHumanMusicLevel(uint32_t human_music_level);
void lcd_pbox_notifySeparateSwitch(bool enable);
void lcd_pbox_notifyEcho3A(bool echo3A_On);
void lcd_pbox_notifyReverbMode(pbox_revertb_t reverbMode);
void lcd_pbox_notifyReservMusicLevel(uint32_t reserv_music_level);
#ifdef __cplusplus
}
#endif
#endif
