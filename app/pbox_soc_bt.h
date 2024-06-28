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
    PBOX_SOCBT_DSP_EQ_SWITCH_CMD,

    //event
    PBOX_SOCBT_DSP_VERSION_EVT = 0x100,
    PBOX_SOCBT_DSP_MAIN_VOLUME_EVT,
    PBOX_SOCBT_DSP_PLACEMENT_EVT,
    PBOX_SOCBT_DSP_MIC1_STATE_EVT,
    PBOX_SOCBT_DSP_MIC2_STATE_EVT,
    PBOX_SOCBT_DSP_IN_OUT_DOOR_EVT,
    PBOX_SOCBT_DSP_POWER_ON_EVT,
    PBOX_SOCBT_DSP_STEREO_MODE_EVT,
    PBOX_SOCBT_DSP_VOCAL_FADEOUT_EVT,
    PBOX_SOCBT_DSP_VOCAL_RATIO_EVT,
    PBOX_SOCBT_DSP_ACCOMP_RATIO_EVT,
    PBOX_SOCBT_DSP_RESERV_RATIO_EVT,
    PBOX_SOCBT_DSP_SWITCH_SOURCE_EVT,
    PBOX_SOCBT_DSP_MUSIC_VOLUME_EVT,
    PBOX_SOCBT_DSP_MIC_DATA_EVT,
    PBOX_SOCBT_DSP_LIGHT_BAR_VOLUME_EVT,
    PBOX_SOCBT_DSP_LIGHT_BAR_MODE_EVT,
    PBOX_SOCBT_DSP_LIGHT_BAR_POWER_ONOFF_EVT,
    PBOX_SOCBT_DSP_LIGHT_STROBE_CTRL_EVT,
    PBOX_SOCBT_DSP_LIGHT_PARTY_ONOFF_EVT,
    PBOX_SOCBT_DSP_EQ_BASS_ONOFF_EVT,
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
        equalizer_t eqmode;
        mic_mux_t micMux;
        placement_t placement;
        inout_door_t outdoor;
        stereo_mode_t stereo;
        bool fadeout;
        mic_data_t micdata;
        pbox_vocal_t vocal;
        uint8_t level;
        uint8_t mode;
        uint8_t strobe;
        uint8_t bass;
        uint8_t party;
        struct socbt_input_source {
            play_status_t status;
            input_source_t input;
        } input_source;
    };
} pbox_socbt_msg_t;

int pbox_create_btsoc_task(void);
int pbox_stop_btsoc_task(void);

const struct NotifyFuncs* pbox_socbt_get_notify_funcs(void);
#ifdef __cplusplus
}
#endif
#endif