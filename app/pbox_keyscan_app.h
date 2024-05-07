#ifndef _PTBOX_KEYSCAN_APP_H_
#define _PTBOX_KEYSCAN_APP_H_
#include <stdint.h>
#include <stdbool.h>
#include "pbox_common.h"
#include <sys/time.h>
#include <linux/input.h>
#ifdef __cplusplus
extern "C" {
#endif

void maintask_keyscan_fd_process(int fd);
int  pbox_create_KeyProcessTask(void);

#ifdef __cplusplus
}
#endif
#endif