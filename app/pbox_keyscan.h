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

#if ENABLE_SARAADC==0
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
};

struct dot_vol_control
{
    int		vol_step;
    int		dot_vol;
    int		codec_vol;
};

int  pbox_create_KeyScanTask(void);

int find_event_dev(int event_type);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif /* C++ */

#endif
