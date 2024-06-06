
#ifndef _PBOX_LED_APP_H_
#define _PBOX_LED_APP_H_
#include <stdint.h>
#include <stdbool.h>
#include "pbox_common.h"
#include "rk_btsink.h"
#ifdef __cplusplus
extern "C" {
#endif

// Function declarations for LED effects
void pbox_app_led_PlayPause(bool play);
void pbox_app_led_displayGender(gender_t gender);
void pbox_app_led_PrevNext(bool next);
void pbox_app_led_MusicVolumeLevel(uint32_t mVolume);
void pbox_app_led_MicVolumeLevel(uint32_t micVolume);
void pbox_app_led_AccompMusicLevel(uint32_t accomp_music_level);
void pbox_app_led_HumanMusicLevel(uint32_t human_music_level);
void pbox_app_led_ReservLevel(uint32_t reserv_music_level);
void pbox_app_led_energyInfo(energy_info_t energy);
void pbox_app_led_btState(btsink_state_t mode);
void pbox_app_led_uacState(bool start);
void pbox_app_led_uacVolume(uint32_t volume);
void pbox_app_led_startup_effect(void);

#define pbox_app_led_uacState(...)          ((void) 0)
#define pbox_app_led_uacVolume(...)         ((void) 0)

#define pbox_app_led_TrackInfo(...)         ((void) 0)
#define pbox_app_led_TrackPosition(...)     ((void) 0)
#define pbox_app_led_vocalFadeout_switch(...) ((void) 0)
#define pbox_app_led_echo3A(...)            ((void) 0)
#define pbox_app_led_revertMode(...)        ((void) 0)
#define pbox_app_led_usbState(...)          ((void) 0)

#define pbox_app_lcd_displayEqMode(...)     ((void) 0)
#define pbox_app_led_MusicStereoMode(...)   ((void) 0)
#define pbox_app_led_MusicPlaceMode(...)    ((void) 0)
#define pbox_app_led_MusicOutdoorMode(...)  ((void) 0)
#define pbox_app_led_MicMux(...)            ((void) 0)
#define pbox_app_led_MicTreble(...)         ((void) 0)
#define pbox_app_led_MicBass(...)           ((void) 0)
#define pbox_app_led_MicReverb(...)         ((void) 0)
#ifdef __cplusplus
}
#endif

#endif