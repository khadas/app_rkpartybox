/**
 * @file lv_demo_music.h
 *
 */

#ifndef LV_DEMO_MUSIC_H
#define LV_DEMO_MUSIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lv_demos.h"
#include "pbox_lvgl.h"

#if LV_USE_DEMO_MUSIC

/*********************
 *      DEFINES
 *********************/

#if LV_DEMO_MUSIC_LARGE
#  define LV_DEMO_MUSIC_HANDLE_SIZE  40
#else
#  define LV_DEMO_MUSIC_HANDLE_SIZE  20
#endif

#undef LV_DEMO_MUSIC_LANDSCAPE
#define  LV_DEMO_MUSIC_LANDSCAPE  1    //横屏模式

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
typedef enum {
    UI_WIDGET_PLAY_PAUSE,
    UI_WIDGET_MAIN_VOLUME,
    UI_WIDGET_TRACK_INFO,
    UI_WIDGET_POSITION_INFO,
    UI_WIDGET_VOCAL_SEPERATE,
    UI_WIDGET_SPECTRUM_CHART,
    UI_WIDGET_3A_SWITCH,
    UI_WIDGET_DEVICE_STATE,
    UI_WIDGET_MIC_MUTE,
    UI_WIDGET_REVERTB_MODE,
}ui_widget_t;

void lv_demo_music(void);
int _lv_demo_music_get_track_num();
int _lv_demo_music_get_track_id();
void _lv_demo_music_update_list(uint32_t trackId);
void _lv_demo_music_update_ui_info(ui_widget_t widget, const pbox_lcd_msg_t *msg);
const char * _lv_demo_music_get_title(uint32_t track_id);
const char * _lv_demo_music_get_artist(uint32_t track_id);
const char * _lv_demo_music_get_genre(uint32_t track_id);
uint32_t _lv_demo_music_get_track_length(uint32_t track_id);
/**********************
 *      MACROS
 **********************/

#endif /*LV_USE_DEMO_MUSIC*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_MUSIC_H*/
