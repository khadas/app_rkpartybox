#ifndef _PTBOX_HARDWARE_BOARD_H_
#define _PTBOX_HARDWARE_BOARD_H_
#include "pbox_model.h"

#ifdef ENABLE_EXT_BT_MCU
#include "board_B2317.h"
#else
#include "board_audio_evb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

char* hardware_get_audio_card(void);

#ifdef __cplusplus
}
#endif
#endif