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
void pbox_app_lcd_displayReservLevel(uint32_t reserv_music_level);
void pbox_app_lcd_displayMusicSeparateSwitch(bool enable, uint32_t hlevel, uint32_t mlevel, uint32_t rlevel);
void pbox_app_lcd_dispplayEnergy(energy_info_t energy);
void pbox_app_lcd_displayEcho3A(bool on);
void pbox_app_lcd_displayRevertbMode(pbox_revertb_t mode);
void pbox_app_lcd_dispplayReflash(void);
void pbox_app_lcd_dispplayUsbState(usb_state_t state);
void maintask_lvgl_fd_process(int fd);

#ifdef __cplusplus
}
#endif
#endif