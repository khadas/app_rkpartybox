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

#ifndef INCLUDE_RK_MPI_KARAOKE_H_
#define INCLUDE_RK_MPI_KARAOKE_H_

#include "rk_comm_karaoke.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

RK_S32 RK_MPI_KARAOKE_Create(void **ctx, KARAOKE_ATTR_S *attr);
RK_S32 RK_MPI_KARAOKE_Destroy(void *ctx);
RK_S32 RK_MPI_KARAOKE_SetDataSource(void *ctx, const char *url, const char *headers);
RK_S32 RK_MPI_KARAOKE_StartBTPlayer(void *ctx, KARAOKE_AUDIO_ATTR_S *attr);
RK_S32 RK_MPI_KARAOKE_StopBTPlayer(void *ctx);
RK_S32 RK_MPI_KARAOKE_StartPlayer(void *ctx);
RK_S32 RK_MPI_KARAOKE_PausePlayer(void *ctx);
RK_S32 RK_MPI_KARAOKE_ResumePlayer(void *ctx);
RK_S32 RK_MPI_KARAOKE_StopPlayer(void *ctx);
RK_S32 RK_MPI_KARAOKE_GetPlayerCurrentPosition(void *ctx, RK_S64 *usec);
RK_S32 RK_MPI_KARAOKE_GetPlayerDuration(void *ctx, RK_S64 *usec);
RK_S32 RK_MPI_KARAOKE_SetPlayerLooping(void *ctx, RK_BOOL loop);
RK_S32 RK_MPI_KARAOKE_SetPlayerSeekTo(void *ctx, RK_S64 usec);
RK_S32 RK_MPI_KARAOKE_SetPlayerVolume(void *ctx, RK_U32 volume);
RK_S32 RK_MPI_KARAOKE_GetPlayerVolume(void *ctx, RK_U32 *volume);
RK_S32 RK_MPI_KARAOKE_SetPlayerParam(void *ctx, KARAOKE_PARAM_S *param);
RK_S32 RK_MPI_KARAOKE_GetPlayerParam(void *ctx, KARAOKE_PARAM_S *param);
RK_S32 RK_MPI_KARAOKE_GetPlayerEnergyLevel(void *ctx, KARAOKE_ENERGY_LEVEL_S *energy);
RK_S32 RK_MPI_KARAOKE_ReleasePlayerEnergyLevel(void *ctx, KARAOKE_ENERGY_LEVEL_S *energy);

RK_S32 RK_MPI_KARAOKE_StartRecorder(void *ctx);
RK_S32 RK_MPI_KARAOKE_StopRecorder(void *ctx);
RK_S32 RK_MPI_KARAOKE_MuteRecorder(void *ctx, RK_BOOL mute);
RK_S32 RK_MPI_KARAOKE_SetRecorderVolume(void *ctx, RK_U32 volume);
RK_S32 RK_MPI_KARAOKE_GetRecorderVolume(void *ctx, RK_U32 *volume);
RK_S32 RK_MPI_KARAOKE_SetRecorderParam(void *ctx, KARAOKE_PARAM_S *param);
RK_S32 RK_MPI_KARAOKE_GetRecorderParam(void *ctx, KARAOKE_PARAM_S *param);

RK_S32 RK_MPI_KARAOKE_StartAudioPlayer(void *ctx, KARAOKE_AUDIO_ATTR_S *attr);
RK_S32 RK_MPI_KARAOKE_StopAudioPlayer(void *ctx);
RK_S32 RK_MPI_KARAOKE_SendAudioFrame(void *ctx, AUDIO_FRAME_S *frame, RK_S32 s32MilliSec);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  // INCLUDE_RK_MPI_KARAOKE_H_

