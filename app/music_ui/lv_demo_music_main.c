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

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * main_cont;
static lv_obj_t * chart_obj;
static lv_chart_series_t * serdata;
static lv_obj_t * title_label;
static lv_obj_t * artist_label;
static lv_obj_t * genre_label;
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

//karaoke control
extern lv_ft_info_t ttf_main_s;
extern lv_ft_info_t ttf_main_m;
extern lv_ft_info_t ttf_main_l;
int32_t mHumanLevel=15, mMusicLevel=100, mGuitarLevel = 100;
int32_t mVolumeLevel=50, mMicVolumeLevel=50;
//bool mEchoReductionEnable = true;
bool mVocalSeperateEnable = false;
lv_obj_t * vocal_label = NULL;
lv_obj_t * accomp_label = NULL;
lv_obj_t * guitar_label = NULL;
lv_obj_t * volume_label = NULL;
lv_obj_t * volume_slider = NULL;
lv_obj_t * mic_volume_label = NULL;
lv_obj_t * accomp_slider = NULL;
lv_obj_t * guitar_slider = NULL;
lv_obj_t * vocal_slider = NULL;
lv_obj_t *origin_switch = NULL;
lv_obj_t *echo_3a_switch = NULL;
lv_style_t accomp_style_disabled;
lv_style_t vocal_style_disabled;
lv_style_t guitar_style_disabled;

/**********************
 *      MACROS
 **********************/

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
    uint32_t id = track_id;
    uint32_t track_num = _lv_demo_music_get_track_num();
    play_status_t play_status = pboxUIdata->play_status;

    printf("%s, next:%d\n", __func__, next);
    lcd_pbox_notifyPrevNext(next);
#if 0
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

    if(play_status == PLAYING) {
        _lv_demo_music_play(id);
    } else {
        track_load(id);   //load the next track
        play_status = _STOP;  //player status change from pause  to stop
    }
#endif
}

void _lv_demo_music_update_track_info(uint32_t id) {
    play_status_t play_status = pboxUIdata->play_status;
    if (play_status == IDLE) {
         lv_label_set_text(title_label, _lv_demo_music_get_title(id));
         lv_label_set_text(artist_label, _lv_demo_music_get_artist(id));
         lv_label_set_text(genre_label, _lv_demo_music_get_genre(id));
    }
}

void _lv_demo_music_play(uint32_t id)
{
    track_load(id);
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
        case UI_WIDGET_MAIN_VOLUME: {
            uint32_t mainVolume = msg->mainVolume;
            char buf[16];
                    lv_snprintf(buf, sizeof(buf), "主音量 %d", mainVolume);
            lv_label_set_text(volume_label, buf);
            lv_slider_set_value(volume_slider, mainVolume, LV_ANIM_OFF);
        } break;
        case UI_WIDGET_TRACK_INFO: {
            char *artist = (char*)(msg->track.artist);
            char *title = (char*)(msg->track.title);
            if(artist && (strlen(artist)>0))
                lv_label_set_text(artist_label, artist);
            if(title && (strlen(title)>0))
                lv_label_set_text(title_label, title);
        } break;
        case UI_WIDGET_POSITION_INFO: {
            static int  prev_total = 0;
            uint32_t current = msg->positions.mCurrent;
            uint32_t total = msg->positions.mDuration;
            uint32_t onlytotal = msg->positions.onlyDuration;
            printf("%s position:[%d]-[%d](%d)\n", __func__, current, total, prev_total);
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
        case UI_WIDGET_VOCAL_SEPERATE: {
            pbox_vocal_t vocalSeperate = msg->vocalSeparate;
            bool seperateEnable = vocalSeperate.enable;
            if(seperateEnable) {
                lv_obj_add_state(origin_switch, LV_STATE_CHECKED);
                lv_obj_clear_state(accomp_slider, LV_STATE_DISABLED);
                lv_obj_clear_state(vocal_slider, LV_STATE_DISABLED);
                if (guitar_slider != NULL)
                    lv_obj_clear_state(guitar_slider, LV_STATE_DISABLED);
            }
            else {
                lv_obj_clear_state(origin_switch, LV_STATE_CHECKED);
                lv_obj_add_state(accomp_slider, LV_STATE_DISABLED);
                lv_obj_add_state(vocal_slider, LV_STATE_DISABLED);
                if (guitar_slider != NULL)
                    lv_obj_add_state(guitar_slider, LV_STATE_DISABLED);
            }
        } break;
        case UI_WIDGET_SPECTRUM_CHART: {
            energy_info_t energyData = msg->energy_data;
                    for (int i = 0; i < energyData.size; i++) {
                lv_chart_set_next_value(chart_obj, serdata, energyData.energykeep[i].energy);
                    }
        } break;
        case UI_WIDGET_3A_SWITCH: {
            bool echo3A_On = msg->echo3A_On;
            if(echo3A_On) {
                lv_obj_add_state(echo_3a_switch, LV_STATE_CHECKED);
            }
            else {
                lv_obj_clear_state(echo_3a_switch, LV_STATE_CHECKED);
            }
        } break;
        default:
            break;
    };
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
    lv_obj_set_style_text_color(artist_label, lv_color_hex(0x4472C4/*0x00B06d*/), 0);
    lv_label_set_text(artist_label, _lv_demo_music_get_artist(track_id));
    lv_obj_set_grid_cell(artist_label, LV_GRID_ALIGN_END, 5, 1, LV_GRID_ALIGN_END, 0, 1);

    genre_label = lv_label_create(cont);
    lv_obj_set_style_text_font(genre_label, font_small, 0);
    lv_obj_set_style_text_color(genre_label, lv_color_hex(0x8a86b8), 0);
    lv_label_set_text(genre_label, _lv_demo_music_get_genre(track_id));

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
    else 
         mode = PBOX_REVERT_USER;

    printf("'%s' is selected\n", buf);
    lcd_pbox_notifyReverbMode(mode);
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

static void guitar_slider_event_cb(lv_event_t *e) {
    lv_obj_t * slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "吉他 %d", (int)lv_slider_get_value(slider));

    if (code == LV_EVENT_RELEASED) {
        printf("last slider value%s\n", buf);
        if (guitar_label != NULL) {
            lv_label_set_text(guitar_label, buf);
            mGuitarLevel = (int)lv_slider_get_value(slider);
            lcd_pbox_notifyReservMusicLevel(mGuitarLevel);
        }
    }
}

static void vocal_seperate_event_handler(lv_event_t * e) {
    lv_obj_t * obj = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        printf("State: %s\n", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "On" : "Off");
    }
    mVocalSeperateEnable = lv_obj_has_state(obj, LV_STATE_CHECKED) ? true : false;

    if(mVocalSeperateEnable) {
        lv_obj_clear_state(accomp_slider, LV_STATE_DISABLED);
        lv_obj_clear_state(vocal_slider, LV_STATE_DISABLED);
        if (guitar_slider != NULL)
            lv_obj_clear_state(guitar_slider, LV_STATE_DISABLED);
    }
    else {
        lv_obj_add_state(accomp_slider, LV_STATE_DISABLED);
        lv_obj_add_state(vocal_slider, LV_STATE_DISABLED);
        if (guitar_slider != NULL)
            lv_obj_add_state(guitar_slider, LV_STATE_DISABLED);
    }

    lcd_pbox_notifySeparateSwitch(mVocalSeperateEnable);
}

static void accomp_slider_event_cb(lv_event_t *e) {
    lv_obj_t * slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "伴声 %d", (int)lv_slider_get_value(slider));

    if (code == LV_EVENT_RELEASED) {
        printf("last slider value%s\n", buf);
        lv_label_set_text(accomp_label, buf);
        mMusicLevel = (int)lv_slider_get_value(slider);
        lcd_pbox_notifyAccompMusicLevel(mMusicLevel);
    }
}

static void vocal_slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "原唱 %d", (int)lv_slider_get_value(slider));

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
    lv_snprintf(buf, sizeof(buf), "主音量 %d", (int)lv_slider_get_value(slider));

    if (code == LV_EVENT_RELEASED) {
        printf("master volume value %s\n", buf);
        lv_label_set_text(volume_label, buf);
        mVolumeLevel = (int)lv_slider_get_value(slider);
        lcd_pbox_notifyMainVolLevel(mVolumeLevel);
    }
}

static void mic_volume_change_event_cb(lv_event_t * e) {
    lv_obj_t * slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "麦音量 %d", (int)lv_slider_get_value(slider));

    if (code == LV_EVENT_RELEASED) {
        printf("mic volume value %s\n", buf);
        lv_label_set_text(mic_volume_label, buf);
        mMicVolumeLevel = (int)lv_slider_get_value(slider);
        lcd_pbox_notifyMicVolLevel(mMicVolumeLevel);
    }
}

static lv_obj_t * create_chart_box(lv_obj_t * parent)
{
    /* Create the spectrum chart visualizer */
    lv_obj_t *obj = lv_chart_create(parent);
    lv_chart_set_axis_tick(obj, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 6, 1, true, 80);
    lv_chart_set_axis_tick(obj, LV_CHART_AXIS_PRIMARY_X, 5, 3, 10, 1, true, 50);
    lv_chart_set_type(obj, LV_CHART_TYPE_BAR);
    lv_chart_set_div_line_count(obj, 10, 0);
    lv_chart_set_point_count(obj, 10);
    lv_chart_set_range(obj, LV_CHART_AXIS_PRIMARY_X, 0, 100);
    serdata = lv_chart_add_series(obj, lv_theme_get_color_primary(obj), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_next_value(obj, serdata, 0);
    lv_chart_set_next_value(obj, serdata, 0);
    lv_chart_set_next_value(obj, serdata, 0);
    lv_chart_set_next_value(obj, serdata, 0);
    lv_chart_set_next_value(obj, serdata, 0);
    lv_chart_set_next_value(obj, serdata, 0);
    lv_chart_set_next_value(obj, serdata, 0);
    lv_chart_set_next_value(obj, serdata, 0);
    lv_chart_set_next_value(obj, serdata, 0);
    lv_chart_set_next_value(obj, serdata, 0);
    
    return obj;
}

static lv_obj_t * create_misc_box(lv_obj_t * parent)
{
    char *guitar_mode = getenv("GUITAR_MODE");
    int mode = 0;
    if (guitar_mode)
        mode = atoi(guitar_mode);
    /* Create the misc control box */
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_height(cont, 320);//LV_SIZE_CONTENT);
#if LV_DEMO_MUSIC_LARGE
    lv_obj_set_style_pad_bottom(cont, 17, 0);
#else
    lv_obj_set_style_pad_bottom(cont, 8, 0);
#endif
    static const lv_coord_t misc_grid_col[] = {LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_FR(1),LV_GRID_CONTENT,LV_GRID_FR(1),
                                          LV_GRID_CONTENT, LV_GRID_FR(1),LV_GRID_TEMPLATE_LAST};

    static const lv_coord_t misc_grid_row[] = {LV_GRID_CONTENT, LV_GRID_FR(4), LV_GRID_CONTENT, LV_GRID_FR(4),LV_GRID_CONTENT,
                                                                LV_GRID_FR(4), LV_GRID_CONTENT, LV_GRID_FR(4), LV_GRID_CONTENT,
                                                                LV_GRID_FR(4), LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(cont, misc_grid_col, misc_grid_row);
    lv_obj_set_style_grid_row_align(cont, LV_GRID_ALIGN_SPACE_BETWEEN, 0);

    //reverb control
    lv_obj_t * reverb_label = lv_label_create(cont);
    lv_label_set_text(reverb_label, "混响");
    lv_obj_set_style_text_font(reverb_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(reverb_label, LV_GRID_ALIGN_CENTER, 5, 1, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_t * reverb_dd = lv_dropdown_create(cont);
    lv_obj_set_grid_cell(reverb_dd, LV_GRID_ALIGN_END, 5, 1, LV_GRID_ALIGN_START, 1, 1);
    lv_dropdown_set_options(reverb_dd, "OFF\nSTUDIO\nKTV\nCONCERT");
    lv_dropdown_set_selected(reverb_dd, 3);//0,1,2,3 so 3 means CONCERT
    lv_obj_add_event_cb(reverb_dd, reverb_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * echo_3a_label = lv_label_create(cont);
    lv_label_set_text(echo_3a_label, "3A算法");
    lv_obj_set_style_text_font(echo_3a_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(echo_3a_label, LV_GRID_ALIGN_CENTER, 5, 1, LV_GRID_ALIGN_START, 4, 1);
    echo_3a_switch = lv_switch_create(cont);
    lv_obj_set_grid_cell(echo_3a_switch, LV_GRID_ALIGN_CENTER, 5, 1, LV_GRID_ALIGN_START, 5, 1);
    lv_obj_add_event_cb(echo_3a_switch, echol_3a_seperate_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_state(echo_3a_switch, LV_STATE_CHECKED);

    //原唱与伴奏切换
    lv_obj_t * origin_label = lv_label_create(cont);
    lv_label_set_text(origin_label, "声音分离");
    lv_obj_set_style_text_font(origin_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(origin_label, LV_GRID_ALIGN_CENTER, 5, 1, LV_GRID_ALIGN_START, 8, 1);
    origin_switch = lv_switch_create(cont);
    lv_obj_set_grid_cell(origin_switch, LV_GRID_ALIGN_CENTER, 5, 1, LV_GRID_ALIGN_START, 9, 1);
    lv_obj_add_event_cb(origin_switch, vocal_seperate_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_clear_state(origin_switch, LV_STATE_CHECKED);

    //volume control
    volume_label = lv_label_create(cont);
    lv_label_set_text(volume_label, "主音量 50");
    lv_obj_set_style_text_font(volume_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(volume_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);
    volume_slider = lv_slider_create(cont);
    lv_obj_set_width(volume_slider, LV_PCT(50));
    lv_obj_set_grid_cell(volume_slider, LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_event_cb(volume_slider, master_volume_change_event_cb, LV_EVENT_RELEASED, NULL);
    lv_slider_set_mode(volume_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_range(volume_slider, 0, 100);
    //mVolumeLevel = 50;
    lv_slider_set_value(volume_slider, 50, LV_ANIM_OFF);

    //mic volume control
    mic_volume_label = lv_label_create(cont);
    lv_label_set_text(mic_volume_label, "麦音量 100");
    lv_obj_set_style_text_font(mic_volume_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(mic_volume_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_t * mic_volume_slider = lv_slider_create(cont);
    lv_obj_set_width(mic_volume_slider, LV_PCT(50));
    lv_obj_set_grid_cell(mic_volume_slider, LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    lv_obj_add_event_cb(mic_volume_slider, mic_volume_change_event_cb, LV_EVENT_RELEASED, NULL);
    lv_slider_set_mode(mic_volume_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_range(mic_volume_slider, 0, 100);
    //mMicVolumeLevel = 100;
    lv_slider_set_value(mic_volume_slider, 100, LV_ANIM_OFF);

    //mGuitarLevel = 100;
    if (mode == 1) {
        //accompaniment and vocal control
        guitar_label = lv_label_create(cont);
        lv_label_set_text(guitar_label, "吉他 100");
        //lv_obj_add_style(voice_label, &style_text_muted, 0);
        lv_obj_set_style_text_font(guitar_label, ttf_main_s.font, 0);
        lv_obj_set_grid_cell(guitar_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 4, 1);
        guitar_slider = lv_slider_create(cont);
        lv_obj_set_width(guitar_slider, LV_PCT(50));
        lv_obj_set_grid_cell(guitar_slider, LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 4, 1);
        lv_obj_add_event_cb(guitar_slider, guitar_slider_event_cb, LV_EVENT_RELEASED, NULL);
        lv_slider_set_mode(guitar_slider, LV_SLIDER_MODE_NORMAL);
        lv_slider_set_range(guitar_slider, 0, 100);

        lv_slider_set_value(guitar_slider, 100, LV_ANIM_OFF);
        lv_style_init(&guitar_style_disabled);
        lv_style_set_bg_color(&guitar_style_disabled, lv_color_hex(0xCCCCCC));
        lv_obj_add_style(guitar_slider, &guitar_style_disabled, LV_PART_INDICATOR | LV_STATE_DISABLED);
        lv_obj_add_style(guitar_slider, &guitar_style_disabled, LV_PART_MAIN | LV_STATE_DISABLED);
        lv_obj_add_style(guitar_slider, &guitar_style_disabled, LV_PART_KNOB | LV_STATE_DISABLED);
        lv_obj_add_state(guitar_slider, LV_STATE_DISABLED);
    }

    //accompaniment and vocal control
    accomp_label = lv_label_create(cont);
    lv_label_set_text(accomp_label, "伴声 100");
    //lv_obj_add_style(voice_label, &style_text_muted, 0);
    lv_obj_set_style_text_font(accomp_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(accomp_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 6 - (mode?0:1) * 2, 1);
    accomp_slider = lv_slider_create(cont);
    lv_obj_set_width(accomp_slider, LV_PCT(50));
    lv_obj_set_grid_cell(accomp_slider, LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 6 - (mode?0:1) * 2, 1);
    lv_obj_add_event_cb(accomp_slider, accomp_slider_event_cb, LV_EVENT_RELEASED, NULL);
    lv_slider_set_mode(accomp_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_range(accomp_slider, 0, 100);
    //mMusicLevel = 100;
    lv_slider_set_value(accomp_slider, 100, LV_ANIM_OFF);
    lv_style_init(&accomp_style_disabled);
    lv_style_set_bg_color(&accomp_style_disabled, lv_color_hex(0xCCCCCC));
    lv_obj_add_style(accomp_slider, &accomp_style_disabled, LV_PART_INDICATOR | LV_STATE_DISABLED);
    lv_obj_add_style(accomp_slider, &accomp_style_disabled, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_style(accomp_slider, &accomp_style_disabled, LV_PART_KNOB | LV_STATE_DISABLED);
    lv_obj_add_state(accomp_slider, LV_STATE_DISABLED);

    vocal_label = lv_label_create(cont);
    lv_label_set_text(vocal_label, "原唱 15");
    lv_obj_set_style_text_font(vocal_label, ttf_main_s.font, 0);
    lv_obj_set_grid_cell(vocal_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 8 - (mode?0:1) * 2, 1);
    vocal_slider = lv_slider_create(cont);
    lv_obj_set_width(vocal_slider, LV_PCT(50));
    lv_obj_set_grid_cell(vocal_slider, LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 8 - (mode?0:1) * 2, 1);
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

    _lv_demo_music_list_btn_check(track_id, false);
    track_id = id;
    _lv_demo_music_list_btn_check(id, true);
    lcd_pbox_notifyTrackid(track_id);

    lv_label_set_text(title_label, _lv_demo_music_get_title(track_id));
    lv_label_set_text(artist_label, _lv_demo_music_get_artist(track_id));
    lv_label_set_text(genre_label, _lv_demo_music_get_genre(track_id));
}

static void play_event_click_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
        _lv_demo_music_resume();
    }
    else {
        _lv_demo_music_pause();
    }
}

static void prev_click_event_cb(lv_event_t * e)
{
    LV_UNUSED(e);
    _lv_demo_music_album_next(false);
}

static void next_click_event_cb(lv_event_t * e)
{
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
	   printf("seek to\n");
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

#endif /*LV_USE_DEMO_MUSIC*/
