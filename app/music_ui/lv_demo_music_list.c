/**
 * @file lv_demo_music_list.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_music_list.h"
#if LV_USE_DEMO_MUSIC

#include "lv_demo_music_main.h"
extern lv_ft_info_t ttf_main_s;
extern lv_ft_info_t ttf_main_m;
extern lv_ft_info_t ttf_main_l;
//extern lv_style_t style_txt_s;
//extern lv_style_t style_txt_m;
//extern lv_style_t style_txt_l;

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
//extern int track_num;
static lv_obj_t * add_list_btn(lv_obj_t * parent, uint32_t track_id);
static void btn_click_event_cb(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * list;
static const lv_font_t * font_small;
static const lv_font_t * font_medium;
static lv_style_t style_scrollbar;
static lv_style_t style_btn;
static lv_style_t style_btn_pr;
static lv_style_t style_btn_chk;
static lv_style_t style_btn_dis;
static lv_style_t style_title;
static lv_style_t style_artist;
static lv_style_t style_time;
LV_IMG_DECLARE(img_lv_demo_music_btn_list_play);
LV_IMG_DECLARE(img_lv_demo_music_btn_list_pause);

#if 0
static lv_style_t style_txt;
static lv_style_t style_list;
static void style_init(void)
{
    lv_style_init(&style_txt);
    lv_style_set_text_font(&style_txt, ttf_main_s.font);
    lv_style_set_text_color(&style_txt, lv_color_make(0xff, 0x23, 0x23));

    lv_style_init(&style_list);
    lv_style_set_text_font(&style_list, ttf_main_m.font);
    lv_style_set_text_color(&style_list, lv_color_black());
}
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * _lv_demo_music_list_create(lv_obj_t * parent)
{
#if LV_DEMO_MUSIC_LARGE
    font_small = &lv_font_montserrat_16;
    font_medium = &lv_font_montserrat_22;
#else
    font_small = &lv_font_montserrat_12;
    font_medium = &lv_font_montserrat_16;
#endif

    //style_init();

    lv_style_init(&style_scrollbar);
    lv_style_set_width(&style_scrollbar,  4);
    lv_style_set_bg_opa(&style_scrollbar, LV_OPA_COVER);
    lv_style_set_bg_color(&style_scrollbar, lv_color_hex3(0xeee));
    lv_style_set_radius(&style_scrollbar, LV_RADIUS_CIRCLE);
    lv_style_set_pad_right(&style_scrollbar, 4);

    static const lv_coord_t grid_cols[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
#if LV_DEMO_MUSIC_LARGE
    static const lv_coord_t grid_rows[] = {35,  30, LV_GRID_TEMPLATE_LAST};
#else
    static const lv_coord_t grid_rows[] = {22,  17, LV_GRID_TEMPLATE_LAST};
#endif
    lv_style_init(&style_btn);
    lv_style_set_bg_opa(&style_btn, LV_OPA_TRANSP);
    lv_style_set_grid_column_dsc_array(&style_btn, grid_cols);
    lv_style_set_grid_row_dsc_array(&style_btn, grid_rows);
    lv_style_set_grid_row_align(&style_btn, LV_GRID_ALIGN_CENTER);
    lv_style_set_layout(&style_btn, LV_LAYOUT_GRID);
#if LV_DEMO_MUSIC_LARGE
    lv_style_set_pad_right(&style_btn, 30);
#else
    lv_style_set_pad_right(&style_btn, 20);
#endif
    lv_style_init(&style_btn_pr);
    lv_style_set_bg_opa(&style_btn_pr, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn_pr,  lv_color_hex(0x4c4965));

    lv_style_init(&style_btn_chk);
    lv_style_set_bg_opa(&style_btn_chk, LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn_chk, lv_color_hex(0x4c4965));

    lv_style_init(&style_btn_dis);
    lv_style_set_text_opa(&style_btn_dis, LV_OPA_40);
    lv_style_set_img_opa(&style_btn_dis, LV_OPA_40);
#if 0
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_medium);
    lv_style_set_text_color(&style_title, lv_color_hex(0xffffff));
#else
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, ttf_main_m.font);
    lv_style_set_text_color(&style_title, lv_color_black());
#endif

    lv_style_init(&style_artist);
    lv_style_set_text_font(&style_artist, font_small);
    lv_style_set_text_color(&style_artist, lv_color_hex(0xb1b0be));

    lv_style_init(&style_time);
    lv_style_set_text_font(&style_time, font_medium);
    lv_style_set_text_color(&style_time, lv_color_hex(0xffffff));

    /*Create an empty transparent container*/
    list = lv_obj_create(parent);
    lv_obj_remove_style_all(list);
    lv_obj_set_size(list, LV_HOR_RES, LV_VER_RES - LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_set_y(list, LV_DEMO_MUSIC_HANDLE_SIZE);
    lv_obj_add_style(list, &style_scrollbar, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);

    uint32_t track_id;
    uint32_t track_num = _lv_demo_music_get_track_num();
    for(track_id = 0; track_id < track_num; track_id++) {
        add_list_btn(list,  track_id);
    }

#if LV_DEMO_MUSIC_SQUARE || LV_DEMO_MUSIC_ROUND
    lv_obj_set_scroll_snap_y(list, LV_SCROLL_SNAP_CENTER);
#endif

    if (track_num > 0) //scan some mp3 file
        _lv_demo_music_list_btn_check(0, true);

    return list;
}

void _lv_demo_music_update_track_list(lv_obj_t * list) {
    uint32_t track_id;
    uint32_t track_num = _lv_demo_music_get_track_num();
    for(track_id = 0; track_id < track_num; track_id++) {
        add_list_btn(list,  track_id);
    }
}

void _lv_demo_music_list_btn_check(uint32_t track_id, bool state)
{
    lv_obj_t * btn = lv_obj_get_child(list, track_id);
    //lv_obj_t * icon = lv_obj_get_child(btn, 0);

    if(state) {
        lv_obj_add_state(btn, LV_STATE_CHECKED);
       // lv_img_set_src(icon, &img_lv_demo_music_btn_list_pause);
       // lv_obj_scroll_to_view(btn, LV_ANIM_ON);
    }
    else {
        lv_obj_clear_state(btn, LV_STATE_CHECKED);
        //lv_img_set_src(icon, &img_lv_demo_music_btn_list_play);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_obj_t * add_list_btn(lv_obj_t * parent, uint32_t track_id)
{
#if 0
    uint32_t t = _lv_demo_music_get_track_length(track_id);
    char time[32];
    lv_snprintf(time, sizeof(time), "%"LV_PRIu32":%02"LV_PRIu32, t / 60, t % 60);
#endif
    const char * title = _lv_demo_music_get_title(track_id);
    const char * artist = _lv_demo_music_get_artist(track_id);

    lv_obj_t * btn = lv_obj_create(parent);
    lv_obj_remove_style_all(btn);
#if LV_DEMO_MUSIC_LARGE
    lv_obj_set_size(btn, lv_pct(100), 110);
#else
    lv_obj_set_size(btn, lv_pct(100), 60);
#endif

    lv_obj_add_style(btn, &style_btn, 0);
    lv_obj_add_style(btn, &style_btn_pr, LV_STATE_PRESSED);
    lv_obj_add_style(btn, &style_btn_chk, LV_STATE_CHECKED);
    lv_obj_add_style(btn, &style_btn_dis, LV_STATE_DISABLED);
    lv_obj_add_event_cb(btn, btn_click_event_cb, LV_EVENT_CLICKED, NULL);

#if 0
    if(track_id >= 3) {
        lv_obj_add_state(btn, LV_STATE_DISABLED);
    }

    lv_obj_t * icon = lv_img_create(btn);
    lv_img_set_src(icon, &img_lv_demo_music_btn_list_play);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 2);
#endif

    lv_obj_t * title_label = lv_label_create(btn);
    lv_label_set_text(title_label, title);
    lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_add_style(title_label, &style_title, LV_PART_MAIN);

#if 0
    lv_obj_t * artist_label = lv_label_create(btn);
    lv_label_set_text(artist_label, artist);
    lv_obj_add_style(artist_label, &style_artist, 0);
    lv_obj_set_grid_cell(artist_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
#endif

    lv_obj_t * time_label = lv_label_create(btn);
   // lv_label_set_text(time_label, time);
    lv_label_set_text(time_label, "");
    lv_obj_add_style(time_label, &style_time, 0);
    lv_obj_set_grid_cell(time_label, LV_GRID_ALIGN_END, 2, 1, LV_GRID_ALIGN_CENTER, 0, 2);

    LV_IMG_DECLARE(img_lv_demo_music_list_border);
    lv_obj_t * border = lv_img_create(btn);
    lv_img_set_src(border, &img_lv_demo_music_list_border);
    lv_obj_set_width(border, lv_pct(120));
    lv_obj_align(border, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(border, LV_OBJ_FLAG_IGNORE_LAYOUT);

    return btn;
}


static void btn_click_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_target(e);

    uint32_t idx = lv_obj_get_child_id(btn);

    _lv_demo_music_stop();

    _lv_demo_music_play(idx);
}
#endif /*LV_USE_DEMO_MUSIC*/

