#ifndef _PTBOX_HARDWARE_BOARD_H_
#define _PTBOX_HARDWARE_BOARD_H_

#include "hal_input.h"

#ifdef __cplusplus
extern "C" {
#endif

#if ENABLE_EXT_BT_MCU
#include "board_audio_vendor.h"
#else
#include "board_audio_evb.h"
#endif

char* hardware_get_audio_card(void);
char* getInputSourceString(input_source_t source);
int get_userspace_key_from_kernel(uint32_t value);

#define PBOX_INPUT_SRCS HW_SUPPORT_SRCS
#define FAVOR_SRC_ORDER {SRC_CHIP_BT, SRC_CHIP_USB, SRC_CHIP_UAC, SRC_EXT_BT, SRC_EXT_USB, SRC_EXT_AUX}

#ifdef __cplusplus
}
#endif
#endif