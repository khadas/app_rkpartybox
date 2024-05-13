#ifndef _PTBOX_HARDWARE_BOARD_AUDIO_EVB_H_
#define _PTBOX_HARDWARE_BOARD_AUDIO_EVB_H_

#include "hal_input.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_CARD_EXT_BT       NULL
#define AUDIO_CARD_EXT_USB      NULL
#define AUDIO_CARD_EXT_AUX      NULL

#define AUDIO_CARD_CHIP_VAD       "hw:0,0"
#define AUDIO_CARD_RKCHIP_BT    "hw:7,1,0"
#define AUDIO_CARD_RKCHIP_UAC   "hw:2,0"

#define HW_SUPPORT_SRCS (MASK_SRC_CHIP_USB|MASK_SRC_CHIP_BT|MASK_SRC_CHIP_UAC)

#ifdef __cplusplus
}
#endif
#endif