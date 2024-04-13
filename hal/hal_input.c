
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