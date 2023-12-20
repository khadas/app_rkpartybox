
#ifndef _PBOX_LED_APP_H_
#define _PBOX_LED_APP_H_
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

// Function declarations for LED effects
void pbox_app_led_PlayPause(bool play);
void pbox_app_led_PrevNext(bool next);
void pbox_app_led_TrackInfo(const char* title, const char* artist);
void pbox_app_led_TrackPosition(unsigned int mCurrent, unsigned int mDuration);
void pbox_app_led_MainVolumeLevel(uint32_t mainVolume);
void pbox_app_led_MicVolumeLevel(uint32_t micVolume);
void pbox_app_led_AccompMusicLevel(uint32_t accomp_music_level);
void pbox_app_led_HumanMusicLevel(uint32_t human_music_level);
void pbox_app_led_ReservLevel(uint32_t reserv_music_level);
void pbox_app_led_MusicSeparateSwitch(bool enable, uint32_t hlevel, uint32_t mlevel, uint32_t rlevel);
void pbox_app_led_energyInfo(energy_info_t energy);
void pbox_app_led_echo3A(bool enable);
void pbox_app_led_revertMode(pbox_revertb_t mode);
#ifdef __cplusplus
}
#endif

#endif  // _PBOX_MULTI_DISPLAY_H_