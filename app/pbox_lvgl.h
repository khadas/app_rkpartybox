#ifndef _PBOX_LVGL_H
#define _PBOX_LVGL_H
#include <stdbool.h>
#include <stdint.h>
#include "pbox_common.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    //command
    PBOX_LCD_DISP_PLAY_PAUSE = 1,
    PBOX_LCD_DISP_PREV_NEXT,
    PBOX_LCD_DISP_TRACK_INFO,
    PBOX_LCD_DISP_TRACK_POSITION,
    PBOX_LCD_DISP_MAIN_VOL_LEVEL,
    PBOX_LCD_DISP_MIC_VOL_LEVEL,
    PBOX_LCD_DISP_ACCOMP_MUSIC_LEVEL,
    PBOX_LCD_DISP_HUMAN_MUSIC_LEVEL,
    PBOX_LCD_DISP_MUSIC_SEPERATE_SWITCH,
    PBOX_LCD_DISP_ECHO_3A_SWITCH,
    PBOX_LCD_DISP_REVERT_MODE,
    PBOX_LCD_DISP_LOOP_MODE,
    PBOX_LCD_DISP_ENERGY_INFO,
    PBOX_LCD_DISP_GUITAR_LEVEL,

    //event
    PBOX_LCD_PLAY_PAUSE_EVT = 0x100,
    PBOX_LCD_PREV_NEXT_EVT,
    PBOX_LCD_LOOP_MODE_EVT,
    PBOX_LCD_SEEK_POSITION_EVT,
    PBOX_LCD_MAIN_VOL_LEVEL_EVT,
    PBOX_LCD_MIC_VOL_LEVEL_EVT,
    PBOX_LCD_ACCOMP_MUSIC_LEVEL_EVT,//surroundings/environment sound
    PBOX_LCD_HUMAN_MUSIC_LEVEL_EVT,
    PBOX_LCD_SEPERATE_SWITCH_EVT,
    PBOX_LCD_ECHO_3A_EVT,
    PBOX_LCD_REVERT_MODE_EVT,
    PBOX_LCD_GUITAR_MUSIC_LEVEL_EVT,
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
        uint32_t        mainVolume;
        uint32_t        micVolume;
        uint32_t        accomp_music_level;//surroundings/environment sound level
        uint32_t        human_music_level;
        uint32_t        guitar_music_level;
        pbox_revertb_t      reverbMode;
        pbox_vocal_t        vocalSeparate;
        bool                echo3A_On;
        bool                enable;
        struct {
            unsigned int mCurrent;
            unsigned int mDuration;
        } positions;
        energy_info_t energy_data;
    };
} pbox_lcd_msg_t;

int pbox_create_lvglTask(void);
#ifdef __cplusplus
}
#endif
#endif