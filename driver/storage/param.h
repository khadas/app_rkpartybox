// Copyright 2021 Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "iniparser.h"

#ifndef _UTILS_PARAM_H_
#define _UTILS_PARAM_H_

#ifdef __cplusplus
extern "C" {
#endif
extern dictionary *g_ini_d_;

int rk_param_get_int(const char *entry, int default_val);
int rk_param_set_int(const char *entry, int val);
bool rk_param_get_bool(const char *entry, bool default_val);
int rk_param_set_bool(const char *entry, bool val);
float rk_param_get_float(const char *entry, float default_val);
int rk_param_set_float(const char *entry, float val);

double rk_param_get_double(const char *entry, double default_val);

const char *rk_param_get_string(const char *entry, const char *default_val);
int rk_param_set_string(const char *entry, const char *val);
int rk_param_save();
int rk_param_init(char *ini_path);
int rk_param_deinit();
int rk_param_reload();
#ifdef __cplusplus
}
#endif

#endif