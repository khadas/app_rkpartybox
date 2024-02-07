
#ifndef _PTBOX_MODEL_H_
#define _PTBOX_MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_EXT_BT_MCU
#define ENABLE_LCD_DISPLAY 0
#define ENABLE_USE_SOCBT 1
#define ENABLE_RK_LED_EFFECT 0
#define MAX_APP_NAME_LENGTH 127
#define MAX_MUSIC_NAME_LENGTH (MAX_APP_NAME_LENGTH*2)
#define ENABLE_RK_ROCKIT    1
#define ENABLE_UAC    0
#else
#define ENABLE_LCD_DISPLAY 1
#define ENABLE_USE_SOCBT 0
#define ENABLE_RK_LED_EFFECT 1
#define MAX_APP_NAME_LENGTH 255
#define MAX_MUSIC_NAME_LENGTH (MAX_APP_NAME_LENGTH*2)
#define ENABLE_RK_ROCKIT    1
#define ENABLE_UAC    1
#endif

#ifdef __cplusplus
}
#endif

#endif
