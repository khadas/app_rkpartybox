/*
 * Copyright 2022 Rockchip Electronics Co. LTD
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
 *
 */

#ifndef INCLUDE_RT_MPI_MPI_AMIX_H_
#define INCLUDE_RT_MPI_MPI_AMIX_H_

#include "rk_common.h"
#include "rk_comm_aio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

RK_S32 RK_MPI_AMIX_ListContents(AUDIO_DEV AmixDevId);
RK_S32 RK_MPI_AMIX_ListControls(AUDIO_DEV AmixDevId);
RK_S32 RK_MPI_AMIX_SetControl(AUDIO_DEV AmixDevId, const char *control, char *values);
RK_S32 RK_MPI_AMIX_GetControl(AUDIO_DEV AmixDevId, const char *control);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  /* End of INCLUDE_RT_MPI_MPI_AMIX_H_ */
