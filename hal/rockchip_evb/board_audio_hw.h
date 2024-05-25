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

#define AUDIO_CARD_EXT_BT       NULL
#define AUDIO_CARD_EXT_USB      NULL
#define AUDIO_CARD_EXT_AUX      NULL

#define AUDIO_CARD_CHIP_VAD       "hw:0,0"
#define AUDIO_CARD_RKCHIP_BT    "hw:7,1,0"
#define AUDIO_CARD_RKCHIP_UAC   "hw:2,0"

#define HW_SUPPORT_SRCS (MASK_SRC_CHIP_USB|MASK_SRC_CHIP_BT|MASK_SRC_CHIP_UAC)
#define FAVOR_SRC_ORDER {SRC_CHIP_BT, SRC_CHIP_USB, SRC_CHIP_UAC, SRC_EXT_BT, SRC_EXT_USB, SRC_EXT_AUX}

#ifdef __cplusplus
}
#endif
#endif