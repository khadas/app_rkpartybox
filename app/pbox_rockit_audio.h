#ifndef _PTBOX_ROCKIT_AUDIO_H_
#define _PTBOX_ROCKIT_AUDIO_H_

#include <stdint.h>
#include <stdbool.h>
#include "pbox_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rockit_pbx_t {
    rc_pb_ctx *pboxCtx;
    int signfd[2];
    pthread_t aux_player_tid;
    os_task_t uacRecordTask;
    pbox_audioFormat_t audioFormat;
};

void *pbox_rockit_record_routine(void *params);

#ifdef __cplusplus
}
#endif
#endif//_PBOX_ROCKIT_H_