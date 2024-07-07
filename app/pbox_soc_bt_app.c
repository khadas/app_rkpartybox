#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "pbox_app.h"
#include "pbox_soc_bt.h"
#include "pbox_soc_bt_app.h"
#include "pbox_socketpair.h"

typedef void (*socbt_event_handle)(const pbox_socbt_msg_t*);

static void handleDspVersionEvent(const pbox_socbt_msg_t *msg);
static void handleMainVolumeEvent(const pbox_socbt_msg_t *msg);
static void handlePlacementEvent(const pbox_socbt_msg_t *msg);
static void handleMic1MuxEvent(const pbox_socbt_msg_t *msg);
static void handleMic2MuxEvent(const pbox_socbt_msg_t *msg);
static void handleInOutDoorEvent(const pbox_socbt_msg_t *msg);
static void handlePowerOnEvent(const pbox_socbt_msg_t *msg);
static void handleStereoModeEvent(const pbox_socbt_msg_t *msg);
static void handleHumanVoiceFadeoutEvent(const pbox_socbt_msg_t *msg);
static void handleVocalHumanRatioEvent(const pbox_socbt_msg_t *msg);
static void handleVocalAccompRatioEvent(const pbox_socbt_msg_t *msg);
static void handleVocalReservRatioEvent(const pbox_socbt_msg_t *msg);
static void handleSwitchVocalModeEvent(const pbox_socbt_msg_t *msg);
static void handleSwitchSourceEvent(const pbox_socbt_msg_t *msg);
static void handleMusicVolumeEvent(const pbox_socbt_msg_t *msg);
static void handleLightBarVolumeEvent(const pbox_socbt_msg_t *msg);
static void handleLightBarModeEvent(const pbox_socbt_msg_t *msg);
static void handleLightBarPowerOnoffEvent(const pbox_socbt_msg_t *msg);
static void handleStrobeCtrlEvent(const pbox_socbt_msg_t *msg);
static void handleLightPartyOnoffEvent(const pbox_socbt_msg_t *msg);
static void handleEqBassOnoffEvent(const pbox_socbt_msg_t *msg);

int unix_socket_socbt_send(void *info, int length)
{
    return unix_socket_send_cmd(PBOX_CHILD_BT, info, length);
}

void pbox_app_btsoc_echo_dsp_version(char *dspver) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_VERSION_CMD,
    };

    snprintf(msg.fw_ver, MAX_SHORT_NAME_LENGTH, "%s", dspver);
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_echo_main_volume(float volume) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_MAIN_VOLUME_CMD,
    };

    msg.volume = volume;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_echo_eq_mode(equalizer_t mode) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_EQ_SWITCH_CMD,
    };

    msg.eqmode = mode;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_echo_placement(placement_t placement) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_PLACEMENT_CMD,
    };

    msg.placement = placement;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_echo_inout_door(inout_door_t inout){
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_IN_OUT_DOOR_CMD,
    };

    msg.outdoor = inout;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_echo_poweron(bool poweron){
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_POWER_ON_CMD,
    };

    msg.poweron = 1;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_echo_stereo_mode(stereo_mode_t mode){
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_STEREO_MODE_CMD,
    };

    msg.stereo = mode;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_echo_human_voice_fadeout(bool fadeout){
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_HUMAN_VOICE_FADEOUT_CMD,
    };

    msg.fadeout = fadeout;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_echo_input_source_with_playing_status(input_source_t source, play_status_t status){
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_SWITCH_SOURCE_CMD,
    };

    msg.input_source.input = source;
    msg.input_source.status = status;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_echo_music_volume(float level) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_MUSIC_VOLUME_CMD,
    };

    msg.musicVolLevel = level;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void handleDspVersionEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s DSP Version: %s\n", __func__, msg->fw_ver);
    if(msg->op == OP_READ) {
        pbox_app_echo_dsp_version(DISP_BTMCU);
        return;
    }
}

void handleMainVolumeEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Main Volume: %f\n", __func__, msg->volume);
    if(msg->op == OP_READ) {
        pbox_app_echo_main_volume(DISP_BTMCU);
        return;
    }

    pbox_app_btsoc_set_volume(msg->volume, DISP_All_EXCLUDE_BTMCU);
}

void handlePlacementEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Placement: %u\n", __func__, msg->placement);
    if(msg->op == OP_READ) {
        pbox_app_echo_placement(DISP_BTMCU);
        return;
    }

    pbox_app_btsoc_set_placement(msg->placement, DISP_All_EXCLUDE_BTMCU);
}

void handleMic1MuxEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Mic State: %d\n", __func__, msg->micMux);
    if(msg->op == OP_READ) {
        pbox_app_echo_micMux(0, DISP_BTMCU);
        return;
    }

    pbox_app_btsoc_set_mic_mux(0, msg->micMux, DISP_All_EXCLUDE_BTMCU);
}

void handleMic2MuxEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Mic State: %d\n", __func__, msg->micMux);
    if(msg->op == OP_READ) {
        pbox_app_echo_micMux(1, DISP_BTMCU);
        return;
    }

    pbox_app_btsoc_set_mic_mux(1, msg->micMux, DISP_All_EXCLUDE_BTMCU);
}

void handleInOutDoorEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s In/Out Door: %d\n", __func__, msg->outdoor);
    if(msg->op == OP_READ) {
        pbox_app_echo_inout_door(DISP_BTMCU);
        return;
    }

    pbox_app_btsoc_set_outdoor_mode(msg->outdoor, DISP_All_EXCLUDE_BTMCU);
    //tmp coding, for vendor board no human voice button..
    //pbox_app_btsoc_set_human_voice_fadeout(msg->outdoor?false:true, DISP_All_EXCLUDE_BTMCU);
}

void handleMicDataEvent(const pbox_socbt_msg_t *msg) {
    uint8_t index = msg->micdata.index;
    mic_set_kind_t kind = msg->micdata.kind;

    ALOGD("%s op:%d Mic[%d] kind: %d\n", __func__, msg->op, index, kind);
    if(msg->op == OP_READ) {
        pbox_app_echo_micdata(index, kind, DISP_BTMCU);
        return;
    }

    pbox_app_btsoc_set_mic_data(msg->micdata, DISP_All_EXCLUDE_BTMCU);
}

void handlePowerOnEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s\n", __func__);
    if(msg->op == OP_READ) {
        pbox_app_echo_poweron_status(DISP_BTMCU);
        return;
    }
}

void handleStereoModeEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Sound Mode: %d\n", __func__, msg->stereo);
    if(msg->op == OP_READ) {
        pbox_app_echo_stereo_mode(DISP_BTMCU);
        return;
    }

    pbox_app_btsoc_set_stereo_mode(msg->stereo, DISP_All_EXCLUDE_BTMCU);
}

void handleHumanVoiceFadeoutEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Human VoiceFadeout: %u\n", __func__, msg->fadeout);
    if(msg->op == OP_READ) {
        pbox_app_echo_voice_fadeout_mode(DISP_BTMCU);
        return;
    }

    pbox_app_btsoc_set_human_voice_fadeout(msg->fadeout, DISP_All_EXCLUDE_BTMCU);
}

void handleVocalHumanRatioEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s  human ratio=: %u\n", __func__, msg->vocal.humanLevel);
    if(msg->op == OP_READ) {
        return;
    }

    if(pboxUIdata->vocallib != VOCAL_HUMAN) {
        pboxUIdata->vocallib = VOCAL_HUMAN;
    }
    pbox_app_music_set_human_music_level(msg->vocal.humanLevel, DISP_All);
}

void handleVocalAccompRatioEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s  accomp ratio=: %u\n", __func__, msg->vocal.accomLevel);
    if(msg->op == OP_READ) {
        return;
    }

    if(pboxUIdata->vocallib != VOCAL_HUMAN) {
        pboxUIdata->vocallib = VOCAL_HUMAN;
    }
    pbox_app_music_set_accomp_music_level(msg->vocal.accomLevel, DISP_All);
}

void handleVocalReservRatioEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s  reserv ratio=: %u\n", __func__, msg->vocal.reservLevel);
    if(msg->op == OP_READ) {
        return;
    }

    if(pboxUIdata->vocallib != VOCAL_GUITAR) {
        pboxUIdata->vocallib = VOCAL_GUITAR;
    }
    pbox_app_music_set_reserv_music_level(msg->vocal.reservLevel, DISP_All);
}

void handleSwitchVocalModeEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s  switch to vocal mode=: %u\n", __func__, msg->mode);
    if(msg->op == OP_READ) {
        return;
    }

    pbox_app_set_dest_vocal_lib(msg->mode);
}

void handleSwitchSourceEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Switch Source: Play Status = %d, Input Source = %d\n", 
                __func__, msg->input_source.status, msg->input_source.input);
    if(msg->op == OP_READ) {
        pbox_app_echo_input_source(DISP_BTMCU);
        return;
    }

    pbox_app_btsoc_set_input_source(msg->input_source.input, msg->input_source.status, DISP_All);
}

void handleMusicVolumeEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Music Volume Volume: %f\n", __func__, msg->musicVolLevel);
    if(msg->op == OP_READ) {
        pbox_app_echo_music_volume(DISP_BTMCU);
        return;
    }

    pbox_app_btsoc_set_music_volume(msg->musicVolLevel, DISP_All_EXCLUDE_BTMCU|DISP_FS);
}

void handleLightBarVolumeEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Light Bar Volume: %d\n", __func__, msg->volume);
    if (msg->op == OP_READ) {
        return;
    }
    pbox_app_btsoc_set_music_volume(msg->volume, DISP_All);
}

void handleLightBarModeEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Light Bar Mode: %d\n", __func__, msg->mode);
    if (msg->op == OP_READ) {
        return;
    }
    pbox_app_light_bar_set_mode(msg->mode, DISP_All);
}

void handleLightBarPowerOnoffEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Light Bar Power On/Off: %d\n", __func__, msg->poweron);
    if (msg->op == OP_READ) {
        return;
    }
    pbox_app_light_bar_set_power_onoff(msg->poweron, DISP_All);
}

void handleStrobeCtrlEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Strobe Control: %d\n", __func__, msg->strobe);
    if (msg->op == OP_READ) {
        return;
    }
    pbox_app_light_strobe_ctrl(msg->strobe, DISP_All);
}

void handleLightPartyOnoffEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s Light Party On/Off: %d\n", __func__, msg->party);
    if (msg->op == OP_READ) {
        return;
    }
    pbox_app_party_light_enable(msg->party, DISP_All);
}

void handleEqBassOnoffEvent(const pbox_socbt_msg_t *msg) {
    ALOGD("%s EQ Bass On/Off: %d\n", __func__, msg->bass);
    if (msg->op == OP_READ) {
        return;
    }
    pbox_app_eq_bass_set_onoff(msg->bass, DISP_All);
}

// Define a struct to associate opcodes with handles
typedef struct {
    pbox_socbt_opcode_t opcode;
    socbt_event_handle handle;
} socbt_event_handle_t;

const socbt_event_handle_t socbtEventTable[] = {
    { PBOX_SOCBT_DSP_VERSION_EVT,       handleDspVersionEvent   },
    { PBOX_SOCBT_DSP_MAIN_VOLUME_EVT,   handleMainVolumeEvent   },
    { PBOX_SOCBT_DSP_PLACEMENT_EVT,     handlePlacementEvent    },
    { PBOX_SOCBT_DSP_MIC1_STATE_EVT,    handleMic1MuxEvent      },
    { PBOX_SOCBT_DSP_MIC2_STATE_EVT,    handleMic2MuxEvent      },
    { PBOX_SOCBT_DSP_IN_OUT_DOOR_EVT,   handleInOutDoorEvent    },
    { PBOX_SOCBT_DSP_MIC_DATA_EVT,      handleMicDataEvent      },
    { PBOX_SOCBT_DSP_POWER_ON_EVT,      handlePowerOnEvent      },
    { PBOX_SOCBT_DSP_STEREO_MODE_EVT,   handleStereoModeEvent   },
    { PBOX_SOCBT_DSP_VOCAL_FADEOUT_EVT, handleHumanVoiceFadeoutEvent },
    { PBOX_SOCBT_DSP_VOCAL_RATIO_EVT,   handleVocalHumanRatioEvent   },
    { PBOX_SOCBT_DSP_ACCOMP_RATIO_EVT,  handleVocalAccompRatioEvent  },
    { PBOX_SOCBT_DSP_RESERV_RATIO_EVT,  handleVocalReservRatioEvent  },
    { PBOX_SOCBT_DSP_SWITCH_VOCAL_EVT,  handleSwitchVocalModeEvent  },
    { PBOX_SOCBT_DSP_SWITCH_SOURCE_EVT, handleSwitchSourceEvent },
    { PBOX_SOCBT_DSP_MUSIC_VOLUME_EVT,  handleMusicVolumeEvent  },
    { PBOX_SOCBT_DSP_LIGHT_BAR_VOLUME_EVT, handleLightBarVolumeEvent },
    { PBOX_SOCBT_DSP_LIGHT_BAR_MODE_EVT, handleLightBarModeEvent },
    { PBOX_SOCBT_DSP_LIGHT_BAR_POWER_ONOFF_EVT, handleLightBarPowerOnoffEvent },
    { PBOX_SOCBT_DSP_LIGHT_STROBE_CTRL_EVT, handleStrobeCtrlEvent },
    { PBOX_SOCBT_DSP_LIGHT_PARTY_ONOFF_EVT, handleLightPartyOnoffEvent },
    { PBOX_SOCBT_DSP_EQ_BASS_ONOFF_EVT, handleEqBassOnoffEvent }
};

void btsoc_main_data_recv(const pbox_socbt_msg_t* msg) {
    if (msg == NULL) {
        ALOGD("Error: Null event message received.\n");
        return;
    }

    for (int i = 0; i < sizeof(socbtEventTable) / sizeof(socbtEventTable[0]); ++i) {
        if (socbtEventTable[i].opcode == msg->msgId) {

            if (socbtEventTable[i].handle != NULL) {
                socbtEventTable[i].handle(msg);
            }
            return;
        }
    }

    ALOGD("Warning: No handle found for event ID %d.\n", msg->msgId);
}

void maintask_btsoc_fd_process(int fd) {
    char buff[sizeof(pbox_socbt_msg_t)] = {0};

    int ret = recv(fd, buff, sizeof(buff), 0);

    if (ret <= 0) {
        if (ret == 0) {
            ALOGW("%s: Connection closed\n", __func__);
        } else if (errno != EINTR) {
            perror("recvfrom");
        }
        return;
    }

    pbox_socbt_msg_t *msg = (pbox_socbt_msg_t *)buff;
    ALOGD("%s sock recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

    if (msg->type != PBOX_EVT)
        return;

    btsoc_main_data_recv(msg);

    return;
}