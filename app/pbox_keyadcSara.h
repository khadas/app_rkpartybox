#ifndef _PBOX_KEYADCSARA_H_
#define _PBOX_KEYADCSARA_H_
#include <stdbool.h>
#include <stdint.h>
#include "pbox_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ENABLE_SARAADC
typedef enum MIC_BUTTON {
    MIC1_BUTTON_BASS,
    MIC1_BUTTON_TREBLE,
    MIC1_BUTTON_REVERB,
    MIC2_BUTTON_BASS,
    MIC2_BUTTON_TREBLE,
    MIC2_BUTTON_REVERB,
    KNOB_BUTTON_NUM,
    KEY_BUTTON_NUM=KNOB_BUTTON_NUM,
} keycode_t;

typedef enum {
    //event
    PBOX_KEYSCAN_BUTTON_EVENT = 0x100,
    PBOX_KEYSCAN_KNOB_EVENT,
} pbox_keyscan_opcode_t;

typedef struct {
    pbox_msg_t type;
    pbox_keyscan_opcode_t msgId;
    union {
        struct _keyinfo {
                keycode_t keycode;
                union {
                    uint32_t value;
                    uint32_t direction;
                };
        } keyinfo;
    };
}  pbox_keyscan_msg_t;

int pbox_create_KeyadcSaraTask(void);
#endif

#ifdef __cplusplus
}
#endif
#endif



