#ifndef _PTBOX_BT_SOC_H_
#define _PTBOX_BT_SOC_H_

#include "pbox_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    //command
    PBOX_SOCBT_DSP_VERSION_CMD,
    PBOX_SOCBT_DSP_MAIN_VOLUME_CMD,
    PBOX_SOCBT_DSP_PLACEMENT_CMD,
    PBOX_SOCBT_DSP_MIC1_STATE_CMD,
    PBOX_SOCBT_DSP_MIC2_STATE_CMD,
    PBOX_SOCBT_DSP_IN_OUT_DOOR_CMD,
    PBOX_SOCBT_DSP_POWER_ON_CMD,
    PBOX_SOCBT_DSP_STEREO_MODE_CMD,
    PBOX_SOCBT_DSP_HUMAN_VOICE_FADEOUT_CMD,
    PBOX_SOCBT_DSP_SWITCH_SOURCE_CMD,
    PBOX_SOCBT_DSP_MUSIC_VOLUME_CMD,

    //event
    PBOX_SOCBT_DSP_VERSION_EVT = 0x100,
    PBOX_SOCBT_DSP_MAIN_VOLUME_EVT,
    PBOX_SOCBT_DSP_PLACEMENT_EVT,
    PBOX_SOCBT_DSP_MIC1_STATE_EVT,
    PBOX_SOCBT_DSP_MIC2_STATE_EVT,
    PBOX_SOCBT_DSP_IN_OUT_DOOR_EVT,
    PBOX_SOCBT_DSP_POWER_ON_EVT,
    PBOX_SOCBT_DSP_STEREO_MODE_EVT,
    PBOX_SOCBT_DSP_HUMAN_VOICE_FADEOUT_EVT,
    PBOX_SOCBT_DSP_SWITCH_SOURCE_EVT,
    PBOX_SOCBT_DSP_MUSIC_VOLUME_EVT,
} pbox_socbt_opcode_t;

typedef struct {
    pbox_msg_t type;
    pbox_socbt_opcode_t msgId;
    ext_cmd_t op;
    union {
        char fw_ver[MAX_SHORT_NAME_LENGTH+1];
        //char stat[32];
        float volume;
        float musicVolLevel;
        uint32_t poweron;
        mic_mux_t micMux;
        placement_t placement;
        inout_door_t outdoor;
        stereo_mode_t stereo;
        bool fadeout;
        struct socbt_input_source {
            play_status_t status;
            input_source_t input;
        } input_source;
    };
} pbox_socbt_msg_t;

int pbox_create_btsoc_task(void);
#ifdef __cplusplus
}
#endif
#endif