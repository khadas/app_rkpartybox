/************* UI include two part:*************************
***************************1. LED effect show **************
***************************2. LCD screnn display ***********
* mainly here, almost are command to display something, so, 
* when key pressed, scream pressed, or bt info feedback,
* we may call api in this page to display something.
************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pbox_common.h"
#include "pbox_lvgl_app.h"
#include "pbox_led_app.h"
#include "pbox_app.h"

#define LED_DISPLAY_MASK DISP_LED
#define LCD_DISPLAY_MASK DISP_LCD

void pbox_multi_displayIsPlaying(bool play, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayPlayPause(play);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_PlayPause(play);
}

void pbox_multi_displayPrevNext(bool next, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayPrevNext(next);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_PrevNext(next);
}

void pbox_multi_displayTrackInfo(const char* title, const char* artist, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayTrackInfo(title, artist);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_TrackInfo(title, artist);
}

void pbox_multi_displayTrackPosition(uint32_t mCurrent, uint32_t mDuration, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayTrackPosition(mCurrent, mDuration);
    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_TrackPosition(mCurrent, mDuration);
}

void pbox_multi_displayMainVolumeLevel(uint32_t mainVolume, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMainVolumeLevel(mainVolume);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_MainVolumeLevel(mainVolume);
}

void pbox_multi_displayMicVolumeLevel(uint32_t micVolume, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMicVolumeLevel(micVolume);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_MicVolumeLevel(micVolume);
}

void pbox_multi_displayAccompMusicLevel(uint32_t accomp_music_level, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayAccompMusicLevel(accomp_music_level);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_AccompMusicLevel(accomp_music_level);
}

void pbox_multi_displayHumanMusicLevel(uint32_t human_music_level, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayHumanMusicLevel(human_music_level);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_HumanMusicLevel(human_music_level);
}

void pbox_multi_displayReservLevel(uint32_t reserv_music_level, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayReservLevel(reserv_music_level);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_ReservLevel(reserv_music_level);
}

void pbox_multi_displayMusicSeparateSwitch(bool enable, uint32_t hlevel, uint32_t mlevel, uint32_t rlevel, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMusicSeparateSwitch(enable, hlevel, mlevel, rlevel);

    if(policy & LED_DISPLAY_MASK)
    pbox_app_led_MusicSeparateSwitch(enable, hlevel, mlevel, rlevel);
}

void pbox_multi_displayEcho3A(bool enable, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayEcho3A(enable);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_echo3A(enable);
}

void pbox_multi_displayRevertMode(pbox_revertb_t mode, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayRevertbMode(mode);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_revertMode(mode);
}

void pbox_multi_displayEnergyInfo(energy_info_t energy, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_dispplayEnergy(energy);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_energyInfo(energy);
}

#undef LED_DISPLAY_MASK
#undef LCD_DISPLAY_MASK