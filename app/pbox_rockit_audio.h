#ifndef _PTBOX_ROCKIT_AUDIO_H_
#define _PTBOX_ROCKIT_AUDIO_H_

#include <stdint.h>
#include <stdbool.h>
#include "os_utils.h"
#include "os_minor_type.h"
#include "pbox_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PROMPT_STEREO,
    PROMPT_MONO,
    PROMPT_WIDEN,
    PROMPT_FADE_ON,
    PROMPT_FADE_OFF,
    PROMPT_GUITAR_FADE_ON,
    PROMPT_GUITAR_FADE_OFF,
    PROMPT_INOUT_SENCE,
    PROMPT_DOA_SENCE,
    PROMPT_ANTI_BACK_ON,
    PROMPT_ANTI_BACK_OFF,
    PROMPT_DIGIT_ZERO,
    PROMPT_DIGIT_ONE,
    PROMPT_DIGIT_TWO,
    PROMPT_DIGIT_THREE,
    PROMPT_DIGIT_FOUR,
    PROMPT_DIGIT_FIVE,
    PROMPT_NUM
} prompt_audio_t;

struct rockit_pbx_t {
    rc_pb_ctx *pboxCtx;
    int signfd[2];
    os_task_t* auxPlayerTask;
    //os_sem_t* auxplay_stop_sem;
    os_task_t* uacRecordTask;
    //os_sem_t* rec_stop_sem;
    pbox_audioFormat_t audioFormat;
};

void *pbox_rockit_record_routine(void *arg);
void* pbox_rockit_aux_player_routine(void *arg);

#ifdef __cplusplus
}
#endif
#endif//_PBOX_ROCKIT_H_