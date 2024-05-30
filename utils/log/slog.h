// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_PBOX_LOGGER_H_
#define SRC_PBOX_LOGGER_H_
#include <stdbool.h>
#include <stdint.h>
#include "os_minor_type.h"

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
uint32_t covert2debugLevel(const char *str);

#define ALOGD(format, ...)                                                      \
  do {                                                                          \
    unsigned int now = os_get_boot_time_ms();                                 \
    if (get_pbox_log_level() < LOG_LEVEL_DEBUG)                                  \
      break;                                                                    \
      fprintf(stderr, "[%s][%04d.%03d]:" format, LOG_TAG, now/1000, now%1000,      \
              ##__VA_ARGS__);                                                   \
  } while (0)

#define ALOGI(format, ...)                                                  \
  do {                                                                         \
    unsigned int now = os_get_boot_time_ms();                                 \
    if (get_pbox_log_level() < LOG_LEVEL_INFO)                                    \
      break;                                                                   \
      fprintf(stderr, "[%s][%04d.%03d]:" format, LOG_TAG, now/1000, now%1000,      \
              ##__VA_ARGS__);                                                  \
  } while (0)

#define ALOGW(format, ...)                                                  \
  do {                                                                         \
    unsigned int now = os_get_boot_time_ms();                                 \
    if (get_pbox_log_level() < LOG_LEVEL_WARN)                                \
      break;                                                                   \
      fprintf(stderr, "[%s][%04d.%03d]:" format, LOG_TAG, now/1000, now%1000,      \
              ##__VA_ARGS__);                                                  \
  } while (0)

#define ALOGE(format, ...)                                                 \
  do {                                                                         \
    unsigned int now = os_get_boot_time_ms();                                 \
    if (get_pbox_log_level() < LOG_LEVEL_ERROR)                               \
      break;                                                                   \
      fprintf(stderr, "[%s][%04d.%03d]:" format, LOG_TAG, now/1000, now%1000,    \
              ##__VA_ARGS__);                                                  \
  } while (0)

#define LOG_DEBUG(format, ...) ALOGD(format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) ALOGI(format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) ALOGW(format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) ALOGE(format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif  //  SRC_INCLUDE_UAC_LOGGER_H_
