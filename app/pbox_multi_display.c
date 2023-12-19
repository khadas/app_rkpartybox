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

void pbox_multi_displayPlayPause(bool play) {
    pbox_app_lcd_displayPlayPause(play);
    pbox_app_led_PlayPause(play);
}

void pbox_multi_displayPrevNext(bool next) {
    pbox_app_lcd_displayPrevNext(next);
    pbox_app_led_PrevNext(next);
}

void pbox_multi_displayTrackInfo(const char* title, const char* artist) {
    pbox_app_lcd_displayTrackInfo(title, artist);
    pbox_app_led_TrackInfo(title, artist);
}

void pbox_multi_displayTrackPosition(unsigned int mCurrent, unsigned int mDuration) {
    pbox_app_lcd_displayTrackPosition(mCurrent, mDuration);
    pbox_app_led_TrackPosition(mCurrent, mDuration);
}

void pbox_multi_displayMainVolumeLevel(uint32_t mainVolume) {
    pbox_app_lcd_displayMainVolumeLevel(mainVolume);
    pbox_app_led_MainVolumeLevel(mainVolume);
}

void pbox_multi_displayMicVolumeLevel(uint32_t micVolume) {
    pbox_app_lcd_displayMicVolumeLevel(micVolume);
    pbox_app_led_MicVolumeLevel(micVolume);
}

void pbox_multi_displayAccompMusicLevel(uint32_t accomp_music_level) {
    pbox_app_lcd_displayAccompMusicLevel(accomp_music_level);
    pbox_app_led_AccompMusicLevel(accomp_music_level);
}

void pbox_multi_displayHumanMusicLevel(uint32_t human_music_level) {
    pbox_app_lcd_displayHumanMusicLevel(human_music_level);
    pbox_app_led_HumanMusicLevel(human_music_level);
}

void pbox_multi_displayGuitarLevel(uint32_t guitar_music_level) {
    pbox_app_lcd_displayGuitarLevel(guitar_music_level);
    pbox_app_led_GuitarLevel(guitar_music_level);
}

void pbox_multi_displayMusicSeparateSwitch(bool enable, uint32_t hlevel, uint32_t mlevel, uint32_t glevel) {
    pbox_app_lcd_displayMusicSeparateSwitch(enable, hlevel, mlevel, glevel);
    pbox_app_led_MusicSeparateSwitch(enable, hlevel, mlevel, glevel);
}
