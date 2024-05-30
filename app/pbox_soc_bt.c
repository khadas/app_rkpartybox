#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <assert.h>
#include <sys/select.h>
#include <sys/socket.h>
#include "pbox_soc_bt.h"
#include "rk_utils.h"
#include "hal_partybox.h"
#include "userial_vendor.h"
#include "pbox_socketpair.h"
#include "bt_vendor_protol.h"
#include "os_task.h"

typedef void (*socbt_cmd_handle)(const pbox_socbt_msg_t*);

static void handleSocbtDspVersionCmd(const pbox_socbt_msg_t* msg);
static void handleSocbtDspMainVolumeCmd(const pbox_socbt_msg_t* msg);
static void handleSocbtDspMic1StateCmd(const pbox_socbt_msg_t* msg);
static void handleSocbtDspMic2StateCmd(const pbox_socbt_msg_t* msg);
static void handleSocbtDspInoutDoorCmd(const pbox_socbt_msg_t* msg);
static void handleSocbtDspPoweronCmd(const pbox_socbt_msg_t* msg);
static void handleSocbtDspStereoModeCmd(const pbox_socbt_msg_t* msg);
static void handleSocbtDspHumanVoiceFadeoutCmd(const pbox_socbt_msg_t* msg);
static void handleSocbtDspSwitchSourceCmd(const pbox_socbt_msg_t* msg);
static void handleSocbtDspMusicVolumeCmd(const pbox_socbt_msg_t* msg);


int unix_socket_socbt_notify(void *info, int length) {
    return unix_socket_notify_msg(PBOX_MAIN_BT, info, length);
}

void soc2pbox_notify_dsp_version(uint32_t opcode, char *ver, int32_t len)
{
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_VERSION_EVT,
    };

    msg.op = opcode;
    ALOGD("%s \n", __func__);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}
//volume: max 0db, u shoud change it to db before call func
void soc2pbox_notify_master_volume(uint32_t opcode, float volume)
{
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_MAIN_VOLUME_EVT,
    };
    //assert(len>0);
    //assert(volume <= hal_dsp_max_main_vol());
    msg.op = opcode;
    msg.volume = volume;//hw_main_gain(buff[0])/10;//buff[0]*100/32;
    ALOGD("%s opcode:%d, volume:%f\n", __func__, opcode, msg.volume);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

//placement, u shoud change it to placement_t type before call func
void soc2pbox_notify_placement(uint32_t opcode, uint8_t placement)
{
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_PLACEMENT_EVT,
    };
    //assert(len>0);
    msg.op = opcode;
    msg.placement = placement;//buff[0];
    ALOGD("%s :%d\n", __func__, msg.placement);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

//micMux, u shoud change it to mic_mux_t type before call func
void soc2pbox_notify_mic1_mux(uint32_t opcode, uint8_t mux)
{
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_MIC1_STATE_EVT,
    };
    //assert(len>0);
    msg.op = opcode;
    msg.micMux = mux;//buff[0];
    ALOGD("%s :%d\n", __func__, msg.micMux);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

//micMux, u shoud change it to mic_mux_t type before call func
void soc2pbox_notify_mic2_mux(uint32_t opcode, uint8_t mux) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_MIC2_STATE_EVT,
    };
    //assert(len>0);
    msg.op = opcode;
    msg.micMux = mux;//buff[0];
    ALOGD("%s :%d\n", __func__, msg.micMux);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

//outdoor, change it to inout_door_t type before call func
void soc2pbox_notify_inout_door(uint32_t opcode, uint8_t inout)
{
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_IN_OUT_DOOR_EVT,
    };
    //assert(len>0);
    msg.op = opcode;
    msg.outdoor = inout;
    ALOGD("%s :%d\n", __func__, msg.outdoor);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

//kind/state, change it to mic_set_kind_t/mic_state_t type before call func
void soc2pbox_notify_mic_data(uint32_t opcode, uint8_t index, mic_set_kind_t kind, mic_state_t state)
{
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_MIC_DATA_EVT,
    };
    //assert(len > 1);
    msg.op = opcode;
    msg.micdata.index = index;
    msg.micdata.kind = kind;
    msg.micdata.micState = state;
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void soc2pbox_notify_dsp_power_state(uint32_t opcode, bool on) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_POWER_ON_EVT,
    };
    msg.op = opcode;
    msg.poweron = true;
    ALOGD("%s opcode:%d\n", __func__, opcode);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void soc2pbox_notify_dsp_stereo_mode(uint32_t opcode, uint8_t stereo) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_STEREO_MODE_EVT,
    };

    msg.op = opcode;
    msg.stereo = stereo;
    ALOGD("%s opcode:%d stereo:%d\n", __func__, opcode, msg.stereo);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void soc2pbox_notify_dsp_human_voice_fadeout(uint32_t opcode, bool fadeout) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_HUMAN_VOICE_FADEOUT_EVT,
    };
    msg.op = opcode;
    msg.fadeout = fadeout;
    ALOGD("%s opcode:%d fadeout:%s\n", __func__, opcode, msg.fadeout? "fade":"org");
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void soc2pbox_notify_dsp_switch_source(uint32_t opcode, uint8_t source, uint8_t status) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_SWITCH_SOURCE_EVT,
    };

    msg.op = opcode;
    msg.input_source.status = status;
    msg.input_source.input = source;
    ALOGD("%s opcode:%d source:%d play_status:%d\n", __func__, opcode, msg.input_source.input, msg.input_source.status);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void soc2pbox_notify_music_volume(uint32_t opcode, float volume) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_MUSIC_VOLUME_EVT,
    };

    msg.op = opcode;
    msg.musicVolLevel = volume;
    ALOGD("%s opcode:%d musicVolLevel:%f\n", __func__, opcode, msg.musicVolLevel);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void soc2pbox_notify_mic_mute(uint32_t opcode, uint8_t index, bool mute)
{
    mic_state_t state;
    state.micmute = mute;
    soc2pbox_notify_mic_data(opcode, index, MIC_SET_DEST_MUTE, state);
}

void soc2pbox_notify_mic_mux(uint32_t opcode, uint8_t index, uint8_t mux)
{
    mic_state_t state;
    state.micMux = mux;
    soc2pbox_notify_mic_data(opcode, index, MIC_SET_DEST_MUX, state);
}

void soc2pbox_notify_mic_3a_effect(uint32_t opcode, uint8_t index, bool on)
{
    mic_state_t state;
    state.echo3a = on;
    soc2pbox_notify_mic_data(opcode, index, MIC_SET_DEST_ECHO_3A, state);
}

void soc2pbox_notify_mic_reverb_mode(uint32_t opcode, uint8_t index, uint8_t reverbMode)
{
    mic_state_t state;
    state.reverbMode = reverbMode;
    soc2pbox_notify_mic_data(opcode, index, MIC_SET_DEST_REVERB_MODE, state);
}

void soc2pbox_notify_mic_reverb(uint32_t opcode, uint8_t index, float micReverb)
{
    mic_state_t state;
    state.micReverb = micReverb;
    soc2pbox_notify_mic_data(opcode, index, MIC_SET_DEST_REVERB, state);
}

void soc2pbox_notify_mic_bass(uint32_t opcode, uint8_t index, float micBass)
{
    mic_state_t state;
    state.micBass = micBass;
    soc2pbox_notify_mic_data(opcode, index, MIC_SET_DEST_BASS, state);
}

void soc2pbox_notify_mic_treble(uint32_t opcode, uint8_t index, float micTreble)
{
    mic_state_t state;
    state.micTreble = micTreble;
    soc2pbox_notify_mic_data(opcode, index, MIC_SET_DEST_TREBLE, state);
}

void soc2pbox_notify_mic_volume(uint32_t opcode, uint8_t index, float micVolume)
{
    mic_state_t state;
    state.micVolume = micVolume;
    soc2pbox_notify_mic_data(opcode, index, MIC_SET_DEST_VOLUME, state);
}

typedef struct {
    pbox_socbt_opcode_t opcode;
    socbt_cmd_handle handle;
} socbt_cmd_handle_t;

const socbt_cmd_handle_t socbtCmdTable[] = {
    { PBOX_SOCBT_DSP_VERSION_CMD,       handleSocbtDspVersionCmd   },
    { PBOX_SOCBT_DSP_MAIN_VOLUME_CMD,   handleSocbtDspMainVolumeCmd},

    { PBOX_SOCBT_DSP_MIC1_STATE_CMD,    handleSocbtDspMic1StateCmd },
    { PBOX_SOCBT_DSP_MIC2_STATE_CMD,    handleSocbtDspMic2StateCmd },
    { PBOX_SOCBT_DSP_IN_OUT_DOOR_CMD,   handleSocbtDspInoutDoorCmd },
    { PBOX_SOCBT_DSP_POWER_ON_CMD,      handleSocbtDspPoweronCmd   },
    { PBOX_SOCBT_DSP_STEREO_MODE_CMD,   handleSocbtDspStereoModeCmd },
    { PBOX_SOCBT_DSP_HUMAN_VOICE_FADEOUT_CMD,   handleSocbtDspHumanVoiceFadeoutCmd},
    { PBOX_SOCBT_DSP_SWITCH_SOURCE_CMD, handleSocbtDspSwitchSourceCmd},
    { PBOX_SOCBT_DSP_MUSIC_VOLUME_CMD,  handleSocbtDspMusicVolumeCmd},
};

void handleSocbtDspVersionCmd(const pbox_socbt_msg_t* msg) {
    char *fw_ver =(char *)(msg->fw_ver);
    ALOGD("%s fw:%s\n", __func__, fw_ver);
    dsp2btsoc_echo_version(fw_ver);
}

void handleSocbtDspMainVolumeCmd(const pbox_socbt_msg_t* msg) {
    float volume = msg->volume;
    ALOGD("%s main volume:%f\n", __func__, volume);
    dsp2btsoc_notify_main_volume(volume);
}

void handleSocbtDspMic1StateCmd(const pbox_socbt_msg_t* msg) {
    mic_mux_t mux = msg->micMux;
    ALOGD("%s mux:%d\n", __func__, mux);
    dsp2btsoc_notify_mic_mux(0, mux);
}

void handleSocbtDspMic2StateCmd(const pbox_socbt_msg_t* msg) {
    mic_mux_t mux = msg->micMux;
    ALOGD("%s mux:%d\n", __func__, mux);
    dsp2btsoc_notify_mic_mux(1, mux);
}

void handleSocbtDspInoutDoorCmd(const pbox_socbt_msg_t* msg) {
    inout_door_t inout = msg->outdoor;
    ALOGD("%s inout:%d\n", __func__, inout);
    dsp2btsoc_notify_inout_door(inout);
}

void handleSocbtDspPoweronCmd(const pbox_socbt_msg_t* msg) {
    ALOGD("%s:%d \n", __func__, msg->poweron);
    dsp2btsoc_notify_power_on(msg->poweron);
}

void handleSocbtDspStereoModeCmd(const pbox_socbt_msg_t* msg) {
    stereo_mode_t mode = msg->stereo;
    ALOGD("%s dsp stereo mode:%d\n", __func__, mode);
    dsp2btsoc_notify_stereo_mode(mode);
}

void handleSocbtDspHumanVoiceFadeoutCmd(const pbox_socbt_msg_t* msg) {
    bool fadeout = msg->fadeout;
    ALOGD("%s fadeout :%d\n", __func__, fadeout);
    dsp2btsoc_notify_human_voice_fadeout(fadeout);
}

void handleSocbtDspSwitchSourceCmd(const pbox_socbt_msg_t* msg) {
    struct socbt_input_source source = msg->input_source;
    ALOGD("%s inputsource:%d, status:%d\n", __func__, source.input, source.status);
    dsp2btsoc_notify_switch_source(source.input, source.status);
}

void handleSocbtDspMusicVolumeCmd(const pbox_socbt_msg_t* msg) {
    float musicVolLevel = msg->musicVolLevel;
    ALOGD("%s musicVolLevel:%d\n", __func__, musicVolLevel);
    dsp2btsoc_notify_music_volume(musicVolLevel);
}

// Function to process an incoming pbox_socbt_msg_t event
void process_pbox_socbt_cmd(const pbox_socbt_msg_t* msg) {
    if (msg == NULL) {
        ALOGD("Error: Null event message received.\n");
        return;
    }

    for (int i = 0; i < sizeof(socbtCmdTable) / sizeof(socbtCmdTable[0]); ++i) {
        if (socbtCmdTable[i].opcode == msg->msgId) {
            if (socbtCmdTable[i].handle != NULL) {
                socbtCmdTable[i].handle(msg);
            }
            return;
        }
    }

    ALOGW("Warning: No handle found for event ID %d.\n", msg->msgId);
}

void soc_bt_recv_data(int fd) {
    char buff[sizeof(pbox_socbt_msg_t)];
    int ret = recv(fd, buff, sizeof(buff), 0);
    if (ret <= 0) {
        if (ret == 0) {
            ALOGW("Socket closed\n");
        } else {
            perror("recvfrom failed");
        }
    }

    pbox_socbt_msg_t *msg = (pbox_socbt_msg_t *)buff;
    if(msg->type == PBOX_EVT)
        return;

    process_pbox_socbt_cmd(msg);
}

#define BTSOC_UDP_SOCKET    0
#define BTSOC_UART          1
#define BTSOC_FD_NUM        2
static void *btsoc_sink_server(void *arg) {
    int btsoc_fds[BTSOC_FD_NUM];
    vendor_data_recv_handler_t  uart_vendor_data_recv_hander = vendor_get_data_recv_func();
    os_sem_t* quit_sem = os_task_get_quit_sem(os_gettid());

    btsoc_register_vendor_notify_func(pbox_socbt_get_notify_funcs());
    PBOX_ARRAY_SET(btsoc_fds, -1, sizeof(btsoc_fds)/sizeof(btsoc_fds[0]));

    btsoc_fds[BTSOC_UDP_SOCKET] =   get_server_socketpair_fd(PBOX_SOCKPAIR_BT);
    btsoc_fds[BTSOC_UART] =         open_uart();
    exec_command_system("stty -F /dev/ttyS0 38400 cs8 -cstopb parenb -parodd");
    if (btsoc_fds[BTSOC_UART] < 0) return (void *)-1;

    int max_fd = findMax(btsoc_fds, sizeof(btsoc_fds)/sizeof(btsoc_fds[0]));

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(btsoc_fds[BTSOC_UDP_SOCKET], &read_fds);
    FD_SET(btsoc_fds[BTSOC_UART], &read_fds);

    while(os_sem_trywait(quit_sem) != 0) {
        fd_set read_set = read_fds;
        struct timeval tv = {
            .tv_sec = 0,
            .tv_usec = 200000,
        };
        int retval = select(max_fd + 1, &read_set, NULL, NULL, &tv);
        if (retval == -1) {
            perror("select error");
            //current_state = READ_INIT;
            continue;
        } else if (retval == 0) {
            //ALOGW("select timeout \n");
            continue;
        }

        for(int i = 0, ret = -1; i < BTSOC_FD_NUM; i++) {
            if((ret = FD_ISSET(btsoc_fds[i], &read_set)) == 0)
                continue;

            switch(i) {
                case BTSOC_UDP_SOCKET: {
                    soc_bt_recv_data(btsoc_fds[BTSOC_UDP_SOCKET]);
                } break;

                case BTSOC_UART: {
                    if(uart_vendor_data_recv_hander == NULL) {
                        ALOGW("did you implement your uart/i2c data recv func???");
                        return (void*)(-1);
                    }
                    uart_vendor_data_recv_hander(btsoc_fds[BTSOC_UART]);
                } break;
            }
        }
    }
    close(btsoc_fds[BTSOC_UART]);
    return (void*)(0);
}

static os_task_t* btsoc_task_server;
int pbox_stop_btsoc_task(void) {
    if(btsoc_task_server != NULL) {
        os_task_destroy(btsoc_task_server);
    }
    return 0;
}

int pbox_create_btsoc_task(void)
{
    int ret;
    ret = (btsoc_task_server = os_task_create("pbox_btsoc", btsoc_sink_server, 0, NULL))?0:-1;
    if (ret < 0)
    {
        ALOGE("btsink server start failed\n");
    }
    return ret;
}

const static NotifyFuncs_t notify_funcs = {
    .notify_dsp_version = soc2pbox_notify_dsp_version,
    .notify_master_volume = soc2pbox_notify_master_volume,
    .notify_placement = soc2pbox_notify_placement,
    .notify_mic1_mux = soc2pbox_notify_mic1_mux,
    .notify_mic2_mux = soc2pbox_notify_mic2_mux,
    .notify_inout_door = soc2pbox_notify_inout_door,
    .notify_dsp_power_state = soc2pbox_notify_dsp_power_state,
    .notify_dsp_stereo_mode = soc2pbox_notify_dsp_stereo_mode,
    .notify_human_voice_fadeout = soc2pbox_notify_dsp_human_voice_fadeout,
    .notify_dsp_switch_source = soc2pbox_notify_dsp_switch_source,
    .notify_music_volume = soc2pbox_notify_music_volume,
    .notify_mic_mute = soc2pbox_notify_mic_mute,
    .notify_mic_volume = soc2pbox_notify_mic_volume,
    .notify_mic_mute = soc2pbox_notify_mic_mute,
    .notify_mic_index_mux = soc2pbox_notify_mic_mux,
    .notify_mic_3a_effect = soc2pbox_notify_mic_3a_effect,
    .notify_mic_reverb_mode = soc2pbox_notify_mic_reverb_mode,
    .notify_mic_reverb = soc2pbox_notify_mic_reverb,
    .notify_mic_bass = soc2pbox_notify_mic_bass,
    .notify_mic_treble = soc2pbox_notify_mic_treble,
};

const NotifyFuncs_t* pbox_socbt_get_notify_funcs(void) {
    return &notify_funcs;
}