#ifndef _PBOX_INTERFACE_H_
#define _PBOX_INTERFACE_H_

#include <stdint.h>

// Enum definition for the abstract layer
typedef enum
{
    INTERFACE_IDLE = 0,
    INTERFACE_PLAYING,
    INTERFACE_PAUSE,
    INTERFACE_STOP,
    INTERFACE_PLAY_NUM
} InterfacePlayStatus;

// Function declaration to convert upper layer enum to abstract layer enum
InterfacePlayStatus map_play_status_to_interface(uint32_t status);

int32_t get_maxMicVolume(void);
int32_t get_minMicVolume(void);

#endif // _PBOX_INTERFACE_H_
