
#ifndef _PTBOX_MODEL_H_
#define _PTBOX_MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_EXT_BT_MCU
#define ENABLE_LCD_DISPLAY  0
#define ENABLE_USE_SOCBT    1
#define ENABLE_RK_LED_EFFECT    0
#define MAX_APP_NAME_LENGTH     127
#define MAX_MUSIC_NAME_LENGTH   (MAX_APP_NAME_LENGTH*2)
#define ENABLE_RK_ROCKIT    1
#define ENABLE_UAC          0
#define DEFAULT_SAMPLE_FREQ 48000
#define DEFAULT_MIC_3A      true
#define ENABLE_EXT_MCU_USB  1
#define ENABLE_AUX          1
#define MIC_NUM             2
#define ENABLE_RAW_PCM      0
#define ENABLE_SARAADC   1
#define MAX_MAIN_VOLUME   0
#define MIN_MAIN_VOLUME   (-100)
#define MIN_MAIN_VOLUME_MUTE (MIN_MAIN_VOLUME)
#define DEFAULT_MAIN_VOLUME ((MAX_MAIN_VOLUME+MIN_MAIN_VOLUME)/2)
#define MAX_MIC_PHONE_VOLUME   0
#define MIN_MIC_PHONE_VOLUME   (-100)
#else
#define ENABLE_LCD_DISPLAY 1
#define ENABLE_USE_SOCBT 0
#define ENABLE_RK_LED_EFFECT 1
#define MAX_APP_NAME_LENGTH 255
#define MAX_MUSIC_NAME_LENGTH (MAX_APP_NAME_LENGTH*2)
#define ENABLE_RK_ROCKIT    1
#define ENABLE_UAC    1
#define DEFAULT_SAMPLE_FREQ 44100
#define DEFAULT_MIC_3A      true
#define ENABLE_EXT_MCU_USB   0
#define ENABLE_AUX          0
#define MIC_NUM             1
#define ENABLE_RAW_PCM      0
#define ENABLE_SARAADC   0
#define MAX_MAIN_VOLUME   0
#define MIN_MAIN_VOLUME   (-40)
#define MIN_MAIN_VOLUME_MUTE (-80)
#define DEFAULT_MAIN_VOLUME ((MAX_MAIN_VOLUME+MIN_MAIN_VOLUME)/2)
#define MAX_MIC_PHONE_VOLUME   0
#define MIN_MIC_PHONE_VOLUME   (-100)
#endif

#ifdef __cplusplus
}
#endif

#endif
