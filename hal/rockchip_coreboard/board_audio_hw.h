#ifndef _PTBOX_HARDWARE_BOARD_AUDIO_EVB_H_
#define _PTBOX_HARDWARE_BOARD_AUDIO_EVB_H_
/*--------------------attention--------------------------
* this file is only used in hal dir. 
* for other dir, pls just include hal_partybox.h only
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

#define AUDIO_CARD_SPK_CODEC    "hw:0,0"
#define AUDIO_CARD_CHIP_KALAOK  "hw:0,0"
#define AUDIO_CARD_CHIP_SCENE   NULL
#define AUDIO_CARD_EXT_BT       "hw:1,0"
#define AUDIO_CARD_EXT_USB      AUDIO_CARD_EXT_BT
#define AUDIO_CARD_EXT_AUX      AUDIO_CARD_EXT_BT

#define AUDIO_CARD_RKCHIP_BT      NULL
#define AUDIO_CARD_RKCHIP_UAC      NULL
#define AUDIO_CARD_RKCHIP_USB      NULL

//this is rockchip rockit usage
//--------------start------------------
#define SPK_CODEC_CHANNEL       6

#define KALAOK_REC_CHANNEL      8
#define KALAOK_POOR_COUNT       0
#define KALAOK_REF_LAYOUT       0xfc
#define KALAOK_REC_LAYOUT       0x03
#define KALAOK_REF_CHN_LAYOUT   0xff
#define KALAOK_REF_HARD_MODE    0
//--------------end--------------------

//-------this not used for demo vendor---
#define SCENE_REC_CHANNEL       4
#define SCENE_REF_LAYOUT        0x0c
#define SCENE_REC_LAYOUT        0x03
#define SCENE_REF_HARD_MODE     0
//--------------end----------------------

#define MAX_SARA_ADC 1023
#define MIN_SARA_ADC 0

#define HW_SUPPORT_SRCS (MASK_SRC_EXT_BT|MASK_SRC_EXT_USB|MASK_SRC_EXT_AUX)
#define FAVOR_SRC_ORDER {SRC_CHIP_BT, SRC_CHIP_USB, SRC_CHIP_UAC, SRC_EXT_BT, SRC_EXT_USB, SRC_EXT_AUX}

#ifdef __cplusplus
}
#endif
#endif