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
#include "rk_btsink.h"
#include "pbox_lvgl_app.h"
#include "pbox_light_effect_app.h"
#include "pbox_app.h"

#define LEFT_SHIFT_COUNT(num) (__builtin_ctz(num))

#define LED_DISPLAY_MASK (ENABLE_RK_LED_EFFECT<<LEFT_SHIFT_COUNT(DISP_LED))
#define LCD_DISPLAY_MASK (ENABLE_LCD_DISPLAY<<LEFT_SHIFT_COUNT(DISP_LCD))

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

void pbox_multi_displayTrackPosition(bool durationOnly, uint32_t mCurrent, uint32_t mDuration, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayTrackPosition(durationOnly, mCurrent, mDuration);
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

void pbox_multi_displayMicMute(bool mute, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMicMute(mute);
}

void pbox_multi_displayMicMux(uint8_t index, mic_mux_t mux, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMicMux(index, mux);
    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_MicMux(index, mux);
}

void pbox_multi_displayMicTreble(uint8_t index, uint32_t treble, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMicTreble(index, treble);
    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_MicTreble(index, treble);
}

void pbox_multi_displayMicBass(uint8_t index, uint32_t bass, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMicBass(index, bass);
    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_MicBass(index, bass);
}

void pbox_multi_displayMicReverb(uint8_t index, uint32_t reverb, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMicReverb(index, reverb);
    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_MicReverb(index, reverb);
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

void pbox_multi_displayMusicStereoMode(stereo_mode_t stereo, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMusicStereoMode(stereo);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_MusicStereoMode(stereo);
}

void pbox_multi_displayMusicPlaceMode(placement_t place, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMusicPlaceMode(place);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_MusicPlaceMode(place);
}

void pbox_multi_displayMusicOutdoorMode(inout_door_t outdoor, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMusicOutdoorMode(outdoor);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_MusicOutdoorMode(outdoor);
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

void pbox_multi_displayUsbListupdate(uint32_t trackId, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_dispplayUsbListupdate(trackId);
}

void pbox_multi_displayUsbState(usb_state_t state, display_t policy) {

    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_dispplayUsbState(state);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_usbState(state);
}

void pbox_multi_displaybtState(btsink_state_t state, display_t policy) {

    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_dispplaybtState(state);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_btState(state);
}

void pbox_multi_displayUacState(uac_role_t role, bool start, display_t policy) {
    if(policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_dispplayUacState(start);

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_uacState(start);
}

void pbox_multi_displayUacFreq(uac_role_t role, uint32_t freq, display_t policy) {

}

void pbox_multi_displayUacVolume(uac_role_t role, uint32_t volume, display_t policy) {
    if(policy & LCD_DISPLAY_MASK) {
        if(role == UAC_ROLE_SPEAKER) {
            pbox_app_lcd_displayMainVolumeLevel(volume);
        } else {
            pbox_app_lcd_displayMicVolumeLevel(volume);
        }
    }

    if(policy & LED_DISPLAY_MASK)
        pbox_app_led_uacVolume(volume);
}

void pbox_multi_displayUacMute(uac_role_t role, bool mute, display_t policy) {

}

void pbox_multi_displayUacPpm(uac_role_t role, int32_t ppm, display_t policy) {

}

#undef LED_DISPLAY_MASK
#undef LCD_DISPLAY_MASK
