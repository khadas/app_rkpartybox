#ifndef _PBOX_KEYADCSARA_APP_H_
#define _PBOX_KEYADCSARA_APP_H_
#include <stdbool.h>
#include <stdint.h>
#include "pbox_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ENABLE_SARAADC
void maintask_keyscan_fd_process(int fd);
#endif

#ifdef __cplusplus
}
#endif
#endif