#ifndef _PTBOX_HARDWARE_BOARD_AUDIO_VENDOR_H_
#define _PTBOX_HARDWARE_BOARD_AUDIO_VENDOR_H_

#include "hal_input.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_CARD_EXT_BT       "hw:1,0"
#define AUDIO_CARD_EXT_USB      AUDIO_CARD_EXT_BT
#define AUDIO_CARD_EXT_AUX      AUDIO_CARD_EXT_BT

#define AUDIO_CARD_RKCHIP_BT      NULL
#define AUDIO_CARD_RKCHIP_UAC      NULL

#define HW_SUPPORT_SRCS (MASK_SRC_EXT_BT|MASK_SRC_EXT_USB|MASK_SRC_EXT_AUX)

#ifdef __cplusplus
}
#endif
#endif