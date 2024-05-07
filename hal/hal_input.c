
#include "hal_input.h"

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

int get_userspace_key_from_kernel(int value) {
    for (int i = 0; i < sizeof(KEY_TABLE)/sizeof(key_pair_t); i++) {
        if(value == KEY_TABLE[i].kernel_space)
            return KEY_TABLE[i].user_space;
    }

    return HKEY_IDLE;
}