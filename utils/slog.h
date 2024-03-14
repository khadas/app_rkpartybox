// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_PBOX_LOGGER_H_
#define SRC_PBOX_LOGGER_H_
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN  1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_DEBUG 3
#define LOG_LEVEL_NUM (LOG_LEVEL_DEBUG+1)

#ifndef LOG_TAG
#define LOG_TAG "pbox_app"
#endif

uint32_t get_pbox_log_level(void);
void set_pbox_log_level(uint32_t level);
uint32_t covert2debugLevel(char *str);

#define ALOGD(format, ...)                                                      \
  do {                                                                          \
    if (get_pbox_log_level() < LOG_LEVEL_DEBUG)                                   \
      break;                                                                    \
      fprintf(stderr, "[%s]:" format, LOG_TAG,                                  \
              ##__VA_ARGS__);                                                   \
  } while (0)

#define ALOGI(format, ...)                                                  \
  do {                                                                         \
    if (get_pbox_log_level() < LOG_LEVEL_INFO)                                    \
      break;                                                                   \
      fprintf(stderr, "[%s]:" format, LOG_TAG,                                  \
              ##__VA_ARGS__);                                                  \
  } while (0)

#define ALOGW(format, ...)                                                  \
  do {                                                                         \
    if (get_pbox_log_level() < LOG_LEVEL_WARN)                                \
      break;                                                                   \
      fprintf(stderr, "[%s]:" format, LOG_TAG,                                  \
              ##__VA_ARGS__);                                                  \
  } while (0)

#define ALOGE(format, ...)                                                 \
  do {                                                                         \
    if (get_pbox_log_level() < LOG_LEVEL_ERROR)                               \
      break;                                                                   \
      fprintf(stderr, "[%s]:" format, LOG_TAG,                                  \
              ##__VA_ARGS__);                                                  \
  } while (0)

#ifdef __cplusplus
}
#endif
#endif  //  SRC_INCLUDE_UAC_LOGGER_H_
