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
static void handleSwitchSourceEvent(const pbox_socbt_msg_t *msg);
static void handleMusicVolumeEvent(const pbox_socbt_msg_t *msg);

int unix_socket_socbt_send(void *info, int length)
{
	return unix_socket_send_cmd(PBOX_CHILD_BT, info, length);
}

void pbox_app_btsoc_reply_dsp_version(char *dspver) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_VERSION_CMD,
    };

    strncpy(msg.fw_ver, dspver, MAX_SHORT_NAME_LENGTH);
    msg.fw_ver[MAX_SHORT_NAME_LENGTH] = 0;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_reply_main_volume(uint32_t volume) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_MAIN_VOLUME_CMD,
    };

    msg.volume = volume;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_reply_placement(placement_t placement) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_PLACEMENT_CMD,
    };

    msg.placement = placement;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_reply_inout_door(inout_door_t inout){
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_IN_OUT_DOOR_CMD,
    };

    msg.outdoor = inout;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_reply_poweron(bool poweron){
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_POWER_ON_CMD,
    };

    msg.poweron = 1;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_reply_stereo_mode(stereo_mode_t mode){
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_STEREO_MODE_CMD,
    };

    msg.stereo = mode;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_reply_human_voice_fadeout(bool fadeout){
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_HUMAN_VOICE_FADEOUT_CMD,
    };

    msg.fadeout = fadeout;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_reply_input_source_with_playing_status(input_source_t source, play_status_t status){
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_SWITCH_SOURCE_CMD,
    };

    msg.input_source.input = source;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
    msg.input_source.status = status;
}

void pbox_app_btsoc_reply_music_volume(uint32_t level) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_MUSIC_VOLUME_CMD,
    };

    msg.musicVolLevel = level;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_init(void) {
    pbox_app_btsoc_reply_poweron(true);
//    pbox_app_btsoc_reply_main_volume(pboxUIdata->mainVolumeLevel);
    pbox_app_btsoc_reply_music_volume(pboxUIdata->musicVolumeLevel);
}

void handleDspVersionEvent(const pbox_socbt_msg_t *msg) {
    printf("%s DSP Version: %s\n", __func__, msg->fw_ver);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_dsp_version(DISP_All);
        return;
    }
}

void handleMainVolumeEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Main Volume: %f\n", __func__, msg->volume);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_volume(DISP_All);
        return;
    }

    if(pboxUIdata->mainVolumeLevel != msg->volume)
    pbox_app_music_set_volume(msg->volume, DISP_All);
}

void handlePlacementEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Placement: %u\n", __func__, msg->placement);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_placement(DISP_All);
        return;
    }
    pbox_app_btsoc_set_placement(msg->placement, DISP_All);
}

void handleMic1MuxEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Mic State: %d\n", __func__, msg->micMux);
    if(msg->op == OP_READ) {
        //add implement
        return;
    }

    if(pboxUIdata->micData[0].micMux != msg->micMux)
    pbox_app_music_set_mic_mux(0, msg->micMux, DISP_All);
}

void handleMic2MuxEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Mic State: %d\n", __func__, msg->micMux);
    if(msg->op == OP_READ) {
        //add implement
        return;
    }

    if(pboxUIdata->micData[1].micMux != msg->micMux)
    pbox_app_music_set_mic_mux(1, msg->micMux, DISP_All);
}

void handleInOutDoorEvent(const pbox_socbt_msg_t *msg) {
    printf("%s In/Out Door: %d\n", __func__, msg->outdoor);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_inout_door(DISP_All);
        return;
    }
    //pbox_app_btsoc_set_outdoor_mode(msg->outdoor, DISP_All);
    pbox_app_btsoc_set_human_voice_fadeout(msg->outdoor?false:true, DISP_All);
}

void handlePowerOnEvent(const pbox_socbt_msg_t *msg) {
    printf("%s\n", __func__);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_poweron(DISP_All);
        return;
    }
}

void handleStereoModeEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Sound Mode: %d\n", __func__, msg->stereo);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_stereo_mode(DISP_All);
        return;
    }

    pbox_app_btsoc_set_stereo_mode(msg->stereo, DISP_All);
}

void handleHumanVoiceFadeoutEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Human VoiceFadeout: %u\n", __func__, msg->fadeout);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_human_voice_fadeout(DISP_All);
        return;
    }

    pbox_app_btsoc_set_human_voice_fadeout(msg->fadeout, DISP_All);
}

void handleSwitchSourceEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Switch Source: Play Status = %d, Input Source = %d\n", 
                __func__, msg->input_source.status, msg->input_source.input);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_input_source(DISP_All);
        return;
    }

    pbox_app_btsoc_set_input_source(msg->input_source.input, msg->input_source.status, DISP_All);
}

void handleMusicVolumeEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Music Volume Volume: %f\n", __func__, msg->musicVolLevel);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_music_volume(DISP_All);
        return;
    }

    if(pboxUIdata->musicVolumeLevel != msg->musicVolLevel)
    pbox_app_btsoc_set_music_volume(msg->musicVolLevel, DISP_All);
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
    { PBOX_SOCBT_DSP_MIC1_STATE_EVT,    handleMic1MuxEvent    },
    { PBOX_SOCBT_DSP_MIC2_STATE_EVT,    handleMic2MuxEvent    },
    { PBOX_SOCBT_DSP_IN_OUT_DOOR_EVT,   handleInOutDoorEvent    },
    { PBOX_SOCBT_DSP_POWER_ON_EVT,      handlePowerOnEvent      },
    { PBOX_SOCBT_DSP_STEREO_MODE_EVT,   handleStereoModeEvent   },
    { PBOX_SOCBT_DSP_HUMAN_VOICE_FADEOUT_EVT,   handleHumanVoiceFadeoutEvent   },
    { PBOX_SOCBT_DSP_SWITCH_SOURCE_EVT, handleSwitchSourceEvent },
    { PBOX_SOCBT_DSP_MUSIC_VOLUME_EVT,  handleMusicVolumeEvent  },
};

void btsoc_main_data_recv(const pbox_socbt_msg_t* msg) {
    if (msg == NULL) {
        printf("Error: Null event message received.\n");
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

    printf("Warning: No handle found for event ID %d.\n", msg->msgId);
}

void maintask_btsoc_fd_process(int fd) {
    char buff[sizeof(pbox_socbt_msg_t)] = {0};

    int ret = recv(fd, buff, sizeof(buff), 0);

    if (ret <= 0) {
        if (ret == 0) {
            printf("%s: Connection closed\n", __func__);
        } else if (errno != EINTR) {
            perror("recvfrom");
        }
        return;
    }

    pbox_socbt_msg_t *msg = (pbox_socbt_msg_t *)buff;
    printf("%s sock recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

    if (msg->type != PBOX_EVT)
        return;

    btsoc_main_data_recv(msg);

    return;
}