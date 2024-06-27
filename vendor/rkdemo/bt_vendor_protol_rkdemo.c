#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <assert.h>
#include "slog.h"
#include "bt_vendor_protol.h"
#include "os_minor_type.h"
#include "userial_vendor.h"
#include "hal_partybox.h"
#include "pbox_interface.h"

#define UART_DEVICE "/dev/ttyS0"
#define BUF_SIZE 255
enum State {
    READ_INIT,
    READ_HEADER,
    READ_HEADER_COMPLETE,
    READ_LENGTH,
    READ_DATA,
    READ_NUM
};

#define HEAD_LEN 2
#define CHECKSUM_LEN 1

#define BYTE_HEAD_IN    0xCC
#define BYTE_HEAD_OUT   0xEE

typedef enum {
    CATE_WRITE,
    CATE_READ,
    CATE_NUM,
} category_t;

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

    DSP_MIC_GT_VOLUME = 0x20,
    DSP_MIC_GT_TREBLE = 0x21,
    DSP_MIC_GT_BASS = 0x22,
    DSP_MIC_GT_REVERB = 0x23,
    DSP_MIC_GT_MUTE = 0x24,
    DSP_MIC_GT_MUX = 0x25,
    DSP_MIC_GT_3A = 0x26,
    DSP_MIC_GT_ECHO_MODE = 0x27,
    DSP_VOLCAL_HUMAN_RATIO= 0x28,
    DSP_VOLCAL_ACCOMP_RATIO = 0x29,
    DSP_VOLCAL_RESERV_RATIO = 0x2A,
} soc_dsp_cmd_t;

static const NotifyFuncs_t* rkdemoNotifyFuncs = NULL;
static int rkWriteUartfd = -1;
static void process_data(unsigned char *buff, int len);

static uint8_t calculate_checksum(char buf[], int len) {
    int sum = 0;
    for(int i=0; i<len; i++) {
        sum = sum+buf[i];
    }
    ALOGD("len=%d, checksum:0x%02x\n", len, sum);
    return sum&0x7F;
}

static bool is_check_sum_ok(unsigned char *buf, int len) {
    uint32_t checksum = 0;
    for(int i = 0; i < len - 1; i++) {
        checksum += buf[i];
    }

    if((checksum & 0x7F) == buf[ len- 1]) {
        return true;
    }

    ALOGD("%s checksum fail\n", __func__);
    return false;
}

static uint16_t vendor_build_cmd(category_t opcode, soc_dsp_cmd_t subcommand, uint8_t *src, uint8_t srclen, char *dst, uint8_t dstlen) {
    uint8_t i = 0;
    assert(dstlen >= (srclen+HEAD_LEN+2+CHECKSUM_LEN));//2 means category cmd with sub command
    dst[i++] = BYTE_HEAD_OUT;
    dst[i++] = 2 + srclen + CHECKSUM_LEN;
    dst[i++] = opcode; //category command
    dst[i++] = subcommand;
    memcpy(&dst[i], src, srclen);
    i = i+ srclen;
    dst[i] = calculate_checksum(dst, i);
    return ++i;
}

static void dsp2btsoc_notify(uint32_t command, uint8_t* value, uint16_t len) {
    uint8_t *buff = os_malloc(BUF_SIZE);
    uint16_t ret;

    ALOGD("%s command:%d value:%d\n", __func__, command, (uint8_t)(*value));
    ret = vendor_build_cmd(CATE_WRITE, command, value, len, buff, BUF_SIZE);

    if(rkWriteUartfd != -1)
        userial_vendor_send_data(rkWriteUartfd, buff, ret);
    os_free(buff);
}

static void dsp2btsoc_inquiry(uint32_t command, uint8_t* value, uint16_t len) {
    uint8_t *buff = os_malloc(BUF_SIZE);
    uint8_t ret;

    ALOGD("%s sub:%d value:%d\n", __func__, command, value);
    ret = vendor_build_cmd(CATE_READ, command, value, len, buff, BUF_SIZE);

    if(rkWriteUartfd != -1)
        userial_vendor_send_data(rkWriteUartfd, buff, ret);
    os_free(buff);
}

static uint16_t get_vendor_table_index(float value, const float table[], uint16_t size)
{
    assert(size > 0);

    ALOGD("%s size:%d\n", __func__, size);
    for (int i = size-1; i > 0; i--) {
        if(value > table[i-1] && value <= table[i]) {
            return i;
        }
    }

    if(value > table[size])
        return size-1;
    return 0;
}

int dsp2btsoc_echo_version(char *version) {
    dsp2btsoc_notify(DSP_POWER_ON, version, strlen(version));
    return 0;
}

int dsp2btsoc_notify_main_volume(float volume) {
    uint16_t size;
    const float *ptable = hw_get_main_volume_table(&size);

    assert(size > 0);
    //covert the float volume to customer disered range..
    volume = volume*10;
    uint8_t value = get_vendor_table_index(volume, ptable, size);
    dsp2btsoc_notify(DSP_MUSIC_VOLUME, &value, 1);
    return 0;
}

int dsp2btsoc_notify_music_volume(float volume) {
    uint16_t size;
    const float *ptable = hw_get_music_volume_table(&size);

    assert(size > 0);
    //covert the float volume to customer disered range..
    volume = volume*10;
    uint8_t value = get_vendor_table_index(volume, ptable, size);
    dsp2btsoc_notify(DSP_MUSIC_VOLUME, &value, 1);

    return 0;
}

int dsp2btsoc_notify_mic_mux(uint8_t index, uint8_t mux) {
    switch (index) {
        case 0: {
            dsp2btsoc_notify(DSP_MIC1_STATE, &mux, 1);
        } break;
        case 1: {
            dsp2btsoc_notify(DSP_MIC2_STATE, &mux, 1);
        } break;
    }
    return 0;
}

int dsp2btsoc_notify_inout_door(uint8_t inout) {
    dsp2btsoc_notify(DSP_IN_OUT_DOOR, &inout, 1);
    return 0;
}

int dsp2btsoc_notify_power_on(bool on) {
    uint8_t status = on;
    dsp2btsoc_notify(DSP_POWER_ON, &status, 1);
    return 0;
}

int dsp2btsoc_notify_stereo_mode(uint8_t mode) {
    dsp2btsoc_notify(DSP_SOUND_MODE, &mode, 1);
    return 0;
}

int dsp2btsoc_notify_human_voice_fadeout(bool fadeout) {
    uint8_t fade = fadeout;
    dsp2btsoc_notify(DSP_HUMAN_VOICE_FADEOUT, &fade, 1);
    return 0;
}

int dsp2btsoc_notify_switch_source(uint8_t source, uint8_t status) {
    uint8_t value = 0;
    switch (source) {
        case SRC_EXT_BT: {
            value = 0x01;
        } break;
        case SRC_EXT_AUX: {
            value = 0x02;
        } break;
        case SRC_EXT_USB: {
            value = 0x04;
        } break;
    }

    if(status == INTERFACE_PLAYING) {
        value |= 0x80;
    }

    dsp2btsoc_notify(DSP_SWITCH_SOURCE, &value, 1);
    return 0;
}

void rkdemo_btsoc_notify_dspver_query(uint32_t opcode, char *buff, int32_t len) {
    assert(len>0);
    rkdemoNotifyFuncs->notify_dsp_version(opcode, buff, len);
}

void rkdemo_btsoc_notify_master_volume(uint32_t opcode, uint8_t *buff, int32_t len) {
    float volume;
    assert(len>0);
    assert(buff[0] <= hal_dsp_max_main_vol());

    //covert to rockchip standard volume db(max: 0db)
    volume = hw_main_gain(buff[0])/10;
    rkdemoNotifyFuncs->notify_master_volume(opcode, volume);
}

void rkdemo_btsoc_notify_music_volume_level(uint32_t opcode, uint8_t *buff, int32_t len) {
    float volume;
    assert(len>0);
    assert(buff[0] <= hal_dsp_max_music_vol());

    //covert to rockchip standard volume db(max: 0db)
    volume = hw_music_gain(buff[0])/10;
    ALOGD("%s opcode:%d musicVolLevel:%f\n", __func__, opcode, volume);
    rkdemoNotifyFuncs->notify_music_volume(opcode, volume);
}

void rkdemo_btsoc_notify_placement(uint32_t opcode, char *buff, int32_t len) {
    uint8_t placement = buff[0];
    assert(len>0);

    rkdemoNotifyFuncs->notify_placement(opcode, placement);
}

void rkdemo_btsoc_notify_mic1_state(uint32_t opcode, char *buff, int32_t len) {
    uint8_t mux;
    assert(len>0);

    mux = buff[0];
    rkdemoNotifyFuncs->notify_mic1_mux(opcode, mux);
}

void rkdemo_btsoc_notify_mic2_state(uint32_t opcode, char *buff, int32_t len) {
    uint8_t mux;
    assert(len>0);

    mux = buff[0];
    rkdemoNotifyFuncs->notify_mic2_mux(opcode, mux);
}

void rkdemo_btsoc_notify_inout_door(uint32_t opcode, char *buff, int32_t len) {
    assert(len>0);
    rkdemoNotifyFuncs->notify_inout_door(opcode, buff[0]);
}

void rkdemo_btsoc_notify_mic_mute(uint32_t opcode, char *buff, int32_t len) {
    assert(len>1);
    uint8_t index = buff[0];
    uint8_t mute = buff[1];
    rkdemoNotifyFuncs->notify_mic_mute(opcode, index, mute);
}

void rkdemo_btsoc_notify_mic_mux(uint32_t opcode, char *buff, int32_t len) {
    assert(len>1);
    uint8_t index = buff[0];
    uint8_t mux = buff[1];
    rkdemoNotifyFuncs->notify_mic_index_mux(opcode, index, mux);
}

void rkdemo_btsoc_notify_3a_effect(uint32_t opcode, char *buff, int32_t len) {
    assert(len>1);
    uint8_t index = buff[0];
    uint8_t echo3a = buff[1];
    rkdemoNotifyFuncs->notify_mic_3a_effect(opcode, index, echo3a);
}

void rkdemo_btsoc_notify_echo_mode(uint32_t opcode, char *buff, int32_t len) {
    assert(len>1);
    uint8_t index = buff[0];
    uint8_t echo = buff[1];
    rkdemoNotifyFuncs->notify_mic_reverb_mode(opcode, index, echo);
}

void rkdemo_btsoc_notify_mic_reverb(uint32_t opcode, char *buff, int32_t len) {
    assert(len>1);

    uint8_t index = buff[0];
    uint8_t reverb = buff[1];

    assert(reverb <= hal_dsp_max_mic_reverb());
    //covert to rockchip standard reverb input
    float level = hw_mic_reverb(reverb);

    rkdemoNotifyFuncs->notify_mic_reverb(opcode, index, level);
}

void rkdemo_btsoc_notify_mic_bass(uint32_t opcode, char *buff, int32_t len) {
    assert(len>1);
    uint8_t index = buff[0];
    uint8_t bass = buff[1];

    assert(bass <= hal_dsp_max_mic_bass());
    //covert to rockchip standard reverb input
    float level = hw_mic_bass(bass)/10;
    rkdemoNotifyFuncs->notify_mic_bass(opcode, index, bass);
}

void rkdemo_btsoc_notify_mic_treble(uint32_t opcode, char *buff, int32_t len) {
    assert(len>1);
    uint8_t index = buff[0];
    uint8_t treble = buff[1];

    assert(treble <= hal_dsp_max_mic_treble());
    //covert to rockchip standard reverb input
    float level = hw_mic_treble(treble)/10;
    rkdemoNotifyFuncs->notify_mic_treble(opcode, index, treble);
}


#define VND_ORG2TARGET(value, type, MIN_TARGET, MAX_TARGET, MIC_ORG, MAX_ORG) \
                        ((MAX_TARGET - MIN_TARGET)*((type)(value) - MIC_ORG)/(MAX_ORG - MIC_ORG) + MIN_TARGET)
void rkdemo_btsoc_notify_mic_volume(uint32_t opcode, char *buff, int32_t len) {
    assert(len>1);
    uint8_t index = buff[0];
    uint8_t volume = buff[1];

    assert(volume <= hal_dsp_max_mic_vol());
    //covert to rockchip standard reverb input
    float level = VND_ORG2TARGET(volume, float, get_minMicVolume(), get_maxMicVolume(), 0, hal_dsp_max_mic_vol());
    rkdemoNotifyFuncs->notify_mic_volume(opcode, index, volume);
}

void rkdemo_btsoc_notify_dsp_power_state(uint32_t opcode) {
    assert(len>0);
    ALOGD("%s opcode:%d\n", __func__, opcode);
    rkdemoNotifyFuncs->notify_dsp_power_state(opcode, true);
}

void rkdemo_btsoc_notify_dsp_stereo_mode(uint32_t opcode, char *buff, int32_t len) {
    assert(len>0);
    uint8_t stereo = buff[0];
    ALOGD("%s opcode:%d stereo:%d\n", __func__, opcode, stereo);
    rkdemoNotifyFuncs->notify_dsp_stereo_mode(opcode, stereo);
}

void rkdemo_btsoc_notify_dsp_human_voice_fadeout(uint32_t opcode, char *buff, int32_t len) {
    assert(len>0);
    uint8_t fadeout = buff[0];
    ALOGD("%s opcode:%d fadeout:%s\n", __func__, opcode, fadeout? "yes":"no");
    rkdemoNotifyFuncs->notify_human_voice_fadeout(opcode, fadeout);
}

void rkdemo_btsoc_notify_dsp_vocal_human_level(uint32_t opcode, char *buff, int32_t len) {
    assert(len>0);
    uint8_t level = buff[0];
    ALOGD("%s opcode:%d level:%d\n", __func__, opcode, level);
    rkdemoNotifyFuncs->notify_vocal_human_level(opcode, level);
}

void rkdemo_btsoc_notify_dsp_vocal_accomp_level(uint32_t opcode, char *buff, int32_t len) {
    assert(len>0);
    uint8_t level = buff[0];
    ALOGD("%s opcode:%d level:%d\n", __func__, opcode, level);
    rkdemoNotifyFuncs->notify_vocal_accomp_level(opcode, level);
}

void rkdemo_btsoc_notify_dsp_vocal_reserv_level(uint32_t opcode, char *buff, int32_t len) {
    assert(len>0);
    uint8_t level = buff[0];
    ALOGD("%s opcode:%d level:%d\n", __func__, opcode, level);
    rkdemoNotifyFuncs->notify_vocal_reserv_level(opcode, level);
}

void rkdemo_btsoc_notify_dsp_switch_source(uint32_t opcode, char *buff, int32_t len) {
    assert(len>0);

    input_source_t source;
    uint8_t status = ((buff[0]>>7)&0x01) ? INTERFACE_STOP:INTERFACE_PLAYING;

    switch(buff[0]&0x1F) {
        case 0: {
            source = SRC_EXT_BT;
        } break;
        case 1: {
            source = SRC_EXT_AUX;
        } break;
        case 2: {
            source = SRC_EXT_USB;
        } break;
    }
    ALOGD("%s opcode:%d play_status:%d, source:%d\n", __func__, opcode, status, source);
    rkdemoNotifyFuncs->notify_dsp_switch_source(opcode, source, status);
}

void rkdemo_btsoc_notify_dsp_power(uint32_t opcode, char *buff, int32_t len) {
    rkdemo_btsoc_notify_dsp_power_state(opcode);
    if(opcode == CATE_WRITE) {
        char temp;
        assert(len>=8);
        temp = buff[0] & 0xf;
        rkdemoNotifyFuncs->notify_dsp_stereo_mode(opcode, temp);
        temp = buff[0] >> 4;
        rkdemoNotifyFuncs->notify_inout_door(opcode, temp);

        float volume;
        assert(buff[1] <= hal_dsp_max_main_vol());
        //covert to rockchip standard volume input db(max: 0db)
        volume = hw_main_gain(buff[1])/10;
        rkdemoNotifyFuncs->notify_master_volume(opcode, volume);

        assert(buff[2] <= hal_dsp_max_music_vol());
        //covert to rockchip standard volume input db(max: 0db)
        volume = hw_music_gain(buff[2])/10;
        rkdemoNotifyFuncs->notify_music_volume(opcode, volume);

        rkdemoNotifyFuncs->notify_mic1_mux(opcode, buff[3]);
        rkdemoNotifyFuncs->notify_mic2_mux(opcode, buff[4]);
        rkdemoNotifyFuncs->notify_placement(opcode, buff[5]);
        //rkdemoNotifyFuncs->notify_human_voice_fadeout(opcode, buff[6]);
        rkdemoNotifyFuncs->notify_human_voice_fadeout(opcode, buff[6]);

        input_source_t source;
        uint8_t status = ((buff[7]>>7)&0x01) ? INTERFACE_STOP:INTERFACE_PLAYING;

        switch(buff[0]&0x1F) {
            case 0: {
                source = SRC_EXT_BT;
            } break;
            case 1: {
                source = SRC_EXT_AUX;
            } break;
            case 2: {
                source = SRC_EXT_USB;
            } break;
        }
        rkdemoNotifyFuncs->notify_dsp_switch_source(opcode, source, status);
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
struct uart_data_recv_class {
    enum State current_state;
    uint32_t index;
    uint32_t bytes_to_read;
};

void rkdemo_uart_data_recv_handler(int fd, struct uart_data_recv_class* pMachine) {
    unsigned char buf[BUF_SIZE];
    assert(pMachine != NULL);
    // static enum State current_state = READ_INIT;
    // static uint32_t index, bytes_to_read = 0;

    //ALOGD("%s current state= %d\n", __func__, pMachine->current_state);
    switch (pMachine->current_state) {
        case READ_INIT:
            pMachine->index = 0;
            pMachine->current_state = READ_HEADER;
            break;

        case READ_HEADER:
            if (read(fd, &buf[pMachine->index], 1) > 0) {
                if (buf[pMachine->index] == 0xCC) {
                    pMachine->index++;
                    pMachine->current_state = READ_LENGTH;
                }
            }
            break;

        case READ_LENGTH:
            if (read(fd, &buf[pMachine->index], 1) > 0) {
                pMachine->bytes_to_read = buf[pMachine->index]; // include checksum value.
                //ALOGD("buf[%d]=[%02x], bytes_to_read:%d\n", pMachine->index, buf[pMachine->index], buf[pMachine->index]);

                pMachine->index++;
                pMachine->current_state = READ_DATA;
            }
            break;

        case READ_DATA:
            {
                int nread = read(fd, &buf[pMachine->index], pMachine->bytes_to_read);
                ALOGD("index=%d, bytes_to_read=%d, nread=%d\n", pMachine->index, pMachine->bytes_to_read, nread);

                if (nread > 0) {
                    pMachine->index += nread;
                    if (pMachine->index >= pMachine->bytes_to_read + HEAD_3NOD_LEN) {//2 means head(1 byte) + length(1 byte)
                        if (is_check_sum_ok(buf, pMachine->index)) {
                            process_data(buf, pMachine->index);
                            pMachine->current_state = READ_INIT;
                        } else {
                            search_next_header(buf, &pMachine->index, pMachine->index);
                            pMachine->current_state = (pMachine->index > 1) ? READ_HEADER_COMPLETE : READ_INIT;
                        }
                    }
                }
            }
            break;

        case READ_HEADER_COMPLETE:
            if (pMachine->index > 1) {  // header and len data already exsist.
                pMachine->bytes_to_read = buf[1]; //include checksum byte.
                pMachine->current_state = READ_DATA;
            } else {
                pMachine->current_state = READ_LENGTH;
            }
            break;
    }
}

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

    ALOGD("%s recv\n", __func__);
    userial_dump_data(buff, len);
    ALOGD("%s opcode:%d, command:[%d]\n", __func__, opcode, command);

    switch((soc_dsp_cmd_t)command) {
        case DSP_VERSION: {
            rkdemo_btsoc_notify_dspver_query(opcode, &buff[4], para_len);
        } break;
        case DSP_MASTER_VOLUME: {
            rkdemo_btsoc_notify_master_volume(opcode, &buff[4], para_len);
        } break;
        case DSP_SPK_PLACEMENT: {
            rkdemo_btsoc_notify_placement(opcode, &buff[4], para_len);
        } break;
        case DSP_MIC1_STATE: {
            rkdemo_btsoc_notify_mic1_state(opcode, &buff[4], para_len);
        } break;
        case DSP_MIC2_STATE: {
            rkdemo_btsoc_notify_mic2_state(opcode, &buff[4], para_len);
        } break;
        case DSP_IN_OUT_DOOR: {
            rkdemo_btsoc_notify_inout_door(opcode, &buff[4], para_len);
        } break;
        case DSP_POWER_ON: {
            rkdemo_btsoc_notify_dsp_power(opcode, &buff[4], para_len);
        } break;
        case DSP_SOUND_MODE: {
            rkdemo_btsoc_notify_dsp_stereo_mode(opcode, &buff[4], para_len);
        } break;
        case DSP_HUMAN_VOICE_FADEOUT: {
            rkdemo_btsoc_notify_dsp_human_voice_fadeout(opcode, &buff[4], para_len);
        } break;
        case DSP_VOLCAL_HUMAN_RATIO: {
            rkdemo_btsoc_notify_dsp_vocal_human_level(opcode, &buff[4], para_len);
        } break;
        case DSP_VOLCAL_ACCOMP_RATIO: {
            rkdemo_btsoc_notify_dsp_vocal_accomp_level(opcode, &buff[4], para_len);
        } break;
        case DSP_VOLCAL_RESERV_RATIO: {
            rkdemo_btsoc_notify_dsp_vocal_reserv_level(opcode, &buff[4], para_len);
        } break;
        case DSP_SWITCH_SOURCE: {
            rkdemo_btsoc_notify_dsp_switch_source(opcode, &buff[4], para_len);//status, source);
        } break;
        case DSP_MUSIC_VOLUME: {
            rkdemo_btsoc_notify_music_volume_level(opcode, &buff[4], para_len);
        } break;
        case DSP_MIC_GT_VOLUME: {
            rkdemo_btsoc_notify_mic_volume(opcode, &buff[4], para_len);
        } break;
        case DSP_MIC_GT_TREBLE: {
            rkdemo_btsoc_notify_mic_treble(opcode, &buff[4], para_len);
        } break;
        case DSP_MIC_GT_BASS: {
            rkdemo_btsoc_notify_mic_bass(opcode, &buff[4], para_len);
        } break;
        case DSP_MIC_GT_REVERB: {
            rkdemo_btsoc_notify_mic_reverb(opcode, &buff[4], para_len);
        } break;
        case DSP_MIC_GT_MUTE: {
            rkdemo_btsoc_notify_mic_mute(opcode, &buff[4], para_len);
        } break;
        case DSP_MIC_GT_MUX: {
            rkdemo_btsoc_notify_mic_mux(opcode, &buff[4], para_len);
        } break;
        case DSP_MIC_GT_3A: {
            rkdemo_btsoc_notify_3a_effect(opcode, &buff[4], para_len);
        } break;
        case DSP_MIC_GT_ECHO_MODE: {
            rkdemo_btsoc_notify_echo_mode(opcode, &buff[4], para_len);
        } break;
    }
}

vendor_data_recv_handler_t vendor_get_data_recv_func(void) {
    return rkdemo_uart_data_recv_handler;
}

uart_data_recv_class_t* uart_data_recv_init(void) {
    uart_data_recv_class_t *ctx = os_malloc(sizeof(uart_data_recv_class_t));
    assert(ctx != MULL);
    memset(ctx, 0, sizeof(struct uart_data_recv_class));
    return ctx;
}

int btsoc_register_vendor_notify_func(const NotifyFuncs_t* notify_funcs) {
    rkdemoNotifyFuncs = notify_funcs;
    return 0;
}

int btsoc_register_uart_write_fd(int fd) {
    rkWriteUartfd = fd;
    return 0;
}