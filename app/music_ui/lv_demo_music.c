/**
 * @file lv_demo_music.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_music.h"

#if LV_USE_DEMO_MUSIC

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lv_demo_music_main.h"
#include "lv_demo_music_list.h"
#include "pbox_app.h"
#include "pbox_common.h"

#include <lvgl/lv_conf.h>
#include "lvgl/lv_port_file.h"
#include "lvgl/lv_port_indev.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if LV_DEMO_MUSIC_AUTO_PLAY
    static void auto_step_cb(lv_timer_t * timer);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * ctrl;
static lv_obj_t * list;

//#define TRACK_MAX_NUM 10
#define MUSIC_PATH "/data/"
#define MAIN_FONT         "/data/SmileySans-Oblique.ttf"
//int track_num = 0;
//char title_list[TRACK_MAX_NUM][256] = {};
char *artist_list[TRACK_MAX_NUM][256] = {};
char *genre_list[TRACK_MAX_NUM][256] = {};
uint32_t time_list[TRACK_MAX_NUM] = {};

lv_ft_info_t ttf_main_s;
lv_ft_info_t ttf_main_m;
lv_ft_info_t ttf_main_l;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
#if 0
static char *strlwr(char *s)
{
  char *p = s;
  for(;*s;s++) {
    *s = tolower(*s);
  }
  return p;
}

static char *strrstr(const char *str, const char *token)
{
    int len = strlen(token);
    const char *p = str + strlen(str);

    while (str <= --p)
        if (p[0] == token[0] && strncmp(p, token, len) == 0)
            return (char *)p;
    return NULL;
}

static int file_is_supported(char *filepath)
{
    int ret = 0;

    if (strstr(filepath, ".") == NULL)
        return 0;

    char *suffix = strlwr(strdup(strrstr(filepath, ".") + 1));
    static const char *formats[] =
    {
        "mp4", "mp3"
    };

    for (int i = 0; i < sizeof(formats) / sizeof(formats[0]); i++) {
        if (strcmp(suffix, formats[i]) == 0) {
            ret = 1;
            break;
        }
    }

    printf("%s is %ssupported, [%s]\n", filepath, ret ? "" : "not ", suffix);

    free(suffix);

    return ret;
}
int scan_dir(const char *path, int depth)
{
    DIR *dir;
    struct dirent *entry;
    int file_count = 0;
    dir = opendir(path);
    if (!dir)
    {
        printf("Error opening directory /oem\n");
        return -1;
    }
    printf("open dir %s ok!", path);
    while ((entry = readdir(dir)) != NULL)
    {
	if (entry->d_type == DT_REG) {
	    if (file_is_supported(entry->d_name)) {
                strcpy(title_list[file_count++], entry->d_name);
		if (file_count > TRACK_MAX_NUM)
		    break;
	    }
	}
    }
    track_num = file_count;
    closedir(dir);
    return 0;
}
#endif
static void font_init(void)
{
    lv_freetype_init(64, 1, 0);

    ttf_main_s.name = MAIN_FONT;
    ttf_main_s.weight = 26;
    ttf_main_s.style = FT_FONT_STYLE_NORMAL;
    lv_ft_font_init(&ttf_main_s);

    ttf_main_m.name = MAIN_FONT;
    ttf_main_m.weight = 36;
    ttf_main_m.style = FT_FONT_STYLE_NORMAL;
    lv_ft_font_init(&ttf_main_m);

    ttf_main_l.name = MAIN_FONT;
    ttf_main_l.weight = 140;
    ttf_main_l.style = FT_FONT_STYLE_NORMAL;
    lv_ft_font_init(&ttf_main_l);
}

static void lvgl_init(void) {
    lv_init();
    hal_drm_init(0, 0, LV_DISP_ROT_NONE);
    lv_port_fs_init();
    lv_port_indev_init(0);
}

void lv_demo_music(void)
{
    lvgl_init();
    font_init();

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x343247), 0);

    //scan_dir(MUSIC_PATH, 3);

    list = _lv_demo_music_list_create(lv_scr_act());
    ctrl = _lv_demo_music_main_create(lv_scr_act());

#if LV_DEMO_MUSIC_AUTO_PLAY
    lv_timer_create(auto_step_cb, 1000, NULL);
#endif
}

int _lv_demo_music_get_track_num() {
    return pboxTrackdata->track_num;
}

int _lv_demo_music_get_track_id() {
    return pboxTrackdata->track_id;
}

void _lv_demo_music_update_list(uint32_t trackId) {
    _lv_demo_music_update_track_list(list);
    _lv_demo_music_update_track_info(trackId);
}

const char *_lv_demo_music_get_title(uint32_t track_id)
{
    if (track_id >= TRACK_MAX_NUM)
        return NULL;
    //return title_list[track_id];
    return pbox_app_usb_get_title(track_id);
}

const char *_lv_demo_music_get_artist(uint32_t track_id)
{
    if (track_id >= TRACK_MAX_NUM)
        return NULL;
    return artist_list[track_id];
}

const char *_lv_demo_music_get_genre(uint32_t track_id)
{
    if (track_id >= TRACK_MAX_NUM)
        return NULL;
    return genre_list[track_id];
}

uint32_t _lv_demo_music_get_track_length(uint32_t track_id)
{
    if (track_id >= TRACK_MAX_NUM)
        return 0;
    return time_list[track_id];
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#if LV_DEMO_MUSIC_AUTO_PLAY
static void auto_step_cb(lv_timer_t * t)
{
    LV_UNUSED(t);
    static uint32_t state = 0;

#if LV_DEMO_MUSIC_LARGE
    const lv_font_t * font_small = &lv_font_montserrat_22;
    const lv_font_t * font_large = &lv_font_montserrat_32;
#else
    const lv_font_t * font_small = &lv_font_montserrat_12;
    const lv_font_t * font_large = &lv_font_montserrat_16;
#endif
    printf("%s state:%d", __func__, state);
    switch(state) {
        case 5:
            _lv_demo_music_album_next(true);
            break;

        case 6:
            _lv_demo_music_album_next(true);
            break;
        case 7:
            _lv_demo_music_album_next(true);
            break;
        case 8:
            _lv_demo_music_play(0);
            break;
#if LV_DEMO_MUSIC_SQUARE || LV_DEMO_MUSIC_ROUND
        case 11:
            lv_obj_scroll_by(ctrl, 0, -LV_VER_RES, LV_ANIM_ON);
            break;
        case 13:
            lv_obj_scroll_by(ctrl, 0, -LV_VER_RES, LV_ANIM_ON);
            break;
#else
        case 12:
            lv_obj_scroll_by(ctrl, 0, -LV_VER_RES, LV_ANIM_ON);
            break;
#endif
        case 15:
            lv_obj_scroll_by(list, 0, -300, LV_ANIM_ON);
            break;
        case 16:
            lv_obj_scroll_by(list, 0, 300, LV_ANIM_ON);
            break;
        case 18:
            _lv_demo_music_play(1);
            break;
        case 19:
            lv_obj_scroll_by(ctrl, 0, LV_VER_RES, LV_ANIM_ON);
            break;
#if LV_DEMO_MUSIC_SQUARE || LV_DEMO_MUSIC_ROUND
        case 20:
            lv_obj_scroll_by(ctrl, 0, LV_VER_RES, LV_ANIM_ON);
            break;
#endif
        case 30:
            _lv_demo_music_play(2);
            break;
        case 40: {
                lv_obj_t * bg = lv_layer_top();
                lv_obj_set_style_bg_color(bg, lv_color_hex(0x6f8af6), 0);
                lv_obj_set_style_text_color(bg, lv_color_white(), 0);
                lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);
                lv_obj_fade_in(bg, 400, 0);
                lv_obj_t * dsc = lv_label_create(bg);
                lv_obj_set_style_text_font(dsc, font_small, 0);
                lv_label_set_text(dsc, "The average FPS is");
                lv_obj_align(dsc, LV_ALIGN_TOP_MID, 0, 90);

                lv_obj_t * num = lv_label_create(bg);
                lv_obj_set_style_text_font(num, font_large, 0);
#if LV_USE_PERF_MONITOR
                lv_label_set_text_fmt(num, "%d", lv_refr_get_fps_avg());
#endif
                lv_obj_align(num, LV_ALIGN_TOP_MID, 0, 120);

                lv_obj_t * attr = lv_label_create(bg);
                lv_obj_set_style_text_align(attr, LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_set_style_text_font(attr, font_small, 0);
#if LV_DEMO_MUSIC_SQUARE || LV_DEMO_MUSIC_ROUND
                lv_label_set_text(attr, "Copyright 2020 LVGL Kft.\nwww.lvgl.io | lvgl@lvgl.io");
#else
                lv_label_set_text(attr, "Copyright 2020 LVGL Kft. | www.lvgl.io | lvgl@lvgl.io");
#endif
                lv_obj_align(attr, LV_ALIGN_BOTTOM_MID, 0, -10);
                break;
            }
        case 41:
            lv_scr_load(lv_obj_create(NULL));
            _lv_demo_music_pause();
            break;
    }
    state++;
}

#endif /*LV_DEMO_MUSIC_AUTO_PLAY*/

#endif /*LV_USE_DEMO_MUSIC*/
