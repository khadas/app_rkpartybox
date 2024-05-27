/*
 *  Copyright (c) 2020 Rockchip Electronics Co.Ltd
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef _GET_KEYSCAN_H_
#define _GET_KEYSCAN_H_

#include "pbox_keyscan_app.h"
#ifdef __cplusplus
extern "C" {
#endif

enum event_type
{
    FUNC_KEY_WAKEUP  = 0,
    FUNC_KEY_WIFI_MODE,
    FUNC_KEY_VOL_DOWN,
    FUNC_KEY_VOL_UP,
    FUNC_KEY_MIC_MUTE,
    FUNC_KEY_BT_MODE,
    FUNC_KEY_NORMAL_MODE,
    FUNC_KEY_MIC_UNMUTE,
    FUNC_VOLUME_CHANGE,
    FUNC_LAST_ID,
};

enum support_event_type
{
    EVENT_START = 0,
    KEY_EVENT = EVENT_START, //this is the first type of event
    ROTARY_EVENT,
    EVENT_END = ROTARY_EVENT,
};

struct dot_support_event_type
{
    int		event_type;
    char		*name; /*event name, used to judge this event is supported or not*/
    int key_type;
};

struct dot_vol_control
{
    int		vol_step;
    int		dot_vol;
    int		codec_vol;
};

#define MAX_KEY_BUFFERED	8

#define KEY_DOUBLE_CLICK_PERIOD        (300*1000)   //双击键间隔时间，单位US
#define KEY_LONG_PRESS_PREIOD           3000        //长按时间间隔，单位ms
#define KEY_SWITCH_MSEC                 6000
#define KEY_VERY_LONG_PRESS_PERIOD      10000       //超长按时间间隔，单位ms

typedef struct {
    int key_code;
    int key_code_b;
    key_press_t press_type;
    int is_key_valid;
    float value;
    uint64_t unix_time;//from 1970.01.01 in ms
} pbox_keyevent_msg_t;

struct dot_key
{
    int key_code;
    int key_code_b;
    key_press_t press_type;//0表示短按，1表示大于3s的长按，2表示大于10s的超长按，3表示长按组合键，4表示快速双击
    int is_key_valid;
    int is_combain_key; //combian key
    /*
     * pre_alexa_mode is used if only one key to switch from different mode, such as wifi/bt/mic_mute mode.
     * we can define the previous alexa mode as ALEXA_WIFI_MODE if we want to press one key to switch from
     * wifi mode to other mode.
     * if pre_alexa_mode defined as ALEXA_INVALID_MODE, means this value is not used
     */
    int (*key_process)(float);
    uint64_t ptime;
    uint64_t utime;
};

int  pbox_create_KeyScanTask(void);

int find_event_dev(int event_type);

#ifdef __cplusplus
} /* extern "C" */
#endif /* C++ */

#endif
