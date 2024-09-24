/**
 * @file lv_demo_music_main.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_music_main.h"
#if LV_USE_DEMO_MUSIC

#include "lv_demo_music_list.h"
#include "pbox_lvgl.h"
#include "pbox_app.h"
#include "../pbox_common.h"
#include "pbox_btsink_app.h"
#include "rk_btsink.h"
#include "os_minor_type.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/
//#define ACTIVE_TRACK_CNT    3

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_obj_t * create_cont(lv_obj_t * parent);
static lv_obj_t * create_title_box(lv_obj_t * parent);
static lv_obj_t * create_chart_box(lv_obj_t * parent);
static lv_obj_t * create_ctrl_box(lv_obj_t * parent);
static lv_obj_t * create_handle(lv_obj_t * parent);
static lv_obj_t * create_misc_box(lv_obj_t * parent);

static void play_event_click_cb(lv_event_t * e);
static void prev_click_event_cb(lv_event_t * e);
static void next_click_event_cb(lv_event_t * e);
static void duration_slider_event_cb(lv_event_t * e);
static void timer_cb(lv_timer_t * t);
static void track_load(uint32_t id);
static void create_toast(lv_obj_t * parent, const char * text, uint32_t duration_ms);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * main_cont;
static lv_obj_t * chart_obj;
static lv_chart_series_t * serdata;
static lv_obj_t * title_label;
static lv_obj_t * artist_label;
static lv_obj_t * genre_label;
static lv_obj_t * source_label;
static lv_obj_t * time_obj;
static lv_obj_t * duration_obj;
static lv_obj_t * slider_obj;
static uint32_t time_act;
static lv_timer_t  * sec_counter_timer;
static const lv_font_t * font_small;
static const lv_font_t * font_large;
static uint32_t track_id = 0;
static lv_obj_t * play_obj;
static lv_obj_t * voice_obj;
static lv_obj_t * vocal_toast;
static lv_obj_t * reverb_dd_obj;
static lv_obj_t * eq_mode_obj;
static lv_timer_t  * vocal_toast_timer;

//karaoke control
extern lv_ft_info_t ttf_main_s;
extern lv_ft_info_t ttf_main_m;
extern lv_ft_info_t ttf_main_l;
int32_t mHumanLevel=15, mAccomLevel=100, mMainVolLevel = 100;
int32_t mVolumeLevel=50, mMicVolumeLevel=50;
//bool mEchoReductionEnable = true;
usb_state_t lv_usbstate;
btsink_state_t lv_btstate;
bool lv_uacstate;

bool mVocalSplit = false;
lv_obj_t * vocal_text = NULL;
lv_obj_t * vocal_label = NULL;
lv_obj_t * vocal_slider = NULL;
lv_obj_t * accomp_text = NULL;
lv_obj_t * accomp_label = NULL;
lv_obj_t * accomp_slider = NULL;
lv_obj_t * mainVol_text = NULL;
lv_obj_t * mainVol_label = NULL;
lv_obj_t * mainVol_slider = NULL;
lv_obj_t * volume_text = NULL;
lv_obj_t * volume_label = NULL;
lv_obj_t * volume_slider = NULL;
lv_obj_t * micVol_text = NULL;
lv_obj_t * micVol_label = NULL;
lv_obj_t * mic_volume_slider = NULL;

lv_obj_t *origin_switch = NULL;
lv_obj_t *echo_3a_switch = NULL;
lv_style_t accomp_style_disabled;
lv_style_t vocal_style_disabled;
lv_style_t guitar_style_disabled;
lv_style_t mic_style_disabled;

/**********************
 *      MACROS
 **********************/

#define TOAST_TEXT "Volume UP U device to show \nBETTER music Split Perfermance."
/**********************
 *   GLOBAL FUNCTIONS
 **********************/
lv_obj_t * _lv_demo_music_main_create(lv_obj_t * parent)
{
#if LV_DEMO_MUSIC_LARGE
    font_small = &lv_font_montserrat_22;
    font_large = &lv_font_montserrat_32;
#else
    font_small = &lv_font_montserrat_12;
    font_large = &lv_font_montserrat_16;
#endif

    /*Create the content of the music player*/
    lv_obj_t * cont = create_cont(parent);
    lv_obj_t * title_box = create_title_box(cont);
    lv_obj_t * ctrl_box = create_ctrl_box(cont);
    chart_obj = create_chart_box(cont);
    lv_obj_t * handle_box = create_handle(cont);
    lv_obj_t * misc_obj = create_misc_box(cont);

#if LV_DEMO_MUSIC_ROUND
    lv_obj_set_style_pad_hor(cont, LV_HOR_RES / 6, 0);
#endif

    /*Arrange the content into a grid*/
#if LV_DEMO_MUSIC_SQUARE || LV_DEMO_MUSIC_ROUND
    static const lv_coord_t grid_cols[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_rows[] = {LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                     0,   /*Spectrum obj, set later*/
                                     LV_GRID_CONTENT, /*Title box*/
                                     LV_GRID_FR(3),   /*Spacer*/
                                     LV_GRID_CONTENT, /*Icon box*/
                                     LV_GRID_FR(3),   /*Spacer*/
                                     LV_GRID_CONTENT, /*Control box*/
                                     LV_GRID_FR(3),   /*Spacer*/
                                     LV_GRID_CONTENT, /*Handle box*/
                                     LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                     LV_GRID_TEMPLATE_LAST
                                    };

    grid_rows[1] = LV_VER_RES;

    lv_obj_set_grid_dsc_array(cont, grid_cols, grid_rows);
    lv_obj_set_style_grid_row_align(cont, LV_GRID_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_grid_cell(title_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(ctrl_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 6, 1);
    lv_obj_set_grid_cell(handle_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_ALIGN_CENTER, 8, 1);
#elif LV_DEMO_MUSIC_LANDSCAPE == 0
    static const lv_coord_t grid_cols[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_rows[] = {LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Title box*/
                                           LV_GRID_FR(3),   /*Spacer*/
# if LV_DEMO_MUSIC_LARGE == 0
                                           250,    /*Spectrum obj*/
# else
                                           480,   /*Spectrum obj*/
# endif
                                           LV_GRID_FR(3),   /*Spacer*/
					   LV_GRID_CONTENT, /*misc control*/
					   LV_GRID_FR(3),   /*Spacer */
                                           LV_GRID_CONTENT, /*Control box*/
                                           LV_GRID_FR(3),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Handle box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                           LV_GRID_TEMPLATE_LAST
                                          };

    lv_obj_set_grid_dsc_array(cont, grid_cols, grid_rows);
    lv_obj_set_style_grid_row_align(cont, LV_GRID_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_grid_cell(title_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(chart_obj, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(misc_obj, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 6, 1);
    lv_obj_set_grid_cell(ctrl_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 8, 1);
    lv_obj_set_grid_cell(handle_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 10, 1);
#else
    /*Arrange the content into a grid*/
    static const lv_coord_t grid_cols[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_rows[] = {LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Title box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Control box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_GRID_CONTENT, /*Handle box*/
                                           LV_GRID_FR(1),   /*Spacer*/
                                           LV_DEMO_MUSIC_HANDLE_SIZE,     /*Spacing*/
                                           LV_GRID_TEMPLATE_LAST
                                          };

    lv_obj_set_grid_dsc_array(cont, grid_cols, grid_rows);
    lv_obj_set_style_grid_row_align(cont, LV_GRID_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_grid_cell(title_box, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_set_grid_cell(ctrl_box, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(handle_box, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 6, 1);
    lv_obj_set_grid_cell(misc_obj, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(chart_obj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
#endif

    sec_counter_timer = lv_timer_create(timer_cb, 1000, NULL);
    lv_timer_pause(sec_counter_timer);

    lv_obj_update_layout(main_cont);

    return main_cont;
}

void _lv_demo_music_album_next(bool next)
{
    uint32_t id = _lv_demo_music_get_track_id();
    uint32_t track_num = _lv_demo_music_get_track_num();
    play_status_t play_status = pboxUIdata->play_status;

    printf("%s, next:%d\n", __func__, next);
    lcd_pbox_notifyPrevNext(next);

    //_lv_demo_music_list_btn_check(id, false);
    if(next) {
            id++;
            if(id >= track_num) id = 0;
    }
    else {
            if(id == 0) {
                id = track_num - 1;
            }
            else {
                id--;
            }
    }
    _lv_demo_music_list_btn_check(id, true);

#if 0
    if(play_status == PLAYING) {
        _lv_demo_music_play(id);
    } else {
        track_load(id);   //load the next track
        play_status = _STOP;  //player status change from pause  to stop
    }
#endif
}

void _lv_demo_music_update_track_info(uint32_t id) {
    uint32_t track_num = _lv_demo_music_get_track_num();
    printf("%s id %d\n", __func__, id);
    play_status_t play_status = pboxUIdata->play_status;
    if (track_num > 0) {
        //if (play_status == IDLE || play_status == _STOP) {
            lv_label_set_text(title_label, _lv_demo_music_get_title(id));
            lv_label_set_text(artist_label, _lv_demo_music_get_artist(id));
            lv_label_set_text(genre_label, _lv_demo_music_get_genre(id));
            _lv_demo_music_list_btn_check(id, true);
        //}
    } else {
        if (getBtSinkState() != APP_BT_CONNECTED) {
            lv_label_set_text(title_label, "unknow");
            lv_label_set_text(artist_label, _lv_demo_music_get_artist(id));
            lv_label_set_text(genre_label, _lv_demo_music_get_genre(id));
        }
    }
}

void _lv_demo_music_play(uint32_t id)
{
    lcd_pbox_notifyTrackid(id);
    _lv_demo_music_resume();
}

void _lv_demo_music_stop() {
    lcd_pbox_notifyPlayStop();
}

void _lv_demo_music_resume(void)
{
    uint32_t duration;
    lv_obj_add_state(play_obj, LV_STATE_CHECKED);
    lv_obj_invalidate(play_obj);
#if 0
    {
        if (play_status == IDLE || play_status == _STOP) {
            //lv_timer_resume(sec_counter_timer);
            duration = (uint32_t)(rk_demo_music_get_duration()/(1000*1000));
            lv_slider_set_range(slider_obj, 0, duration);
            lv_slider_set_left_value(slider_obj, 0, LV_ANIM_OFF);
            lv_label_set_text_fmt(duration_obj, "%"LV_PRIu32":%02"LV_PRIu32, duration/ 60, duration % 60);
        } else if (play_status == _PAUSE) {
        }
        lv_timer_resume(sec_counter_timer);
    }
#endif
    lcd_pbox_notifyPlayPause(1);
}

void _lv_demo_music_pause(void)
{
    lv_timer_pause(sec_counter_timer);
    lv_obj_clear_state(play_obj, LV_STATE_CHECKED);
    lv_obj_invalidate(play_obj);
    lcd_pbox_notifyPlayPause(0);
}

void _lv_demo_music_update_ui_info(ui_widget_t widget, const pbox_lcd_msg_t *msg)
{
    switch(widget) {
        case UI_WIDGET_PLAY_PAUSE: {
            bool play = msg->play;
            if (play) {
                //lv_timer_resume(sec_counter_timer);
                lv_obj_add_state(play_obj, LV_STATE_CHECKED);
                lv_obj_invalidate(play_obj);
            } else {
                lv_obj_clear_state(play_obj, LV_STATE_CHECKED);
                lv_obj_invalidate(play_obj);
            }
        } break;
        case UI_WIDGET_MUSIC_VOLUME: {
            uint32_t mVolume = msg->mVolume;
            char buf[16];
            lv_snprintf(buf, sizeof(buf), "%3d", mVolume);
            lv_label_set_text(volume_label, buf);
            lv_slider_set_value(volume_slider, mVolume, LV_ANIM_OFF);
        } break;
        case UI_WIDGET_MAIN_VOLUME: {
            uint32_t mVolume = msg->mVolume;
            char buf[16];
            lv_snprintf(buf, sizeof(buf), "%3d", mVolume);
            lv_label_set_text(mainVol_label, buf);
            lv_slider_set_value(mainVol_slider, mVolume, LV_ANIM_OFF);
        } break;
        case UI_WIDGET_MIC_VOLUME: {
            uint32_t micVolume = msg->micVolume;
            char buf[16];
            lv_snprintf(buf, sizeof(buf), "%3d", micVolume);
            lv_label_set_text(micVol_label, buf);
            lv_slider_set_value(mic_volume_slider, micVolume, LV_ANIM_OFF);
        } break;
        case UI_WIDGET_GENDER_INFO: {
            gender_t gender = msg->gender;
            if(gender == GENDER_F) {
                lv_obj_set_style_text_color(title_label, lv_color_hex(0xD55774), 0);
                lv_obj_set_style_text_color(artist_label, lv_color_hex(0xD55774), 0);
            } else if (gender == GENDER_M) {
                lv_obj_set_style_text_color(title_label, lv_color_hex(0x0000FF), 0);
                lv_obj_set_style_text_color(artist_label, lv_color_hex(0x0000FF), 0);
            } else if(gender == GENDER_COMBO) {
                lv_obj_set_style_text_color(title_label, lv_color_hex(0x7030A0), 0);
                lv_obj_set_style_text_color(artist_label, lv_color_hex(0x7030A0), 0);
            } else if (gender == GENDER_TBD) {
                lv_obj_set_style_text_color(title_label, lv_color_hex(0x504d6d), 0);
                lv_obj_set_style_text_color(artist_label, lv_color_hex(0x504d6d), 0);
            }
        } break;
        case UI_WIDGET_TRACK_INFO: {
            uint32_t track_id = _lv_demo_music_get_track_id();
            char *artist = (char*)(msg->track.artist);
            char *title = (char*)(msg->track.title);
            if(artist && (strlen(artist)>0))
                lv_label_set_text(artist_label, artist);
            if(title && (strlen(title)>0))
                lv_label_set_text(title_label, title);
            _lv_demo_music_list_btn_check(track_id, true);
        } break;
        case UI_WIDGET_POSITION_INFO: {
            static int  prev_total = 0;
            uint32_t current = msg->positions.mCurrent;
            uint32_t total = msg->positions.mDuration;
            uint32_t onlytotal = msg->positions.onlyDuration;
            //printf("%s position:[%d]-[%d](%d)\n", __func__, current, total, prev_total);
            if(prev_total != total) {
                prev_total = total;
                total = total/1000;
                lv_slider_set_range(slider_obj, 0, total);
                lv_slider_set_left_value(slider_obj, 0, LV_ANIM_OFF);
                lv_label_set_text_fmt(duration_obj, "%"LV_PRIu32":%02"LV_PRIu32, total/ 60, total % 60);
            }

            //if(!onlytotal) 
            {
            time_act = current = current/1000;
            lv_label_set_text_fmt(time_obj, "%"LV_PRIu32":%02"LV_PRIu32, current / 60, current % 60);
            lv_slider_set_value(slider_obj, current, LV_ANIM_ON);
            }
        } break;

            uint32_t mVolume = msg->mVolume;
            char buf[16];
            lv_snprintf(buf, sizeof(buf), "%3d", mVolume);
            lv_label_set_text(volume_label, buf);
            lv_slider_set_value(volume_slider, mVolume, LV_ANIM_OFF);

        case UI_WIDGET_VOCAL_SEPERATE: {
            char buf[16];
            pbox_vocal_t vocal = msg->vocalSeparate;
            bool seperateEnable = vocal.enable;
            vocal_lib_t lib = vocal.vocaltype;

            if(lib == VOCAL_GUITAR) {
                lv_snprintf(buf, sizeof(buf), "%7s", " Guitar");
                lv_label_set_text(vocal_text, buf);
                lv_snprintf(buf, sizeof(buf), "%3d", vocal.reservLevel);
                lv_label_set_text(vocal_label, buf);
                lv_slider_set_value(vocal_slider, vocal.reservLevel, LV_ANIM_OFF);
            } else {
                lv_snprintf(buf, sizeof(buf), "%7s", " Vocal");
                lv_label_set_text(vocal_text, buf);
                lv_snprintf(buf, sizeof(buf), "%3d", vocal.humanLevel);
                lv_label_set_text(vocal_label, buf);
                lv_slider_set_value(vocal_slider, vocal.humanLevel, LV_ANIM_OFF);
            }

            if(seperateEnable) {
                lv_obj_add_state(origin_switch, LV_STATE_CHECKED);
                lv_obj_clear_state(accomp_slider, LV_STATE_DISABLED);
                lv_obj_clear_state(vocal_slider, LV_STATE_DISABLED);
                if (getBtSinkState() == APP_BT_CONNECTED)
                    create_toast(main_cont, TOAST_TEXT, 5000);
            }
            else {
                lv_obj_clear_state(origin_switch, LV_STATE_CHECKED);
                lv_obj_add_state(accomp_slider, LV_STATE_DISABLED);
                lv_obj_add_state(vocal_slider, LV_STATE_DISABLED);
            }
        } break;
        case UI_WIDGET_SPECTRUM_CHART: {
            energy_info_t energyData = msg->energy_data;
            switch(energyData.dest) {
                case ENERGY_PLAYER: {
                    lv_chart_set_series_color(chart_obj, serdata, lv_theme_get_color_primary(chart_obj));
                } break;
                case ENERGY_MIC: {
                    lv_chart_set_series_color(chart_obj, serdata, lv_color_hex(0xff0000));
                } break;
            }
            for (int i = 0; i < energyData.size; i++) {
                lv_chart_set_next_value(chart_obj, serdata, energyData.energykeep[i].energy);
            }
        } break;
        case UI_WIDGET_3A_SWITCH: {
            bool echo3a = msg->echo3a;
            if(echo3a) {
                lv_obj_add_state(echo_3a_switch, LV_STATE_CHECKED);
            }
            else {
                lv_obj_clear_state(echo_3a_switch, LV_STATE_CHECKED);
            }
        } break;

        case UI_WIDGET_DEVICE_STATE: {
            if(msg->msgId == PBOX_LCD_DISP_BT_STATE) {
                lv_btstate = msg->btState;
            }
            if(msg->msgId == PBOX_LCD_DISP_USB_STATE) {
                lv_usbstate = msg->usbState;
            }
            if(msg->msgId == PBOX_LCD_DISP_UAC_STATE) {
                lv_uacstate = msg->uac_start;
            }

            switch (msg->msgId) {
                case PBOX_LCD_DISP_BT_STATE: {
                    if (lv_btstate == APP_BT_CONNECTED) {
                        char name[MAX_NAME_LENGTH];
                        lv_snprintf(name, sizeof(name), "%s", getBtRemoteName());
                        printf("%s remote name %s\n", __func__, name);
                        lv_label_set_text(source_label, name);
                        lv_obj_set_style_text_color(source_label, lv_color_hex(0x0082FC), 0);
                    } //no break. go through
                    else {
                        lv_label_set_text(source_label, "Wishing air connect");
                        lv_obj_set_style_text_color(source_label, lv_color_hex(0x0082FC), 0);
                    }
                } break;
                case PBOX_LCD_DISP_USB_STATE: {
                    if ((lv_usbstate == USB_CONNECTED) || (lv_usbstate == USB_SCANNED)) {
                        lv_label_set_text(source_label, "USB Inserted");
                        lv_obj_set_style_text_color(source_label, lv_color_hex(0x00B050), 0);
                    } //no break. go through
                    else {
                        lv_label_set_text(source_label, "Wishing USB connect");
                        lv_obj_set_style_text_color(source_label, lv_color_hex(0x00B050), 0);
                    }
                } break;
                case PBOX_LCD_DISP_UAC_STATE: {
                    if (lv_uacstate == true) {
                        lv_label_set_text(source_label, "UAC Audio");
                        lv_obj_set_style_text_color(source_label, lv_color_hex(0x00B050), 0);
                    }
                    else {
                        lv_label_set_text(source_label, "Wishing UAC");
                        lv_obj_set_style_text_color(source_label, lv_color_hex(0x00B050), 0);
                    }
                } break;
            }
        } break;
        case UI_WIDGET_MIC_MUTE: {
            bool mute = msg->micmute;
            printf("%s mute:%s\n", __func__, mute?"on":"off");
            if(mute) {
                lv_obj_add_state(mic_volume_slider, LV_STATE_DISABLED);
            } else {
                lv_obj_clear_state(mic_volume_slider, LV_STATE_DISABLED);
            }
        } break;
        case UI_WIDGET_REVERTB_MODE: {
            pbox_revertb_t mode = msg->reverbMode;
            printf("%s revertb_mode:%d\n", __func__, mode);
            lv_dropdown_set_selected(reverb_dd_obj, mode);
        } break;
        case UI_WIDGET_EQ_INFO: {
            equalizer_t mode = msg->eqMode;
            printf("%s eq_mode:%d\n", __func__, mode);
            lv_dropdown_set_selected(eq_mode_obj, mode);
        } break;
        default:
            break;
    };
}

// Callback Function for Toast Closed
static void vocal_toast_close_cb(lv_timer_t * timer) {
	if (vocal_toast != NULL) {
		lv_obj_del(vocal_toast);
		vocal_toast = NULL;
	}
	if (vocal_toast_timer != NULL) {
		lv_timer_del(vocal_toast_timer);
		vocal_toast_timer = NULL;
	}
}

void create_toast(lv_obj_t * parent, const char * text, uint32_t duration_ms) {
	if (vocal_toast == NULL) {
		vocal_toast = lv_label_create(parent);
		lv_label_set_text(vocal_toast, text);
		lv_obj_set_size(vocal_toast, lv_pct(50), lv_pct(50));
		lv_obj_set_style_bg_color(vocal_toast, lv_color_white(), 0);
		lv_obj_set_style_bg_opa(vocal_toast, LV_OPA_70, 0);
		lv_obj_set_style_text_color(vocal_toast, lv_color_hex(0xF9680D), 0);
		lv_obj_set_style_pad_all(vocal_toast, 10, 0);
		lv_obj_set_style_radius(vocal_toast, 5, 0);
		lv_obj_set_style_text_font(vocal_toast, ttf_main_m.font, 0);
		lv_obj_align(vocal_toast, LV_ALIGN_CENTER, 0, 0);

		if(vocal_toast_timer == NULL)
			vocal_toast_timer = lv_timer_create(vocal_toast_close_cb, duration_ms, vocal_toast);
	}
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_obj_t * create_cont(lv_obj_t * parent)
{
    /*A transparent container in which the player section will be scrolled*/
    main_cont = lv_obj_create(parent);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_remove_style_all(main_cont);                            /*Make it transparent*/
    lv_obj_set_size(main_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_scroll_snap_y(main_cont, LV_SCROLL_SNAP_CENTER);    /*Snap the children to the center*/

    /*Create a container for the player*/
    lv_obj_t * player = lv_obj_create(main_cont);
    lv_obj_set_y(player, - LV_DEMO_MUSIC_HANDLE_SIZE);
#if LV_DEMO_MUSIC_SQUARE || LV_DEMO_MUSIC_ROUND
    lv_obj_set_size(player, LV_HOR_RES, 2 * LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE * 2);
#else
    lv_obj_set_size(player, LV_HOR_RES, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE * 2);
#endif
    lv_obj_clear_flag(player, LV_OBJ_FLAG_SNAPABLE);

    lv_obj_set_style_bg_color(player, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_width(player, 0, 0);
    lv_obj_set_style_pad_all(player, 0, 0);
    lv_obj_set_scroll_dir(player, LV_DIR_VER);

    /* Transparent placeholders below the player container
     * It is used only to snap it to center.*/
    lv_obj_t * placeholder1 = lv_obj_create(main_cont);
    lv_obj_remove_style_all(placeholder1);
    lv_obj_clear_flag(placeholder1, LV_OBJ_FLAG_CLICKABLE);
        //lv_obj_set_style_bg_color(placeholder1, lv_color_hex(0xff0000), 0);
        //lv_obj_set_style_bg_opa(placeholder1, LV_OPA_50, 0);

    lv_obj_t * placeholder2 = lv_obj_create(main_cont);
    lv_obj_remove_style_all(placeholder2);
    lv_obj_clear_flag(placeholder2, LV_OBJ_FLAG_CLICKABLE);
        //lv_obj_set_style_bg_color(placeholder2, lv_color_hex(0x00ff00), 0);
        //lv_obj_set_style_bg_opa(placeholder2, LV_OPA_50, 0);

#if LV_DEMO_MUSIC_SQUARE || LV_DEMO_MUSIC_ROUND
    lv_obj_t * placeholder3 = lv_obj_create(main_cont);
    lv_obj_remove_style_all(placeholder3);
    lv_obj_clear_flag(placeholder3, LV_OBJ_FLAG_CLICKABLE);
    //    lv_obj_set_style_bg_color(placeholder3, lv_color_hex(0x0000ff), 0);
    //    lv_obj_set_style_bg_opa(placeholder3, LV_OPA_20, 0);

    lv_obj_set_size(placeholder1, lv_pct(100), LV_VER_RES);
    lv_obj_set_y(placeholder1, 0);

    lv_obj_set_size(placeholder2, lv_pct(100), LV_VER_RES);
    lv_obj_set_y(placeholder2, LV_VER_RES);

    lv_obj_set_size(placeholder3, lv_pct(100),  LV_VER_RES - 2 * LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_y(placeholder3, 2 * LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE);
#else
    lv_obj_set_size(placeholder1, lv_pct(100), LV_VER_RES);
    lv_obj_set_y(placeholder1, 0);

    lv_obj_set_size(placeholder2, lv_pct(100),  LV_VER_RES - 2 * LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_y(placeholder2, LV_VER_RES + LV_DEMO_MUSIC_HANDLE_SIZE);
#endif

    lv_obj_update_layout(main_cont);

    return player;
}

static lv_obj_t * create_title_box(lv_obj_t * parent)
{
    /*Create the titles*/
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    title_label = lv_label_create(cont);
    //lv_obj_set_style_text_font(title_label, font_large, 0);
    lv_obj_set_style_text_font(title_label, ttf_main_m.font, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x504d6d), 0);
    lv_label_set_text(title_label, _lv_demo_music_get_title(track_id));
    lv_obj_set_height(title_label, lv_font_get_line_height(font_large) * 3 / 2);

    artist_label = lv_label_create(cont);
    lv_obj_set_style_text_font(artist_label, ttf_main_s.font, 0);
    lv_obj_set_style_text_color(artist_label, lv_color_hex(0x5F5F5F/*0x00B06d*/), 0);
    lv_label_set_text(artist_label, _lv_demo_music_get_artist(track_id));
    lv_obj_set_grid_cell(artist_label, LV_GRID_ALIGN_END, 5, 1, LV_GRID_ALIGN_END, 0, 1);

    genre_label = lv_label_create(cont);
    lv_obj_set_style_text_font(genre_label, font_small, 0);
    lv_obj_set_style_text_color(genre_label, lv_color_hex(0x8a86b8), 0);
    lv_label_set_text(genre_label, _lv_demo_music_get_genre(track_id));

    //track sources are from usb disk or bluetooth
    source_label = lv_label_create(cont);
    lv_label_set_text(source_label, "");
    lv_obj_set_style_text_font(source_label, ttf_main_s.font, 0);
    //lv_obj_set_style_text_line_space(source_label, 8, 0);
    //lv_obj_align_to(source_label, title_label, LV_ALIGN_OUT_LEFT_MID, -20, 0);
    lv_obj_set_align(source_label, LV_ALIGN_TOP_LEFT);

    return cont;
}

static void reverb_event_handler(lv_event_t *e) {
    lv_obj_t * dropdown = lv_event_get_target(e);
    pbox_revertb_t mode;
    char buf[64];
    lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));

    if (!strcmp(buf, "STUDIO"))
        mode = PBOX_REVERT_STUDIO;
    else if (!strcmp(buf, "KTV"))
        mode = PBOX_REVERT_KTV;
    else if (!strcmp(buf, "CONCERT"))
        mode = PBOX_REVERT_CONCERT;
    else if (!strcmp(buf, "ECHO"))
        mode = PBOX_REVERT_ECHO;
    else if (!strcmp(buf, "USER"))
        mode = PBOX_REVERT_USER;
    else 
         mode = PBOX_REVERT_OFF;

    printf("'%s' is selected\n", buf);
    lcd_pbox_notifyReverbMode(mode);
}

static void eqmode_event_handler(lv_event_t *e) {
    lv_obj_t * dropdown = lv_event_get_target(e);
    equalizer_t mode;
    char buf[64];
    lv_dropdown_get_selected_str(dropdown, buf, sizeof(buf));

    if (!strcmp(buf, "OFF"))
        mode = EQ_OFF;
    else if (!strcmp(buf, "ROCK"))
        mode = EQ_ROCK;
    else if (!strcmp(buf, "POP"))
        mode = EQ_POP;
    else if (!strcmp(buf, "JAZZ"))
        mode = EQ_JAZZ;
    else if (!strcmp(buf, "ELEC"))
        mode = EQ_ELEC;
    else if (!strcmp(buf, "DANCE"))
        mode = EQ_DANCE;
    else if (!strcmp(buf, "COUNTRY"))
        mode = EQ_CONTR;
    else if (!strcmp(buf, "CLASSIC"))
        mode = EQ_CLASS;
    else if (!strcmp(buf, "BLUES"))
        mode = EQ_BLUES;
    else if (!strcmp(buf, "BALL"))
        mode = EQ_BALL;
    else
         mode = EQ_OFF;

    printf("'%s' is selected\n", buf);
    lcd_pbox_notifyEqMode(mode);
}

void echol_3a_seperate_event_handler(lv_event_t * e) {
    lv_obj_t * obj = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        printf("State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
    }
    bool enable = lv_obj_has_state(obj, LV_STATE_CHECKED) ? true : false;
    lcd_pbox_notifyEcho3A(enable);

}

static void mainVolume_slider_event_cb(lv_event_t *e) {
    lv_obj_t * slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "%3d", (int)lv_slider_get_value(slider));

    if (code == LV_EVENT_RELEASED) {
        printf("last slider value%s\n", buf);
        if (mainVol_label != NULL) {
            lv_label_set_text(mainVol_label, buf);
            mMainVolLevel = (int)lv_slider_get_value(slider);
            lcd_pbox_notifyMainVolumeLevel(mMainVolLevel);
        }
    }
}

static void vocal_seperate_event_handler(lv_event_t * e) {
    lv_obj_t * obj = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        printf("State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
    }
    mVocalSplit = lv_obj_has_state(obj, LV_STATE_CHECKED) ? true : false;

    if(mVocalSplit) {
        lv_obj_clear_state(accomp_slider, LV_STATE_DISABLED);
        lv_obj_clear_state(vocal_slider, LV_STATE_DISABLED);
        if (mainVol_slider != NULL)
            lv_obj_clear_state(mainVol_slider, LV_STATE_DISABLED);
	if (getBtSinkState() == APP_BT_CONNECTED)
		create_toast(main_cont, TOAST_TEXT, 5000);
    }
    else {
        lv_obj_add_state(accomp_slider, LV_STATE_DISABLED);
        lv_obj_add_state(vocal_slider, LV_STATE_DISABLED);
        if (mainVol_slider != NULL)
            lv_obj_add_state(mainVol_slider, LV_STATE_DISABLED);
    }

    lcd_pbox_notifySeparateSwitch(mVocalSplit);
}

static void accomp_slider_event_cb(lv_event_t *e) {
    lv_obj_t * slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "%3d", (int)lv_slider_get_value(slider));

    if (code == LV_EVENT_RELEASED) {
        printf("last slider value%s\n", buf);
        lv_label_set_text(accomp_label, buf);
        mAccomLevel = (int)lv_slider_get_value(slider);
        lcd_pbox_notifyAccompMusicLevel(mAccomLevel);
    }
}

static void vocal_slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "%3d", (int)lv_slider_get_value(slider));

    if (code == LV_EVENT_RELEASED) {
        printf("slider2 value %s\n", buf);
        lv_label_set_text(vocal_label, buf);

        mHumanLevel = (int)lv_slider_get_value(slider);
        lcd_pbox_notifyHumanMusicLevel(mHumanLevel);
    }
}

static void master_volume_change_event_cb(lv_event_t *e) {
    lv_obj_t * slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "%3d", (int)lv_slider_get_value(slider));

    if (code == LV_EVENT_RELEASED) {
        printf("master volume value %s\n", buf);
        lv_label_set_text(volume_label, buf);
        mVolumeLevel = (int)lv_slider_get_value(slider);
        lcd_pbox_notifyMusicVolLevel(mVolumeLevel);
    }
}

static void mic_volume_change_event_cb(lv_event_t * e) {
    lv_obj_t * slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "%3d", (int)lv_slider_get_value(slider));

    if (code == LV_EVENT_RELEASED) {
        printf("mic volume value %s\n", buf);
        lv_label_set_text(micVol_label, buf);
        mMicVolumeLevel = (int)lv_slider_get_value(slider);
        lcd_pbox_notifyMicVolLevel(mMicVolumeLevel);
    }
}

#define NUM_POINTS ENERGY_BAND_DETECT
static lv_obj_t * create_chart_box(lv_obj_t * parent)
{
    /* Create the spectrum chart visualizer */
    lv_obj_t *obj = lv_chart_create(parent);
    lv_chart_set_axis_tick(obj, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 4, 1, true, 80);
    lv_chart_set_axis_tick(obj, LV_CHART_AXIS_PRIMARY_X, 5, 3, 10, 1, true, 50);
    lv_chart_set_type(obj, LV_CHART_TYPE_BAR);
    lv_chart_set_div_line_count(obj, 10, 0);
    lv_chart_set_point_count(obj, NUM_POINTS);
    lv_chart_set_range(obj, LV_CHART_AXIS_PRIMARY_X, 0, 100);
    lv_chart_set_range(obj, LV_CHART_AXIS_PRIMARY_Y, -90, 0);
    serdata = lv_chart_add_series(obj, lv_theme_get_color_primary(obj), LV_CHART_AXIS_PRIMARY_Y);
    for (int i=0; i < NUM_POINTS; i++) {
        lv_chart_set_next_value(obj, serdata, -90);
    }
    return obj;
}

static lv_obj_t * create_misc_box(lv_obj_t * parent)
{
    int mode = 0;
    /* Create the misc control box */
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_height(cont, 360);//LV_SIZE_CONTENT);
#if LV_DEMO_MUSIC_LARGE
    lv_obj_set_style_pad_bottom(cont, 17, 0);
#else
    lv_obj_set_style_pad_bottom(cont, 8, 0);
#endif
    //this is 9 column
    static const lv_coord_t misc_grid_col[] = {
        // | vol     |  bar----|---------|--value--|
        // | reverb--|--select-|  split  |  on/off |
        // |   1st   |   3st   |   5st   |  7st    |
        LV_GRID_FR(0), LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT,
        LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };

    static const lv_coord_t misc_grid_row[] = {
        LV_GRID_CONTENT, LV_GRID_FR(4), LV_GRID_CONTENT, LV_GRID_FR(4), LV_GRID_CONTENT, LV_GRID_FR(4),
        LV_GRID_CONTENT, LV_GRID_FR(4), LV_GRID_CONTENT, LV_GRID_FR(5), LV_GRID_CONTENT, LV_GRID_FR(2),
        LV_GRID_CONTENT, LV_GRID_FR(0), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(cont, misc_grid_col, misc_grid_row);
    lv_obj_set_style_grid_row_align(cont, LV_GRID_ALIGN_SPACE_BETWEEN, 0);

    //reverb control
    lv_obj_t * reverb_label = lv_label_create(cont);
    lv_label_set_text(reverb_label, " REVERB:");
    lv_obj_set_style_text_color(reverb_label, lv_color_hex(0x1F2DA8), 0);
    lv_obj_set_style_text_font(reverb_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(reverb_label, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_START, 10, 1);
    reverb_dd_obj = lv_dropdown_create(cont);
    lv_obj_set_grid_cell(reverb_dd_obj, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_START, 10, 1);
    lv_obj_set_style_text_font(reverb_dd_obj, font_small, 0);
    lv_dropdown_set_options_static(reverb_dd_obj, "OFF\nSTUDIO\nKTV\nCONCERT\nECHO\nUSER");
    lv_dropdown_set_dir(reverb_dd_obj, LV_DIR_BOTTOM);
    lv_dropdown_set_selected(reverb_dd_obj, PBOX_REVERT_KTV);//0,1,2,3 so 3 means CONCERT
    lv_obj_add_event_cb(reverb_dd_obj, reverb_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_width(reverb_dd_obj, 20*8+5);

    lv_obj_t * eqmode_label = lv_label_create(cont);
    lv_label_set_text(eqmode_label, " EQ:");
    lv_obj_set_style_text_color(eqmode_label, lv_color_hex(0x1F2DA8), 0);
    lv_obj_set_style_text_font(eqmode_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(eqmode_label, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_START, 12, 1);
    eq_mode_obj = lv_dropdown_create(cont);
    lv_obj_set_grid_cell(eq_mode_obj, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_START, 12, 1);
    lv_obj_set_style_text_font(eq_mode_obj, font_small, 0);
    lv_dropdown_set_options_static(eq_mode_obj, "OFF\nROCK\nPOP\nJAZZ\nELEC\nDANCE\nCOUNTRY\nCLASSIC\nBLUES\nBALL");
    lv_dropdown_set_dir(eq_mode_obj, LV_DIR_BOTTOM);
    lv_dropdown_set_selected(eq_mode_obj, EQ_OFF);
    lv_obj_add_event_cb(eq_mode_obj, eqmode_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_width(eq_mode_obj, 20*8+5);

    //原唱与伴奏切换
    lv_obj_t * origin_label = lv_label_create(cont);
    lv_label_set_text(origin_label, "SPLIT:");
    lv_obj_set_style_text_color(origin_label, lv_color_hex(0x1F2DA8), 0);
    lv_obj_set_style_text_font(origin_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(origin_label, LV_GRID_ALIGN_CENTER, 5, 1, LV_GRID_ALIGN_START, 10, 1);
    origin_switch = lv_switch_create(cont);
    lv_obj_set_grid_cell(origin_switch, LV_GRID_ALIGN_START, 7, 1, LV_GRID_ALIGN_START, 10, 1);
    lv_obj_add_event_cb(origin_switch, vocal_seperate_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_clear_state(origin_switch, LV_STATE_CHECKED);

    lv_obj_t * echo_3a_label = lv_label_create(cont);
    lv_label_set_text(echo_3a_label, "3A:");
    lv_obj_set_style_text_color(echo_3a_label, lv_color_hex(0x1F2DA8), 0);
    lv_obj_set_style_text_font(echo_3a_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(echo_3a_label, LV_GRID_ALIGN_CENTER, 5, 1, LV_GRID_ALIGN_START, 12, 1);
    echo_3a_switch = lv_switch_create(cont);
    lv_obj_set_grid_cell(echo_3a_switch, LV_GRID_ALIGN_START, 7, 1, LV_GRID_ALIGN_START, 12, 1);
    lv_obj_add_event_cb(echo_3a_switch, echol_3a_seperate_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_state(echo_3a_switch, LV_STATE_CHECKED);

    //volume control
    mainVol_text = lv_label_create(cont);
    lv_label_set_text(mainVol_text, " Main Vol");
    //lv_obj_add_style(voice_label, &style_text_muted, 0);
    lv_obj_set_style_text_font(mainVol_text, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(mainVol_text, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);

    mainVol_label = lv_label_create(cont);
    lv_label_set_text(mainVol_label, "100");
    //lv_obj_add_style(voice_label, &style_text_muted, 0);
    //lv_obj_set_style_text_font(mainVol_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(mainVol_label, LV_GRID_ALIGN_START, 7, 1, LV_GRID_ALIGN_START, 0, 1);
    mainVol_slider = lv_slider_create(cont);
    lv_obj_set_width(mainVol_slider, LV_PCT(50));
    lv_obj_set_grid_cell(mainVol_slider, LV_GRID_ALIGN_END, 3, 3, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(mainVol_slider, mainVolume_slider_event_cb, LV_EVENT_RELEASED, NULL);
    lv_slider_set_mode(mainVol_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_range(mainVol_slider, 0, 100);
    lv_slider_set_value(mainVol_slider, 100, LV_ANIM_OFF);

    volume_text = lv_label_create(cont);
    lv_label_set_text(volume_text, " Music Vol");
    lv_obj_set_style_text_font(volume_text, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(volume_text, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 2, 1);

    volume_label = lv_label_create(cont);
    lv_label_set_text(volume_label, "50");
    //lv_obj_set_style_text_font(volume_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(volume_label, LV_GRID_ALIGN_START, 7, 1, LV_GRID_ALIGN_START, 2, 1);
    volume_slider = lv_slider_create(cont);
    lv_obj_set_width(volume_slider, LV_PCT(50));
    lv_obj_set_grid_cell(volume_slider, LV_GRID_ALIGN_END, 3, 3, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_add_event_cb(volume_slider, master_volume_change_event_cb, LV_EVENT_RELEASED, NULL);
    lv_slider_set_mode(volume_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_range(volume_slider, 0, 100);
    lv_slider_set_value(volume_slider, 50, LV_ANIM_OFF);

    //mic volume control
    micVol_text = lv_label_create(cont);
    lv_label_set_text(micVol_text, " Mic Vol");
    lv_obj_set_style_text_font(micVol_text, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(micVol_text, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 4, 1);

    micVol_label = lv_label_create(cont);
    lv_label_set_text(micVol_label, "100");
    //lv_obj_set_style_text_font(micVol_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(micVol_label, LV_GRID_ALIGN_START, 7, 1, LV_GRID_ALIGN_START, 4, 1);
    mic_volume_slider = lv_slider_create(cont);
    lv_obj_set_width(mic_volume_slider, LV_PCT(50));
    lv_obj_set_grid_cell(mic_volume_slider, LV_GRID_ALIGN_END, 3, 3, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_add_event_cb(mic_volume_slider, mic_volume_change_event_cb, LV_EVENT_RELEASED, NULL);
    lv_slider_set_mode(mic_volume_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_range(mic_volume_slider, 0, 100);
    //mMicVolumeLevel = 100;
    lv_slider_set_value(mic_volume_slider, 100, LV_ANIM_OFF);
    lv_style_init(&mic_style_disabled);
    lv_style_set_bg_color(&mic_style_disabled, lv_color_hex(0xCCCCCC));
    lv_obj_add_style(mic_volume_slider, &mic_style_disabled, LV_PART_INDICATOR | LV_STATE_DISABLED);
    lv_obj_add_style(mic_volume_slider, &mic_style_disabled, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_style(mic_volume_slider, &mic_style_disabled, LV_PART_KNOB | LV_STATE_DISABLED);
    lv_obj_clear_state(mic_volume_slider, LV_STATE_DISABLED);

    //accompaniment and vocal control
    accomp_text = lv_label_create(cont);
    lv_label_set_text(accomp_text, " Accom");
    //lv_obj_add_style(voice_label, &style_text_muted, 0);
    lv_obj_set_style_text_font(accomp_text, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(accomp_text, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 6, 1);

    accomp_label = lv_label_create(cont);
    lv_label_set_text(accomp_label, "100");
    //lv_obj_add_style(voice_label, &style_text_muted, 0);
    //lv_obj_set_style_text_font(accomp_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(accomp_label, LV_GRID_ALIGN_START, 7, 1, LV_GRID_ALIGN_START, 6, 1);
    accomp_slider = lv_slider_create(cont);
    lv_obj_set_width(accomp_slider, LV_PCT(50));
    lv_obj_set_grid_cell(accomp_slider, LV_GRID_ALIGN_END, 3, 3, LV_GRID_ALIGN_CENTER, 6, 1);
    lv_obj_add_event_cb(accomp_slider, accomp_slider_event_cb, LV_EVENT_RELEASED, NULL);
    lv_slider_set_mode(accomp_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_range(accomp_slider, 0, 100);
    //mAccomLevel = 100;
    lv_slider_set_value(accomp_slider, 100, LV_ANIM_OFF);
    lv_style_init(&accomp_style_disabled);
    lv_style_set_bg_color(&accomp_style_disabled, lv_color_hex(0xCCCCCC));
    lv_obj_add_style(accomp_slider, &accomp_style_disabled, LV_PART_INDICATOR | LV_STATE_DISABLED);
    lv_obj_add_style(accomp_slider, &accomp_style_disabled, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_style(accomp_slider, &accomp_style_disabled, LV_PART_KNOB | LV_STATE_DISABLED);
    lv_obj_add_state(accomp_slider, LV_STATE_DISABLED);

    vocal_text = lv_label_create(cont);
    lv_label_set_text(vocal_text, " Vocal");
    lv_obj_set_style_text_font(vocal_text, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(vocal_text, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 8, 1);
    vocal_label = lv_label_create(cont);
    lv_label_set_text(vocal_label, " 15");
    //lv_obj_set_style_text_font(vocal_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(vocal_label, LV_GRID_ALIGN_START, 7, 1, LV_GRID_ALIGN_START, 8, 1);
    vocal_slider = lv_slider_create(cont);
    lv_obj_set_width(vocal_slider, LV_PCT(50));
    lv_obj_set_grid_cell(vocal_slider, LV_GRID_ALIGN_END, 3, 3, LV_GRID_ALIGN_CENTER, 8, 1);
    lv_obj_add_event_cb(vocal_slider, vocal_slider_event_cb, LV_EVENT_RELEASED, NULL);
    lv_slider_set_mode(vocal_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_range(vocal_slider, 0, 100);
    //mHumanLevel = 15;
    lv_slider_set_value(vocal_slider, 15, LV_ANIM_OFF);
    lv_style_init(&vocal_style_disabled);
    lv_style_set_bg_color(&vocal_style_disabled, lv_color_hex(0xCCCCCC));
    lv_obj_add_style(vocal_slider, &vocal_style_disabled, LV_PART_INDICATOR | LV_STATE_DISABLED);
    lv_obj_add_style(vocal_slider, &vocal_style_disabled, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_style(vocal_slider, &vocal_style_disabled, LV_PART_KNOB | LV_STATE_DISABLED);
    lv_obj_add_state(vocal_slider, LV_STATE_DISABLED);

    return cont;
}

static lv_obj_t * create_ctrl_box(lv_obj_t * parent)
{
    /*Create the control box*/
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
#if LV_DEMO_MUSIC_LARGE
    lv_obj_set_style_pad_bottom(cont, 17, 0);
#else
    lv_obj_set_style_pad_bottom(cont, 8, 0);
#endif
    static const lv_coord_t grid_col[] = {LV_GRID_FR(2), LV_GRID_FR(3), LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_FR(3), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t grid_row[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(cont, grid_col, grid_row);

    LV_IMG_DECLARE(img_lv_demo_music_btn_loop);
    LV_IMG_DECLARE(img_lv_demo_music_btn_rnd);
    LV_IMG_DECLARE(img_lv_demo_music_btn_next);
    LV_IMG_DECLARE(img_lv_demo_music_btn_prev);
    LV_IMG_DECLARE(img_lv_demo_music_btn_play);
    LV_IMG_DECLARE(img_lv_demo_music_btn_pause);

    lv_obj_t * icon;
    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_btn_rnd);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_btn_loop);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_END, 5, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_btn_prev);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(icon, prev_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    play_obj = lv_imgbtn_create(cont);
    lv_imgbtn_set_src(play_obj, LV_IMGBTN_STATE_RELEASED, NULL, &img_lv_demo_music_btn_play, NULL);
    lv_imgbtn_set_src(play_obj, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &img_lv_demo_music_btn_pause, NULL);
    lv_obj_add_flag(play_obj, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_grid_cell(play_obj, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_add_event_cb(play_obj, play_event_click_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(play_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_width(play_obj, img_lv_demo_music_btn_play.header.w);

    icon = lv_img_create(cont);
    lv_img_set_src(icon, &img_lv_demo_music_btn_next);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(icon, next_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * spacer = lv_label_create(cont);
    lv_label_set_text(spacer, " ");
    lv_obj_set_size(spacer, LV_SIZE_CONTENT, 30);
    lv_obj_set_grid_cell(spacer, LV_GRID_ALIGN_START, 1, 5, LV_GRID_ALIGN_CENTER, 1, 1);

    LV_IMG_DECLARE(img_lv_demo_music_slider_knob);
    slider_obj = lv_slider_create(cont);
    lv_obj_set_style_anim_time(slider_obj, 100, 0);
    lv_obj_add_event_cb(slider_obj,  duration_slider_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(slider_obj,  duration_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

#if LV_DEMO_MUSIC_LARGE == 0
    lv_obj_set_height(slider_obj, 3);
#else
    lv_obj_set_height(slider_obj, 10);
#endif
    lv_obj_set_grid_cell(slider_obj, LV_GRID_ALIGN_STRETCH, 1, 5, LV_GRID_ALIGN_CENTER, 2, 1);

    lv_obj_set_style_bg_img_src(slider_obj, &img_lv_demo_music_slider_knob, LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider_obj, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider_obj, 20, LV_PART_KNOB);
    lv_obj_set_style_bg_grad_dir(slider_obj, LV_GRAD_DIR_HOR, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_obj, lv_color_hex(0x569af8), LV_PART_INDICATOR);
    lv_obj_set_style_bg_grad_color(slider_obj, lv_color_hex(0xa666f1), LV_PART_INDICATOR);
    lv_obj_set_style_outline_width(slider_obj, 0, LV_PART_MAIN);
    //lv_obj_set_style_outline_width(slider_obj, 12, LV_PART_KNOB);

    time_obj = lv_label_create(cont);
    lv_obj_set_style_text_font(time_obj, font_small, 0);
    lv_obj_set_style_text_color(time_obj, lv_color_hex(0x8a86b8), 0);
    lv_label_set_text(time_obj, "0:00");
    lv_obj_set_grid_cell(time_obj, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);

    duration_obj = lv_label_create(cont);
    lv_obj_set_style_text_font(duration_obj, font_small, 0);
    lv_obj_set_style_text_color(duration_obj, lv_color_hex(0x8a86b8), 0);
    lv_label_set_text(duration_obj, "0:00");
    lv_obj_set_grid_cell(duration_obj, LV_GRID_ALIGN_END, 5, 1, LV_GRID_ALIGN_CENTER, 3, 1);

    return cont;
}

static lv_obj_t * create_handle(lv_obj_t * parent)
{
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);

    lv_obj_set_size(cont, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont, 8, 0);

    /*A handle to scroll to the track list*/
    lv_obj_t * handle_label = lv_label_create(cont);
    lv_label_set_text(handle_label, "ALL TRACKS");
    lv_obj_set_style_text_font(handle_label, font_small, 0);
    lv_obj_set_style_text_color(handle_label, lv_color_hex(0x8a86b8), 0);

    lv_obj_t * handle_rect = lv_obj_create(cont);
#if LV_DEMO_MUSIC_LARGE
    lv_obj_set_size(handle_rect, 40, 3);
#else
    lv_obj_set_size(handle_rect, 20, 2);
#endif

    lv_obj_set_style_bg_color(handle_rect, lv_color_hex(0x8a86b8), 0);
    lv_obj_set_style_border_width(handle_rect, 0, 0);

    return cont;
}

static void track_load(uint32_t id)
{
    uint32_t track_num = _lv_demo_music_get_track_num();
    time_act = 0;
    lv_slider_set_value(slider_obj, 0, LV_ANIM_OFF);
    lv_label_set_text(time_obj, "0:00");

    if(id == track_id) return;
    bool next = false;
    if((track_id + 1) % track_num == id) next = true;

    //_lv_demo_music_list_btn_check(track_id, false);
    track_id = id;
    _lv_demo_music_list_btn_check(id, true);
    lcd_pbox_notifyTrackid(track_id);

    lv_label_set_text(title_label, _lv_demo_music_get_title(track_id));
    lv_label_set_text(artist_label, _lv_demo_music_get_artist(track_id));
    lv_label_set_text(genre_label, _lv_demo_music_get_genre(track_id));
}

static uint32_t time_ms = 0;
static void play_event_click_cb(lv_event_t * e)
{
    static bool last_state = false;
    static uint32_t time_ms = 0;

        lv_obj_t * obj = lv_event_get_target(e);
    if((os_unix_time_ms() - time_ms) < 300) {
        if(last_state && (!lv_obj_has_state(obj, LV_STATE_CHECKED))) {
            printf("last state playing skip\n");
            lv_obj_add_state(obj, LV_STATE_CHECKED);
            lv_obj_invalidate(obj);
        }
        else if (!last_state && (lv_obj_has_state(obj, LV_STATE_CHECKED))) {
            printf("last state playing skip\n");
            lv_obj_clear_state(obj, LV_STATE_CHECKED);
            lv_obj_invalidate(obj);
        }
        return;
    }

    time_ms = os_unix_time_ms();

    if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
        _lv_demo_music_resume();
        last_state = true;
    }
    else {
        _lv_demo_music_pause();
        last_state = false;
    }
}

static void prev_click_event_cb(lv_event_t * e)
{
    static uint32_t prev_last_time = 0;
    LV_UNUSED(e);

    if((os_unix_time_ms() - prev_last_time) < 300) {
        return;
    }

    prev_last_time = os_unix_time_ms();

    _lv_demo_music_album_next(false);
}

static void next_click_event_cb(lv_event_t * e)
{
    static uint32_t next_last_time = 0;
    if((os_unix_time_ms() - next_last_time) < 300) {
        return;
    }

    next_last_time = os_unix_time_ms();

    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        _lv_demo_music_album_next(true);
    }
}

static bool duration_slider_oping = false;
static void duration_slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (isBtA2dpConnected())
        return;
    duration_slider_oping = (code == LV_EVENT_VALUE_CHANGED) ? true: false;

    time_act = lv_slider_get_value(slider);

    if (time_act > lv_slider_get_max_value(slider)) {
        printf("%s time_act=%d max: %d\n",__func__, time_act, lv_slider_get_max_value(slider));
        return;
    }

    if(code == LV_EVENT_VALUE_CHANGED || code == LV_EVENT_RELEASED) {
        lv_label_set_text_fmt(time_obj, "%"LV_PRIu32":%02"LV_PRIu32, time_act/60, time_act % 60);
        lv_slider_set_value(slider, time_act, LV_ANIM_ON);
        if( code == LV_EVENT_RELEASED) {
            printf("seek to %d:%d\n", time_act / 60, time_act % 60);
            lcd_pbox_notifySeekPosition(1000 * time_act, 1000 * lv_slider_get_max_value(slider));
       }
    }
}

static void timer_cb(lv_timer_t * t)
{
    if (duration_slider_oping)
        return;
    LV_UNUSED(t);
    time_act++;

    if (time_act > lv_slider_get_max_value(slider_obj)) {
        //printf("%s time_act=%d max: %d\n",__func__, time_act, lv_slider_get_max_value(slider_obj));
        return;
    }

    lv_label_set_text_fmt(time_obj, "%"LV_PRIu32":%02"LV_PRIu32, time_act / 60, time_act % 60);
    lv_slider_set_value(slider_obj, time_act, LV_ANIM_ON);
}

bool is_energy_debug(void);

void _lv_demo_music_destroy(void)
{
    if (serdata != NULL)
        lv_chart_remove_series(chart_obj, serdata);
    if (sec_counter_timer != NULL)
        lv_timer_del(sec_counter_timer);
    if (vocal_toast_timer != NULL)
        lv_timer_del(vocal_toast_timer);
}
#endif /*LV_USE_DEMO_MUSIC*/
