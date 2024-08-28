
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include "board_audio_hw.h"
#include "pbox_interface.h"

typedef struct {
    int kernel_space;
    int user_space;
} key_pair_t;

const input_source_t frozenSrcOrder[SRC_NUM] = FAVOR_SRC_ORDER;

char* getInputSourceString(input_source_t source) {
    switch (source) {
        case SRC_CHIP_USB: return "usb";
        case SRC_CHIP_BT: return "bt";
        case SRC_CHIP_UAC: return "uac";
        case SRC_EXT_BT: return "ext_bt";
        case SRC_EXT_USB: return "ext_usb";
        case SRC_EXT_AUX: return "ext_aux";
        default: return "unkown";
    }
}

static const key_pair_t KEY_TABLE[] = {
    /*kernel    user*/
    {373,       HKEY_MODE}, //KEY_MODE
    {207,       HKEY_PLAY}, //KEY_PLAY
    {209,       HKEY_GPIO_BOOST},
    {115,       HKEY_VOLUP}, //KEY_VOLUMEUP
    {114,       HKEY_VOLDOWN}, //HKEY_VOLDOWN
    {248,       HKEY_MIC1MUTE}, //KEY_MICMUTE
    {0x21e,     HKEY_GPIO_LIGHTS},
};

int hal_key_convert_kernel_to_upper(uint32_t value) {
    for (int i = 0; i < sizeof(KEY_TABLE)/sizeof(key_pair_t); i++) {
        if(value == KEY_TABLE[i].kernel_space)
            return KEY_TABLE[i].user_space;
    }

    return HKEY_IDLE;
}

char *hal_get_audio_card(input_source_t source) {
    switch(source) {
        //case SRC_CHIP_USB:  return AUDIO_CARD_RKCHIP_USB; //no need
        case SRC_CHIP_BT:   return AUDIO_CARD_RKCHIP_BT;
        case SRC_CHIP_UAC:  return AUDIO_CARD_RKCHIP_UAC;
        case SRC_EXT_BT:    return AUDIO_CARD_EXT_BT;
        case SRC_EXT_USB:   return AUDIO_CARD_EXT_USB;
        case SRC_EXT_AUX:   return AUDIO_CARD_EXT_AUX;
        default: {
            printf("%s: no such input source:%d\n", __func__, source);
            return NULL;
        }
    }
    return NULL;
}

//this is speaker or codec
char *hal_get_spk_codec_card(void) {
    return AUDIO_CARD_SPK_CODEC;
}

uint8_t hal_get_audio_spk_codec_channel(void) {
    return SPK_CODEC_CHANNEL;
}

//this is guitar
char *hal_get_kalaok_guitar_card(void) {
    return AUDIO_CARD_CHIP_GUITAR;
}

//this is kalaok mic
char *hal_get_kalaok_mic_card(void) {
    return AUDIO_CARD_CHIP_KALAOK;
}

uint8_t hal_get_kalaok_mic_ref_layout(void) {
    return KALAOK_REF_LAYOUT;
}

uint8_t hal_get_kalaok_mic_rec_layout(void) {
    return KALAOK_REC_LAYOUT;
}

uint8_t hal_get_kalaok_mic_chn_layout(void) {
    return KALAOK_REF_CHN_LAYOUT;
}

uint8_t hal_get_kalaok_mic_rec_channel(void) {
    return KALAOK_REC_CHANNEL;
}

uint8_t hal_get_kalaok_poor_count(void) {
    return KALAOK_POOR_COUNT;
}

uint8_t hal_get_kalaok_ref_hard_mode(void) {
    return KALAOK_REF_HARD_MODE;
}

uint32_t hal_get_kalaok_rec_sample_rate(void) {
    return KALAOK_REC_SAMPLE_RATE;
}

//=============scene mic=====================
char *hal_get_audio_scene_card(void) {
    return AUDIO_CARD_CHIP_SCENE;
}
uint8_t hal_get_scene_mic_ref_layout(void) {
    return SCENE_REF_LAYOUT;
}
uint8_t hal_get_scene_mic_rec_layout(void) {
    return SCENE_REC_LAYOUT;
}
uint8_t hal_get_scene_mic_rec_channel(void) {
    return SCENE_REC_CHANNEL;
}
uint8_t hal_get_scene_ref_hard_mode(void) {
    return SCENE_REF_HARD_MODE;
}
uint32_t hal_get_scene_rec_sample_rate(void) {
    return SCENE_REC_SAMPLE_RATE;
}

input_source_t hal_get_favor_source_order(int index) {
    return frozenSrcOrder[index];
}
uint32_t hal_get_supported_sources(void) {
    return HW_SUPPORT_SRCS;
}

uint32_t hal_get_supported_mic_matrix(void) {
    return HW_MIC_MATRIX;
}

bool hal_get_sara_adc_usage(void) {
    return USE_SARA_ADC_KEY;
}

uint32_t hal_get_mic_guitar_num(void) {
    return HW_MIC_GUITAR_NUM;
}

uint32_t hal_get_mic_num(void) {
    uint32_t num;
    // for (int i < 0; i< HW_MIC_GUITAR_NUM; i++) {
    //     if(HW_MIC_MATRIX&(1<<i)) {
    //         num++;
    //     }
    // }
    // return num;
    switch(HW_MIC_MATRIX) {
        case 1: {
            num = 1;
        } break;
        case 3: {
            num = 2;
        } break;
        case 7: {
            num = 3;
        } break;
        default: return 0;
    }
    assert(num <= HW_MIC_GUITAR_NUM);
    return num;
}