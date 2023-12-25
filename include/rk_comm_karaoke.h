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

#ifndef INCLUDE_RT_COMM_KARAOKE_H_
#define INCLUDE_RT_COMM_KARAOKE_H_

#include "rk_type.h"
#include "rk_comm_aio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum rkKARAOKE_EVT_E {
    KARAOKE_EVT_PLAYBACK_ERROR = 0,
    KARAOKE_EVT_PLAYBACK_COMPLETE,
    KARAOKE_EVT_AWAKEN,
    KARAOKE_EVT_BUTT
} KARAOKE_EVT_E;

typedef enum rkKARAOKE_PARAM_E {
    KARAOKE_PARAM_EQDRC = 0,
    KARAOKE_PARAM_REVERB,
    KARAOKE_PARAM_VOLCAL_SEPARATE,
    KARAOKE_PARAM_3A,
    KARAOKE_PARAM_TYPE_BUTT
} KARAOKE_PARAM_E;

typedef enum rkKARAOKE_REVERB_MODE_E {
    KARAOKE_REVERB_MODE_USER = 0,
    KARAOKE_REVERB_MODE_STUDIO,
    KARAOKE_REVERB_MODE_KTV,
    KARAOKE_REVERB_MODE_CONCERT,
    KARAOKE_REVERB_MODE_BUTT
} KARAOKE_REVERB_MODE_E;

typedef enum rkKARAOKE_WAKE_UP_CMD_E {
    KARAOKE_WAKE_UP_CMD_START_PLAYER = 1,
    KARAOKE_WAKE_UP_CMD_PAUSE_PLARER,
    KARAOKE_WAKE_UP_CMD_STOP_PLARER,
    KARAOKE_WAKE_UP_CMD_PREV,
    KARAOKE_WAKE_UP_CMD_NEXT,
    KARAOKE_WAKE_UP_CMD_VOLUME_UP,
    KARAOKE_WAKE_UP_CMD_VOLUME_DOWN,
    KARAOKE_WAKE_UP_CMD_ORIGINAL_SINGER_OPEN,
    KARAOKE_WAKE_UP_CMD_ORIGINAL_SINGER_CLOSE,

    KARAOKE_WAKE_UP_CMD_RECIEVE = 100,
    KARAOKE_WAKE_UP_CMD_RECIEVE_BUT_NO_TASK,

    KARAOKE_WAKE_UP_CMD_BUTT
} KARAOKE_WAKE_UP_CMD_E;

typedef struct rkKARAOKE_3A_PARAM_S {
    RK_BOOL bBypass;
} KARAOKE_3A_PARAM_S;

typedef struct rkKARAOKE_EQ_DRC_PARAM_S {
    RK_BOOL bBypass;
} KARAOKE_EQ_DRC_PARAM_S;

typedef struct _RTKaraokeReverbParam {
    RK_BOOL bBypass;
    KARAOKE_REVERB_MODE_E enMode;
} RTKaraokeReverbParam;

typedef struct rkKARAOKE_VOCAL_SEPARATE_PARAM_S {
    RK_BOOL bBypass;
    RK_U32  u32HumanLevel;         /* RW; Range: [0, 100];*/
    RK_U32  u32OtherLevel;         /* RW; Range: [0, 100];*/
    RK_U32  u32ReserveLevel[32];   /* RW; Range: [0, 100];*/
} KARAOKE_VOCAL_SEPARATE_PARAM_S;

typedef struct rkKARAOKE_ENERGY_LEVEL_S {
    RK_S16  *ps16EnergyVec;
    RK_VOID *pFrame;
} KARAOKE_ENERGY_LEVEL_S;

typedef struct rkKARAOKE_PARAM_S {
    KARAOKE_PARAM_E enType;
    union {
        KARAOKE_EQ_DRC_PARAM_S           stEqDrcParam;
        RTKaraokeReverbParam             stReverbParam;
        KARAOKE_VOCAL_SEPARATE_PARAM_S   stVolcalSeparateParam;
        KARAOKE_3A_PARAM_S               st3AParam;
    };
} KARAOKE_PARAM_S;

typedef struct rkKARAOKE_AUDIO_ATTR_S {
    RK_U32 u32SampleRate;
    RK_U32 u32Channels;
    RK_U32 u32BitWidth;
} KARAOKE_AUDIO_ATTR_S;

typedef RK_VOID (*RK_KARAOKE_Callback)(RK_VOID *pPrivateData, KARAOKE_EVT_E event, RK_S32 ext1, RK_VOID *ptr);
typedef struct rkRTKaraokeAttr {
    RK_CHAR             *pRecorderCardName;
    RK_CHAR             *pPlayerCardName;
    RK_U32               u32AIChannels;
    RK_U32               u32AOChannels;
    RK_U32               u32ChnLayout;
    RK_U32               u32RefLayout;
    RK_U32               u32RecLayout;
    RK_U32               u32PlayerEnergyBandCnt;
    RK_KARAOKE_Callback  callback;
    RK_VOID             *pPrivateData;
} KARAOKE_ATTR_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  // INCLUDE_RT_COMM_KARAOKE_H_

