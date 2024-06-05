
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
    {115,       HKEY_VOLUP}, //KEY_VOLUMEUP
    {114,       HKEY_VOLDOWN}, //HKEY_VOLDOWN
    {248,       HKEY_MIC1MUTE}, //KEY_MICMUTE
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
        case SRC_CHIP_UAC:{
            if (is_rolling_board())
                return "hw:3,0";//todo AUDIO_CARD_RKCHIP_UAC_ROLLING
            else
                return AUDIO_CARD_RKCHIP_UAC;
        }
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

char *hal_get_audio_vad_card(void) {
    return AUDIO_CARD_CHIP_VAD;
}

const input_source_t frozenSrcOrder[SRC_NUM] = FAVOR_SRC_ORDER;
input_source_t hal_get_favor_source_order(int index) {
    return frozenSrcOrder[index];
}

uint32_t hal_get_supported_sources(void) {
    return HW_SUPPORT_SRCS;
}