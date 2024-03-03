#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <assert.h>
#include <sys/select.h>
#include "pbox_soc_bt.h"

#include "board.h"
#include "hal_hw.h"

#define UART_DEVICE "/dev/ttyS0"
#define BUF_SIZE 256

typedef void (*socbt_cmd_handle)(const pbox_socbt_msg_t*);

typedef enum {
    DSP_VERSION = 0,
    DSP_MASTER_VOLUME = 0x01,
    DSP_SPK_PLACEMENT = 0x02,
    DSP_MIC1_STATE = 0x03,
    DSP_MIC2_STATE = 0x04,
    DSP_IN_OUT_DOOR = 0x05,
    DSP_POWER_ON = 0x06,
    DSP_SOUND_MODE = 0x08,
    DSP_HUMAN_VOICE_FADEOUT = 0x0A,
    DSP_SWITCH_SOURCE = 0x0B,
    DSP_MUSIC_VOLUME = 0x10,
} soc_dsp_cmd_t;

enum State {
    READ_INIT,
    READ_HEADER,
    READ_HEADER_COMPLETE,
    READ_LENGTH,
    READ_DATA,
    READ_NUM
};

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

void socbt_pbox_notify_dspver_query(uint32_t opcode, char *buff, int32_t len) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_VERSION_EVT,
    };

    msg.op = opcode;
    printf("%s \n", __func__);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void socbt_pbox_notify_adjust_master_volume(uint32_t opcode, uint8_t *buff, int32_t len) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_MAIN_VOLUME_EVT,
    };
    assert(len>0);
    assert(buff[0]<=DSP_MAIN_MAX_VOL);
    msg.op = opcode;
    msg.volume = HW_MAIN_GAIN(buff[0])/10;//buff[0]*100/32;
    printf("%s opcode:%d, volume[%d]:%f\n", __func__, opcode, buff[0], msg.volume);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void socbt_pbox_notify_adjust_placement(uint32_t opcode, char *buff, int32_t len) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_PLACEMENT_EVT,
    };
    assert(len>0);
    msg.op = opcode;
    msg.placement = buff[0];
    printf("%s :%d\n", __func__, msg.placement);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void socbt_pbox_notify_adjust_mic1_state(uint32_t opcode, char *buff, int32_t len) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_MIC1_STATE_EVT,
    };
    assert(len>0);
    msg.op = opcode;
    msg.micMux = buff[0];
    printf("%s :%d\n", __func__, msg.micMux);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void socbt_pbox_notify_adjust_mic2_state(uint32_t opcode, char *buff, int32_t len) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_MIC2_STATE_EVT,
    };
    assert(len>0);
    msg.op = opcode;
    msg.micMux = buff[0];
    printf("%s :%d\n", __func__, msg.micMux);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void socbt_pbox_notify_adjust_inout_door(uint32_t opcode, char *buff, int32_t len) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_IN_OUT_DOOR_EVT,
    };
    assert(len>0);
    msg.op = opcode;
    msg.outdoor = buff[0];
    printf("%s :%d\n", __func__, msg.outdoor);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void socbt_pbox_notify_dsp_power_state(uint32_t opcode) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_POWER_ON_EVT,
    };
    msg.op = opcode;
    msg.poweron = true;
    printf("%s opcode:%d\n", __func__, opcode);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void socbt_pbox_notify_dsp_stereo_mode(uint32_t opcode, char *buff, int32_t len) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_STEREO_MODE_EVT,
    };
    assert(len>0);
    msg.op = opcode;
    msg.stereo = buff[0];
    printf("%s opcode:%d stereo:%d\n", __func__, opcode, msg.stereo);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void socbt_pbox_notify_dsp_human_voice_fadeout(uint32_t opcode, char *buff, int32_t len) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_HUMAN_VOICE_FADEOUT_EVT,
    };
    assert(len>0);
    msg.op = opcode;
    msg.fadeout = buff[0];
    printf("%s opcode:%d fadeout:%s\n", __func__, opcode, msg.fadeout? "fade":"org");
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void socbt_pbox_notify_dsp_switch_source(uint32_t opcode, char *buff, int32_t len) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_SWITCH_SOURCE_EVT,
    };
    assert(len>0);

    msg.op = opcode;
    msg.input_source.status = ((buff[0]>>7)&0x01) ? _STOP:PLAYING;
    //msg.input_source.input = buff[0]&0x1F;
    switch(buff[0]&0x1F) {
        case 0: {
            msg.input_source.input = SRC_BT;
        } break;
#if ENABLE_AUX
        case 1: {
            msg.input_source.input = SRC_AUX;
        } break;
#endif
        case 2: {
            msg.input_source.input = SRC_USB;
        } break;
    }
    printf("%s opcode:%d play_status:%d, source:%d\n", __func__, opcode, msg.input_source.status, msg.input_source.input);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void socbt_pbox_notify_adjust_music_volume_level(uint32_t opcode, uint8_t *buff, int32_t len) {
    pbox_socbt_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_SOCBT_DSP_MUSIC_VOLUME_EVT,
    };
    assert(len>0);
    assert(buff[0]<=DSP_MUSIC_MAX_VOL);
    msg.op = opcode;
    msg.musicVolLevel = HW_MUSIC_GAIN(buff[0])/10;//main_gain[buff[0]]/10;//buff[0]*100/32;
    printf("%s opcode:%d musicVolLevel:%f\n", __func__, opcode, msg.musicVolLevel);
    unix_socket_socbt_notify(&msg, sizeof(pbox_socbt_msg_t));
}

void socbt_pbox_notify_dsp_power(uint32_t opcode, char *buff, int32_t len) {
    socbt_pbox_notify_dsp_power_state(opcode);
    if(opcode == OP_WRITE) {
        char temp;
        assert(len>=8);
        temp = buff[0] & 0xf;
        socbt_pbox_notify_dsp_stereo_mode(opcode, &temp, 1);
        temp = buff[0] >> 4;
        socbt_pbox_notify_adjust_inout_door(opcode, &temp, 1);
        socbt_pbox_notify_adjust_master_volume(opcode, &buff[1], 1);
        socbt_pbox_notify_adjust_music_volume_level(opcode, &buff[2], 1);
        socbt_pbox_notify_adjust_mic1_state(opcode, &buff[3], 1);
        socbt_pbox_notify_adjust_mic2_state(opcode, &buff[4], 1);
        socbt_pbox_notify_adjust_placement(opcode, &buff[5], 1);
        //socbt_pbox_notify_dsp_human_voice_fadeout(opcode, &buff[6], 1);
        socbt_pbox_notify_dsp_switch_source(opcode, &buff[7], 1);
    }
}

bool is_check_sum_ok(unsigned char *buf, int len) {
    uint32_t checksum = 0;
    for(int i = 0; i < len - 1; i++) {
        checksum += buf[i];
    }

    if((checksum & 0x7F) == buf[ len- 1]) {
        return true;
    }

    return false;
}
#define CHECKSUM_LEN   1
void process_data(unsigned char *buff, int len) {
    uint32_t command_len, para_len;
    uint32_t opcode, command;

    if (len < 4+CHECKSUM_LEN) {
        return;
    }

    command_len = buff[1];
    para_len = command_len - 3;//1 byte opcode  + 1 byte command + 1 byte checksum
    opcode = buff[2];
    command = buff[3];

    printf("%s recv ", __func__);
    dump_data(buff, len);
    printf("%s opcode:%d, command:[%d]\n", __func__, opcode, command);

    switch((soc_dsp_cmd_t)command) {
        case DSP_VERSION: {
            socbt_pbox_notify_dspver_query(opcode, &buff[4], para_len);
        } break;
        case DSP_MASTER_VOLUME: {
            socbt_pbox_notify_adjust_master_volume(opcode, &buff[4], para_len);
        } break;
        case DSP_SPK_PLACEMENT: {
            socbt_pbox_notify_adjust_placement(opcode, &buff[4], para_len);
        } break;
        case DSP_MIC1_STATE: {
            socbt_pbox_notify_adjust_mic1_state(opcode, &buff[4], para_len);
        } break;
        case DSP_MIC2_STATE: {
            socbt_pbox_notify_adjust_mic2_state(opcode, &buff[4], para_len);
        } break;
        case DSP_IN_OUT_DOOR: {
            socbt_pbox_notify_adjust_inout_door(opcode, &buff[4], para_len);
        } break;
        case DSP_POWER_ON: {
            socbt_pbox_notify_dsp_power(opcode, &buff[4], para_len);
        } break;
        case DSP_SOUND_MODE: {
            socbt_pbox_notify_dsp_stereo_mode(opcode, &buff[4], para_len);
        } break;
        case DSP_HUMAN_VOICE_FADEOUT: {
            socbt_pbox_notify_dsp_human_voice_fadeout(opcode, &buff[4], para_len);
        } break;
        case DSP_SWITCH_SOURCE: {
            socbt_pbox_notify_dsp_switch_source(opcode, &buff[4], para_len);//status, source);
        } break;
        case DSP_MUSIC_VOLUME: {
            socbt_pbox_notify_adjust_music_volume_level(opcode, &buff[4], para_len);
        } break;
    }
}

void search_next_header(unsigned char *buf, int *index, int total_len) {
    for (int i = 1; i < total_len; ++i) {
        if (buf[i] == 0xCC) {
            memmove(buf, buf + i, total_len - i);
            *index = total_len - i;
            return;
        }
    }
    *index = 0;
}

#define HEAD_3NOD_LEN 2
void uart_data_recv_handler(int fd) {
    unsigned char buf[BUF_SIZE];
    static enum State current_state = READ_INIT;
    static uint32_t index, bytes_to_read = 0;

    //printf("%s current state= %d\n", __func__, current_state);
    switch (current_state) {
        case READ_INIT:
            index = 0;
            current_state = READ_HEADER;
            break;

        case READ_HEADER:
            if (read(fd, &buf[index], 1) > 0) {
                if (buf[index] == 0xCC) {
                    index++;
                    current_state = READ_LENGTH;
                }
            }
            break;

        case READ_LENGTH:
            if (read(fd, &buf[index], 1) > 0) {
                bytes_to_read = buf[index]; // include checksum value.
                printf("buf[%d]=[%02x], bytes_to_read:%d\n", index, buf[index], buf[index]);

                index++;
                current_state = READ_DATA;
            }
            break;

        case READ_DATA:
            {
                int nread = read(fd, &buf[index], bytes_to_read);
                //printf("index=%d, bytes_to_read=%d, nread=%d\n", index, bytes_to_read, nread);

                if (nread > 0) {
                    index += nread;
                    if (index == bytes_to_read + HEAD_3NOD_LEN) {//2 means head(1 byte) + length(1 byte)
                        if (is_check_sum_ok(buf, index)) {
                            process_data(buf, index);
                            current_state = READ_INIT;
                        } else {
                            search_next_header(buf, &index, index);
                            current_state = (index > 1) ? READ_HEADER_COMPLETE : READ_INIT;
                        }
                    }
                }
            }
            break;

        case READ_HEADER_COMPLETE:
            if (index > 1) {  // header and len data already exsist.
                bytes_to_read = buf[1]; //include checksum byte.
                current_state = READ_DATA;
            } else {
                current_state = READ_LENGTH;
            }
            break;
    }
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

uint8_t calculate_checksum(char buf[], int len) {
    int sum = 0;
    for(int i=0; i<len; i++) {
        sum = sum+buf[i];
    }
    printf("len=%d, checksum:0x%02x\n", len, sum);
    return sum&0x7F;
}

int sendPowerOnCmd(void) {
    uint8_t len = 0;
    uint8_t command[32];
    int i = 0;
    command[i++] = 0xEE;
    command[i++] = 0x03; //len
    command[i++] = 0x00; //op code
    command[i++] = 0x06; //cmd code
    command[i] = calculate_checksum(&command[0], i);
    i++;

    userial_send(command, i);
}

int sendMusicVolume(uint32_t volume) {
    uint8_t len = 0;
    uint8_t command[32];
    int i = 0;
    command[i++] = 0xEE;
    command[i++] = 0x04; //len
    command[i++] = 0x00; //op code
    command[i++] = 0x10; //cmd code
    command[i++] = 32*volume/100;
    command[i] = calculate_checksum(&command[0], i);
    i++;

    userial_send(command, i);
}

int sendDspVersionCmd(char *version) {
    uint8_t len = strlen(version);
    uint8_t command[32];
    int i = 0;
    command[i++] = 0xEE;
    command[i++] = 0x03 + len; //len
    command[i++] = 0x00; //op code
    command[i++] = 0x00; //cmd code

    strncpy(&command[i], version, len);
    i+= len;
    command[i++] = calculate_checksum(command, i);

    userial_send(command, i);
}

void handleSocbtDspVersionCmd(const pbox_socbt_msg_t* msg) {
    char *fw = msg->fw_ver;
    printf("%s fw:%s\n", __func__, fw);
    sendDspVersionCmd(fw);
}

void handleSocbtDspMainVolumeCmd(const pbox_socbt_msg_t* msg) {
    uint32_t volume = msg->volume;
    printf("%s main volume:%d\n", __func__, volume);

}

void handleSocbtDspMic1StateCmd(const pbox_socbt_msg_t* msg) {
    mic_mux_t mic = msg->micMux;
    printf("%s mic:%d\n", __func__, mic);
}

void handleSocbtDspMic2StateCmd(const pbox_socbt_msg_t* msg) {
    mic_mux_t mic = msg->micMux;
    printf("%s mic:%d\n", __func__, mic);

}

void handleSocbtDspInoutDoorCmd(const pbox_socbt_msg_t* msg) {
    inout_door_t inout = msg->outdoor;
    printf("%s inout:%d\n", __func__, inout);
}

void handleSocbtDspPoweronCmd(const pbox_socbt_msg_t* msg) {
    printf("%s \n", __func__);
    sendPowerOnCmd();
}

void handleSocbtDspStereoModeCmd(const pbox_socbt_msg_t* msg) {
    stereo_mode_t mode = msg->stereo;
    printf("%s dsp stereo mode:%d\n", __func__, mode);
}

void handleSocbtDspHumanVoiceFadeoutCmd(const pbox_socbt_msg_t* msg) {
    bool fadeout = msg->fadeout;
    printf("%s fadeout :%d\n", __func__, fadeout);
}

void handleSocbtDspSwitchSourceCmd(const pbox_socbt_msg_t* msg) {
    struct socbt_input_source source = msg->input_source;
    printf("%s inputsource:%d, status:%d\n", __func__, source.input, source.status);
}

void handleSocbtDspMusicVolumeCmd(const pbox_socbt_msg_t* msg) {
    uint32_t musicVolLevel = msg->musicVolLevel;
    printf("%s musicVolLevel:%d\n", __func__, musicVolLevel);
    sendMusicVolume(musicVolLevel);
}

// Function to process an incoming pbox_socbt_msg_t event
void process_pbox_socbt_cmd(const pbox_socbt_msg_t* msg) {
    if (msg == NULL) {
        printf("Error: Null event message received.\n");
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

    printf("Warning: No handle found for event ID %d.\n", msg->msgId);
}

void soc_bt_recv_data(int fd) {
    char buff[sizeof(pbox_socbt_msg_t)];
    int ret = recv(fd, buff, sizeof(buff), 0);
    if (ret <= 0) {
        if (ret == 0) {
            printf("Socket closed\n");
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

    pthread_setname_np(pthread_self(), "pbox_btsoc");
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

    while (1) {
        fd_set read_set = read_fds;

        int retval = select(max_fd + 1, &read_set, NULL, NULL, NULL);

        if (retval == -1) {
            perror("select error");
            //current_state = READ_INIT;
            continue;
        } else if (retval == 0) {
            printf("select timeout \n");
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
                    uart_data_recv_handler(btsoc_fds[BTSOC_UART]);
                } break;
            }
        }
    }
    close(btsoc_fds[BTSOC_UART]);
    return 0;
}

int pbox_create_btsoc_task(void)
{
    pthread_t tid_server;
    int ret;

    ret = pthread_create(&tid_server, NULL, btsoc_sink_server, NULL);
    if (ret < 0)
    {
        printf("btsink server start failed\n");
    }

    return ret;
}