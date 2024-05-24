#ifndef _PTBOX_HARDWARE_BOARD_AUDIO_VENDOR_H_
#define _PTBOX_HARDWARE_BOARD_AUDIO_VENDOR_H_
/*--------------------attention--------------------------
* this file is only used in hal dir. for other dir,
* pls just include hal_partybox.h only
* -------------------------------------------------------*/
#include "hal_partybox.h"
#ifdef __cplusplus
extern "C" {
#endif

#define DSP_MAIN_MAX_VOL        32
#define DSP_MUSIC_MAX_VOL       32
#define DSP_MIC_REVERB_MAX_VOL  32
#define DSP_MIC_TREBLE_MAX_VOL  32
#define DSP_MIC_BASS_MAX_VOL    32
#define DSP_MIC_MAX_VOL         32

#define AUDIO_CARD_CHIP_VAD       "hw:0,0"
#define AUDIO_CARD_EXT_BT       "hw:1,0"
#define AUDIO_CARD_EXT_USB      AUDIO_CARD_EXT_BT
#define AUDIO_CARD_EXT_AUX      AUDIO_CARD_EXT_BT

#define AUDIO_CARD_RKCHIP_BT      NULL
#define AUDIO_CARD_RKCHIP_UAC      NULL
#define AUDIO_CARD_RKCHIP_USB      NULL

#define HW_SUPPORT_SRCS (MASK_SRC_EXT_BT|MASK_SRC_EXT_USB|MASK_SRC_EXT_AUX)
#define FAVOR_SRC_ORDER {SRC_CHIP_BT, SRC_CHIP_USB, SRC_CHIP_UAC, SRC_EXT_BT, SRC_EXT_USB, SRC_EXT_AUX}

#ifdef __cplusplus
}
#endif
#endif