/*
 * Copyright 2020 Rockchip Electronics Co. LTD
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
#ifndef SRC_INCLUDE_UAC_CONTROL_H_
#define SRC_INCLUDE_UAC_CONTROL_H_

#include "uac_common_def.h"
#ifdef __cplusplus
extern "C" {
#endif

struct UACControl {
    void (*uacRoleChange)(uint32_t, bool);
    void (*uacSetSampleRate)(uint32_t, uint32_t);
    void (*uacSetVolume)(uint32_t, uint32_t);
    void (*uacSetMute)(uint32_t, bool);
    void (*uacSetPpm)(uint32_t, int32_t);
    void (*adbSetConnect)(bool);
};

void uac_role_change(int mode, bool start);
void uac_set_sample_rate(int mode, int samplerate);
void uac_set_volume(int mode, int volume);
void uac_set_mute(int mode, bool mute);
void uac_set_ppm(int mode, int ppm);
void adb_set_connect(bool connect);

void uac_control_create(struct UACControl *uac);
void uac_control_destory();
#ifdef __cplusplus
}
#endif
#endif  // SRC_INCLUDE_UAC_CONTROL_H_
