#include "pbox_interface.h"
#include "pbox_common.h"  // Assuming this header file contains the upper layer's enum definition

InterfacePlayStatus map_play_status_to_interface(uint32_t status) {
    switch (status) {
        case IDLE:
            return INTERFACE_IDLE;
        case PLAYING:
            return INTERFACE_PLAYING;
        case _PAUSE:
            return INTERFACE_PAUSE;
        case _STOP:
            return INTERFACE_STOP;
        default:
            return INTERFACE_PLAY_NUM;
    }
}

int32_t get_maxMicVolume(void) {
    return MAX_MIC_PHONE_VOLUME;
}

int32_t get_minMicVolume(void) {
    return MIN_MIC_PHONE_VOLUME;
}


