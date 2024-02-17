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
static void handleMic1StateEvent(const pbox_socbt_msg_t *msg);
static void handleMic2StateEvent(const pbox_socbt_msg_t *msg);
static void handleInOutDoorEvent(const pbox_socbt_msg_t *msg);
static void handlePowerOnEvent(const pbox_socbt_msg_t *msg);
static void handleStereoModeEvent(const pbox_socbt_msg_t *msg);
static void handleHumanSplitEvent(const pbox_socbt_msg_t *msg);
static void handleSwitchSourceEvent(const pbox_socbt_msg_t *msg);
static void handleMusicGroundEvent(const pbox_socbt_msg_t *msg);

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

void pbox_app_btsoc_reply_placement(uint32_t placement) {
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

    msg.inout_door = inout;
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

    msg.stereo_mode = mode;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_reply_human_split(uint32_t level){
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_HUMAN_SPLIT_CMD,
    };

    msg.human_level = level;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_reply_input_source_with_playing_status(input_source_t source, play_status_t status){
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_SWITCH_SOURCE_CMD,
    };

    msg.input_source.input = source;
    msg.input_source.status = status;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void pbox_app_btsoc_reply_accom_level(uint32_t level) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_SOCBT_DSP_MUSIC_GROUND_CMD,
    };

    msg.accom_level = level;
    unix_socket_socbt_send(&msg, sizeof(pbox_socbt_msg_t));
}

void handleDspVersionEvent(const pbox_socbt_msg_t *msg) {
    printf("%s DSP Version: %s\n", __func__, msg->fw_ver);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_dsp_version(DISP_All);
        return;
    }
}

void handleMainVolumeEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Main Volume: %u\n", __func__, msg->volume);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_volume(DISP_All);
        return;
    }

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

void handleMic1StateEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Mic State: %d\n", __func__, msg->mic_state[0]);
    if(msg->op == OP_READ) {
        //add implement
        return;
    }
}

void handleMic2StateEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Mic State: %d\n", __func__, msg->mic_state[1]);
    if(msg->op == OP_READ) {
        //add implement
        return;
    }
}

void handleInOutDoorEvent(const pbox_socbt_msg_t *msg) {
    printf("%s In/Out Door: %d\n", __func__, msg->inout_door);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_inout_door(DISP_All);
        return;
    }
    pbox_app_btsoc_set_inout_door(msg->inout_door, DISP_All);
}

void handlePowerOnEvent(const pbox_socbt_msg_t *msg) {
    printf("%s\n", __func__);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_poweron(DISP_All);
        return;
    }

    uint32_t stereo_mode= msg->stat[0] & 0xf;
    uint32_t inout_door = msg->stat[0] >> 4;
    uint32_t volume     = msg->stat[1]*100/32;
    uint32_t accom_level= msg->stat[2]*100/32;
    mic_state_t mic1    = msg->stat[3];
    mic_state_t mic2    = msg->stat[4];
    uint32_t placement  = msg->stat[5];
    uint32_t human_level= msg->stat[6]? 0:100;
    play_status_t status = ((msg->stat[7]>>7)&0x01) ? _STOP:PLAYING;
    input_source_t source;
    switch(msg->stat[7]&0x1F) {
        case 0: {
            source = SRC_BT;
        } break;
#if ENABLE_AUX
        case 1: {
            source = SRC_AUX;
        } break;
#endif
        case 2: {
            source = SRC_USB;
        } break;
    }
    pbox_app_btsoc_set_stereo_mode(stereo_mode, DISP_All);
    pbox_app_btsoc_set_inout_door(inout_door, DISP_All);
    pbox_app_music_set_volume(volume, DISP_All);
    pbox_app_btsoc_set_accom_level(accom_level, DISP_All);
    pbox_app_btsoc_set_placement(placement, DISP_All);
    pbox_app_btsoc_set_human_split(human_level, DISP_All);
    pbox_app_btsoc_set_input_source(source, status, DISP_All);
}

void handleStereoModeEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Sound Mode: %d\n", __func__, msg->stereo_mode);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_stereo_mode(DISP_All);
        return;
    }

    pbox_app_btsoc_set_stereo_mode(msg->stereo_mode, DISP_All);
}

void handleHumanSplitEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Human Split Level: %u\n", __func__, msg->human_level);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_human_split(DISP_All);
        return;
    }

    pbox_app_btsoc_set_human_split(msg->human_level, DISP_All);
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

void handleMusicGroundEvent(const pbox_socbt_msg_t *msg) {
    printf("%s Music Ground Volume: %u\n", __func__, msg->accom_level);
    if(msg->op == OP_READ) {
        pbox_app_btsoc_get_accom_level(DISP_All);
        return;
    }

    pbox_app_btsoc_set_accom_level(msg->accom_level, DISP_All);
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
    { PBOX_SOCBT_DSP_MIC1_STATE_EVT,    handleMic1StateEvent    },
    { PBOX_SOCBT_DSP_MIC2_STATE_EVT,    handleMic2StateEvent    },
    { PBOX_SOCBT_DSP_IN_OUT_DOOR_EVT,   handleInOutDoorEvent    },
    { PBOX_SOCBT_DSP_POWER_ON_EVT,      handlePowerOnEvent      },
    { PBOX_SOCBT_DSP_STEREO_MODE_EVT,   handleStereoModeEvent   },
    { PBOX_SOCBT_DSP_HUMAN_SPLIT_EVT,   handleHumanSplitEvent   },
    { PBOX_SOCBT_DSP_SWITCH_SOURCE_EVT, handleSwitchSourceEvent },
    { PBOX_SOCBT_DSP_MUSIC_GROUND_EVT,  handleMusicGroundEvent  },
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