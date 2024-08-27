/*
 * Copyright 2024 Rockchip Electronics Co. LTD
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

#ifndef __INCLUDE_RC_PARTYBOX_H__
#define __INCLUDE_RC_PARTYBOX_H__

#include "rc_comm_partybox.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#if 0
rc_s32 rc_pb_register_filter(enum rc_pb_filter_pos pos, struct rc_pb_filter *filter, void *arg);
rc_s32 rc_pb_unregister_filter(enum rc_pb_filter_pos pos);
rc_s32 rc_pb_create(rc_pb_ctx *ctx, struct rc_pb_attr *attr);
rc_s32 rc_pb_destroy(rc_pb_ctx ctx);
rc_s32 rc_pb_set_volume(rc_pb_ctx ctx, rc_float volume_db);
rc_s32 rc_pb_get_volume(rc_pb_ctx ctx, rc_float *volume_db);
rc_s32 rc_pb_set_param(rc_pb_ctx ctx, struct rc_pb_param *param);
rc_s32 rc_pb_get_param(rc_pb_ctx ctx, struct rc_pb_param *param);

rc_s32 rc_pb_player_start(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_player_attr *attr);
rc_s32 rc_pb_player_stop(rc_pb_ctx ctx, enum rc_pb_play_src src);
rc_s32 rc_pb_player_pause(rc_pb_ctx ctx, enum rc_pb_play_src src);
rc_s32 rc_pb_player_resume(rc_pb_ctx ctx, enum rc_pb_play_src src);
rc_s32 rc_pb_player_dequeue_frame(rc_pb_ctx ctx, enum rc_pb_play_src src,
                                  struct rc_pb_frame_info *frame_info, rc_s32 ms);
rc_s32 rc_pb_player_queue_frame(rc_pb_ctx ctx, enum rc_pb_play_src src,
                                struct rc_pb_frame_info *frame_info, rc_s32 ms);

rc_s32 rc_pb_player_get_position(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_s64 *usec);
rc_s32 rc_pb_player_get_duration(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_s64 *usec);
rc_s32 rc_pb_player_set_loop(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_bool loop);
rc_s32 rc_pb_player_seek(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_s64 usec);
rc_s32 rc_pb_player_set_volume(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_float volume_db);
rc_s32 rc_pb_player_get_volume(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_float *volume_db);
rc_s32 rc_pb_player_set_param(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_param *param);
rc_s32 rc_pb_player_get_param(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_param *param);
rc_s32 rc_pb_player_get_energy(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_energy *energy);
rc_s32 rc_pb_player_release_energy(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_energy *energy);

rc_s32 rc_pb_recorder_start(rc_pb_ctx ctx);
rc_s32 rc_pb_recorder_stop(rc_pb_ctx ctx);
rc_s32 rc_pb_recorder_mute(rc_pb_ctx ctx, enum rc_pb_rec_src src, rc_s32 idx, rc_bool mute);
rc_s32 rc_pb_recorder_set_volume(rc_pb_ctx ctx, enum rc_pb_rec_src src, rc_s32 idx, rc_float volume_db);
rc_s32 rc_pb_recorder_get_volume(rc_pb_ctx ctx, enum rc_pb_rec_src src, rc_s32 idx, rc_float *volume_db);
rc_s32 rc_pb_recorder_set_param(rc_pb_ctx ctx, enum rc_pb_rec_src src, rc_s32 idx, struct rc_pb_param *param);
rc_s32 rc_pb_recorder_get_param(rc_pb_ctx ctx, enum rc_pb_rec_src src, rc_s32 idx, struct rc_pb_param *param);
rc_s32 rc_pb_recorder_get_energy(rc_pb_ctx ctx, enum rc_pb_rec_src src, rc_s32 idx, struct rc_pb_energy *energy);
rc_s32 rc_pb_recorder_release_energy(rc_pb_ctx ctx, enum rc_pb_rec_src src, rc_s32 idx, struct rc_pb_energy *energy);
rc_s32 rc_pb_recorder_dequeue_frame(rc_pb_ctx ctx, enum rc_pb_rec_src src,
                                    struct rc_pb_frame_info *frame_info, rc_s32 ms);
rc_s32 rc_pb_recorder_queue_frame(rc_pb_ctx ctx, enum rc_pb_rec_src src,
                                  struct rc_pb_frame_info *frame_info, rc_s32 ms);

rc_s32 rc_pb_scene_detect_start(rc_pb_ctx ctx, struct rc_pb_scene_detect_attr *attr);
rc_s32 rc_pb_scene_detect_stop(rc_pb_ctx ctx);
rc_s32 rc_pb_scene_get_result(rc_pb_ctx ctx, enum rc_pb_scene_detect_mode mode, rc_float *result);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  // __INCLUDE_RC_PARTYBOX_H__

