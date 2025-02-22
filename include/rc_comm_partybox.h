/*
 * Copyright 2023 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __INCLUDE_RC_PARTYBOX_ATTR_H__
#define __INCLUDE_RC_PARTYBOX_ATTR_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define rc_bool               char
#define rc_true               1
#define rc_false              0

typedef unsigned char         rc_u8;
typedef unsigned short        rc_u16;      // NOLINT
typedef unsigned int          rc_u32;
typedef unsigned long long    rc_u64;      // NOLINT
typedef signed char           rc_s8;
typedef signed short          rc_s16;      // NOLINT
typedef signed int            rc_s32;
typedef long long             rc_s64;      // NOLINT
typedef float                 rc_float;
typedef double                rc_double;
typedef void*                 rc_pb_ctx;
typedef void*                 rc_pb_frame;

#define PB_MAX_AUDIO_CHN_NUM  8

enum rc_pb_filter_pos {
    RC_PB_FILTER_POS_AFTER_CORE0 = 0,
    RC_PB_FILTER_POS_AFTER_CORE1,
    RC_PB_FILTER_POS_AFTER_CORE2,
    RC_PB_FILTER_POS_BUTT
};

enum rc_pb_play_src {
    RC_PB_PLAY_SRC_LOCAL = 0,
    RC_PB_PLAY_SRC_BT,
    RC_PB_PLAY_SRC_UAC,
    RC_PB_PLAY_SRC_PCM,
    RC_PB_PLAY_SRC_BUTT
};

enum rc_pb_rec_src {
    RC_PB_REC_SRC_MIC = 0,
    RC_PB_REC_SRC_GUITAR,
    RC_PB_REC_SRC_BUTT
};

enum rc_pb_event {
    RC_PB_EVENT_PLAYBACK_ERROR = 0,
    RC_PB_EVENT_PLAYBACK_COMPLETE,
    RC_PB_EVENT_AWAKEN,
    RC_PB_EVENT_BUTT
};

enum rc_pb_gt_card_type {
    RC_PB_GUITAR_CARD_TYPE_IND = 0,
    RC_PB_GUITAR_CARD_TYPE_COMBO,
    RC_PB_GUITAR_CARD_TYPE_BUTT
};

enum rc_pb_param_type {
    RC_PB_PARAM_TYPE_3A = 0,
    RC_PB_PARAM_TYPE_REVERB,
    RC_PB_PARAM_TYPE_VOLCAL_SEPARATE,
    RC_PB_PARAM_TYPE_AMIX,
    RC_PB_PARAM_TYPE_RKSTUDIO,
    RC_PB_PARAM_TYPE_SCENE,
    RC_PB_PARAM_TYPE_BUTT
};

enum rc_pb_reverb_mode {
    RC_PB_REVERB_MODE_USER = 0,
    RC_PB_REVERB_MODE_STUDIO,
    RC_PB_REVERB_MODE_KTV,
    RC_PB_REVERB_MODE_CONCERT,
    RC_PB_REVERB_MODE_ECHO,
    RC_PB_REVERB_MODE_BUTT
};

enum rc_pb_ref_mode {
    RC_PB_REF_MODE_NONE = 0,
    RC_PB_REF_MODE_SOFT,
    RC_PB_REF_MODE_HARD_COMBO,
    RC_PB_REF_MODE_HARD_IND
};

enum rc_pb_rkstudio_cmd {
    RC_PB_RKSTUDIO_CMD_DOWNLOAD_GRAPH = 0,
    RC_PB_RKSTUDIO_CMD_SET_PARAM,
    RC_PB_RKSTUDIO_CMD_GET_PARAM,
    RC_PB_RKSTUDIO_CMD_BUTT
};

enum rc_pb_wake_up_cmd {
    RC_PB_WAKE_UP_CMD_START_PLAYER = 1,
    RC_PB_WAKE_UP_CMD_PAUSE_PLARER,
    RC_PB_WAKE_UP_CMD_STOP_PLARER,
    RC_PB_WAKE_UP_CMD_PREV,
    RC_PB_WAKE_UP_CMD_NEXT,
    RC_PB_WAKE_UP_CMD_VOLUME_UP,
    RC_PB_WAKE_UP_CMD_VOLUME_DOWN,
    RC_PB_WAKE_UP_CMD_ORIGINAL_SINGER_OPEN,
    RC_PB_WAKE_UP_CMD_ORIGINAL_SINGER_CLOSE,

    RC_PB_WAKE_UP_CMD_RECIEVE = 100,
    RC_PB_WAKE_UP_CMD_RECIEVE_BUT_NO_TASK,

    RC_PB_WAKE_UP_CMD_BUTT
};

enum rc_pb_scene_detect_mode {
    RC_PB_SCENE_MODE_DOA    = 1 << 0,
    RC_PB_SCENE_MODE_REVERB = 1 << 1,
    RC_PB_SCENE_MODE_GENDER = 1 << 2
};

struct rc_pb_param_howling {
    rc_bool bypass;
};

struct rc_pb_param_reverb {
    rc_bool bypass;
    enum rc_pb_reverb_mode mode;
    rc_s32 dry_level;
    rc_s32 wet_level;
};

struct rc_pb_param_vocal_separate {
    rc_bool bypass;
    rc_u32  human_level;         /* RW; Range: [0, 100]; */
    rc_u32  other_level;         /* RW; Range: [0, 100]; */
    rc_u32  reserve_level[32];   /* RW; Range: [0, 100]; */
    const char *lib_name;
};

struct rc_pb_param_scene_detect {
    rc_bool bypass;
    enum rc_pb_scene_detect_mode scene_mode;
    rc_float result;
};

struct rc_pb_param_amix {
    rc_u32      card;
    const char *control;
    const char *values;
};

struct rc_pb_param_rkstudio {
    rc_bool bypass;
    const char *uri;
    enum rc_pb_rkstudio_cmd cmd;
    rc_u32    id;
    rc_u32    addr;
    rc_float *data;
    rc_u32    cnt;
};

struct rc_pb_param_level_detect {
    rc_u32 rms_tc;
    rc_u32 hold_time;
    rc_u32 decay_time;
    rc_u32 detect_per_frm;  /* RW; default 10 */
    rc_u32 band_cnt;
};

struct rc_pb_param {
    enum rc_pb_param_type type;
    union {
        struct rc_pb_param_howling        howling;
        struct rc_pb_param_reverb         reverb;
        struct rc_pb_param_vocal_separate vocal;
        struct rc_pb_param_amix           amix;
        struct rc_pb_param_rkstudio       rkstudio;
        struct rc_pb_param_scene_detect   scene;
    };
};

struct rc_pb_energy {
    rc_float    *energy_vec;
    rc_pb_frame  frame;
};

struct rc_pb_frame_info {
    rc_pb_frame frame;
    void       *data;
    rc_u32      size;
    rc_u32      sample_rate;
    rc_u32      channels;
    rc_u32      bit_width;
    rc_u32      seq;
    rc_u64      time_stamp;
};

struct rc_pb_player_attr {
    const char *url;
    const char *headers;
    char       *card_name;
    rc_u32      sample_rate;
    rc_u32      channels;
    rc_u32      bit_width;
    rc_u32      valid_bit_width;
    rc_u32      valid_start_bit;
    rc_u32      pool_size;
    rc_u32      pool_cnt;
    rc_u32      mute_ms;
    rc_bool     basic;
    struct rc_pb_param_level_detect detect;
};

struct rc_pb_gt_attr_ind {
    char   *card_name;
    rc_u32  sample_rate;
    rc_u32  channels;
    rc_u32  bit_width;
    rc_u32  mute_ms;
};

struct rc_pb_gt_attr_combo {
    rc_u32  channel_status[PB_MAX_AUDIO_CHN_NUM];  /* RW; 0:not guitar 1:guitar */
};

struct rc_pb_recorder_gt_attr {
    enum rc_pb_gt_card_type type;
    union {
        struct rc_pb_gt_attr_ind    independent;
        struct rc_pb_gt_attr_combo  combo;
    };
};

struct rc_pb_recorder_ref_ind_attr {
    char   *card_name;
    rc_u32  sample_rate;
    rc_u32  channels;
    rc_u32  bit_width;
    rc_u32  mute_ms;
};

struct rc_pb_recorder_attr {
    char   *card_name;
    rc_u32  sample_rate;
    rc_u32  channels;
    rc_u32  bit_width;
    rc_u32  chn_layout;
    rc_u32  ref_layout;
    rc_u32  rec_layout;
    rc_u32  pool_cnt;  /* RW; 0 means no need get frame, can reduce cpu usage */
    rc_u32  mute_ms;
    struct rc_pb_param_level_detect detect;
    enum rc_pb_ref_mode ref_mode;
    struct rc_pb_recorder_gt_attr *guitar;
    struct rc_pb_recorder_ref_ind_attr *ref;
};

struct rc_pb_scene_detect_attr {
    char   *card_name;
    rc_u32  sample_rate;
    rc_u32  channels;
    rc_u32  bit_width;
    rc_u32  ref_layout;
    rc_u32  rec_layout;
    rc_u32  mute_ms;
    enum rc_pb_ref_mode ref_mode;
    enum rc_pb_scene_detect_mode scene_mode;
};

typedef void (*notifyfun_t)(enum rc_pb_event event, rc_s32 cmd, void *opaque);
struct rc_pb_attr {
    char                       *card_name;
    rc_u32                      sample_rate;
    rc_u32                      channels;
    rc_u32                      bit_width;
    notifyfun_t                 notify;
    void                       *opaque;
    rc_float                    volume_db;
    struct rc_pb_recorder_attr *record_attr;
};

struct rc_pb_filter_attr {
    rc_u32 in_bit_width;
    rc_u32 in_sample_rate;
    rc_u32 in_channels;
    rc_u32 out_bit_width;
    rc_u32 out_sample_rate;
    rc_u32 out_channels;
    rc_u32 frame_cnt;
};

struct rc_pb_filter_param {
    void   *in_data;
    rc_u32  in_len;

    void   *out_data;
    rc_u32  out_len;
};

struct rc_pb_filter {
    rc_s32 (*open)(struct rc_pb_filter_attr *attr, void *filter);
    rc_s32 (*process)(void *filter, struct rc_pb_filter_param *param);
    rc_s32 (*flush)(void *filter);
    rc_s32 (*close)(void *filter);
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  // __INCLUDE_RC_PARTYBOX_ATTR_H__

