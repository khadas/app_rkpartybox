#ifndef _PTBOX_LVGL_APP_H_
#define _PTBOX_LVGL_APP_H_
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

void pbox_app_lcd_displayPlayPause(bool play);

void pbox_app_lcd_displayPrevNext(bool next);

void pbox_app_lcd_displayTrackInfo(const char* title, const char* artist);

void pbox_app_lcd_displayTrackPosition(unsigned int mCurrent, unsigned int mDuration);

void pbox_app_lcd_displayMainVolumeLevel(uint32_t mainVolume);

void pbox_app_lcd_displayMicVolumeLevel(uint32_t micVolume);

void pbox_app_lcd_displayAccompMusicLevel(uint32_t accomp_music_level);

void pbox_app_lcd_displayHumanMusicLevel(uint32_t human_music_level);

void pbox_app_lcd_displayGuitarLevel(uint32_t guitar_music_level);

void pbox_app_lcd_displayMusicSeparateSwitch(bool enable, uint32_t hlevel, uint32_t mlevel, uint32_t glevel);

void pbox_app_lcd_dispplayEnergy(energy_info_t energy);

void maintask_lvgl_fd_process(int fd);

#ifdef __cplusplus
}
#endif
#endif