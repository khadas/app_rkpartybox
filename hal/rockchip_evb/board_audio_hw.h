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

#define AUDIO_CARD_SPK_CODEC    "hw:0,0"
#define AUDIO_CARD_CHIP_GUITAR  NULL
#define AUDIO_CARD_CHIP_KALAOK  "mic"
#define AUDIO_CARD_CHIP_SCENE   "scene"
#define AUDIO_CARD_RKCHIP_BT    "hw:7,1,0"
#define AUDIO_CARD_RKCHIP_UAC   "hw:3,0"

//this is rockchip rockit usage
//--------------start------------------
#define SPK_CODEC_CHANNEL       2

#define KALAOK_REC_CHANNEL      4
#define KALAOK_POOR_COUNT       0
#define KALAOK_REF_LAYOUT       0x03
#define KALAOK_REC_LAYOUT       0x04
#define KALAOK_REF_CHN_LAYOUT   0x0f
#define KALAOK_REF_HARD_MODE    1
#define KALAOK_REC_SAMPLE_RATE  48000

#define SCENE_REC_CHANNEL       4
#define SCENE_REF_LAYOUT        0x03
#define SCENE_REC_LAYOUT        0x0c
#define SCENE_REF_HARD_MODE     0
#define SCENE_REC_SAMPLE_RATE  48000
//--------------end--------------------

#define MAX_SARA_ADC 1023
#define MIN_SARA_ADC 0

#define HW_SUPPORT_SRCS (MASK_SRC_CHIP_USB|MASK_SRC_CHIP_BT|MASK_SRC_CHIP_UAC)
#define FAVOR_SRC_ORDER {SRC_CHIP_BT, SRC_CHIP_USB, SRC_CHIP_UAC, SRC_EXT_BT, SRC_EXT_USB, SRC_EXT_AUX}

//set BITx_MIC_MUX (MASK_MIC/GUITAR << x) to support mic/guitar
#define BIT0_MIC_MUX    (MASK_MIC << 0)//index 0 is mic
#define BIT1_MIC_MUX    (MASK_MIC << 1)//index 1 also is mic.
#define HW_MIC_MATRIX   (BIT0_MIC_MUX|BIT1_MIC_MUX)

#ifdef __cplusplus
}
#endif
#endif