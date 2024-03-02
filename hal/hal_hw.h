#ifndef _HAL_HW_H_
#define _HAL_HW_H_
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DSP_MAIN_MAX_VOL        32
#define DSP_MUSIC_MAX_VOL       32
#define DSP_MIC_REVERB_MAX_VOL  32
#define DSP_MIC_TREBLE_MAX_VOL  32
#define DSP_MIC_BASS_MAX_VOL    32
#define DSP_MIC_MAX_VOL         32

float HW_MAIN_GAIN(uint8_t index);
float HW_MUSIC_GAIN(uint8_t index);
float HW_MIC_REVERB(uint8_t index);
float HW_MIC_TREBLE(uint8_t index);
float HW_MIC_BASS(uint8_t index);
float HW_GT_REVERB(uint8_t index);
float HW_GT_TREBLE(uint8_t index);
float HW_GT_BASS(uint8_t index);

#ifdef __cplusplus
}
#endif
#endif