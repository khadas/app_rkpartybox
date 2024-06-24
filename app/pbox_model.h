
#ifndef _PTBOX_MODEL_H_
#define _PTBOX_MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ENABLE_LCD_DISPLAY
#define ENABLE_LCD_DISPLAY 0
#endif

#if ENABLE_LCD_DISPLAY==0
#undef LV_USE_DEMO_MUSIC
#define LV_USE_DEMO_MUSIC 0
#endif

#ifndef ENABLE_EXT_BT_MCU
#define ENABLE_EXT_BT_MCU 0
#endif

#ifndef ENABLE_RK_LED_EFFECT
#define ENABLE_RK_LED_EFFECT 1
#endif

#ifndef MAX_APP_NAME_LENGTH
#define MAX_APP_NAME_LENGTH 255
#endif

#ifndef MAX_MUSIC_NAME_LENGTH
#define MAX_MUSIC_NAME_LENGTH (MAX_APP_NAME_LENGTH*2)
#endif

#ifndef ENABLE_RK_ROCKIT
#define ENABLE_RK_ROCKIT    1
#endif

#ifndef DEFAULT_SAMPLE_FREQ
#define DEFAULT_SAMPLE_FREQ 48000
#endif

#ifndef DEFAULT_MIC_3A
#define DEFAULT_MIC_3A      true
#endif

#ifndef MIC_NUM
#define MIC_NUM             1
#endif

#ifndef MAX_MAIN_VOLUME
#define MAX_MAIN_VOLUME   0
#endif

#ifndef MIN_MAIN_VOLUME
#define MIN_MAIN_VOLUME   (-40)
#endif

#ifndef MIN_MAIN_VOLUME_MUTE
#define MIN_MAIN_VOLUME_MUTE (-80)
#endif

#ifndef DEFAULT_VOLUME
#define DEFAULT_VOLUME ((MAX_MAIN_VOLUME+MIN_MAIN_VOLUME)/2)
#endif

#ifndef MAX_MIC_PHONE_VOLUME
#define MAX_MIC_PHONE_VOLUME   0
#endif

#ifndef MIN_MIC_PHONE_VOLUME
#define MIN_MIC_PHONE_VOLUME   (-80)
#endif

#ifndef DEFAULT_HUMAN_LEVEL
#define DEFAULT_HUMAN_LEVEL 15
#endif

#ifdef __cplusplus
}
#endif

#endif
