/************* UI include two part:**************************
***************************1. LED effect show ***************
***************************2. LCD screen display ************
***************************3. BTMCU display *****************
***************************4. echo to storage device ********
* here echo means display. we echo message to different disps
* mainly here, almost are command to echo something, so,
* when key pressed, scream pressed, or bt info feedback,
* we may call api in this page to echo something.
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
#include "pbox_store_app.h"
#include "pbox_soc_bt_app.h"


#define LEFT_SHIFT_COUNT(num) (__builtin_ctz(num))

#define LED_DISPLAY_MASK (ENABLE_RK_LED_EFFECT << LEFT_SHIFT_COUNT(DISP_LED))
#define LCD_DISPLAY_MASK (ENABLE_LCD_DISPLAY << LEFT_SHIFT_COUNT(DISP_LCD))
#define BTMCU_DISP_MASK (ENABLE_EXT_BT_MCU << LEFT_SHIFT_COUNT(DISP_BTMCU))
#define STORAGE_DISP_MASK (DISP_FS)

void pbox_multi_echoGender(gender_t gender, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayGender(gender);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_displayGender(gender);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_gender(gender);
}

void pbox_multi_echoIsPlaying(bool play, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayPlayPause(play);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_PlayPause(play);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_playPause(play);
}

void pbox_multi_echoPrevNext(bool next, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayPrevNext(next);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_PrevNext(next);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_PrevNext(next);
}

void pbox_multi_echoTrackInfo(const char *title, const char *artist, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayTrackInfo(title, artist);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_TrackInfo(title, artist);
}

void pbox_multi_echoTrackPosition(bool durationOnly, uint32_t mCurrent, uint32_t mDuration, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayTrackPosition(durationOnly, mCurrent, mDuration);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_TrackPosition(mCurrent, mDuration);
}

void pbox_multi_echoMusicVolumeLevel(float musicVolume, display_t policy)
{
    int32_t volume = TARGET2PERCENT(musicVolume, MIN_MAIN_VOLUME, MAX_MAIN_VOLUME);
    volume = volume > 100 ? 100 : volume;
    volume = volume < 0 ? 0 : volume;

    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMusicVolumeLevel(volume);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_MusicVolumeLevel(volume);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_music_volume(volume);
    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_music_volume(musicVolume);
}

void pbox_multi_echoMainVolumeLevel(float mVolume, display_t policy)
{
    int32_t volume = TARGET2PERCENT(mVolume, MIN_MAIN_VOLUME, MAX_MAIN_VOLUME);
    volume = volume > 100 ? 100 : volume;
    volume = volume < 0 ? 0 : volume;

    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_main_volume(volume);
    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_volume(mVolume);
}

void pbox_multi_echoMicVolumeLevel(uint8_t index, float micVolume, display_t policy)
{
    int32_t volume = TARGET2PERCENT(micVolume, MIN_MIC_PHONE_VOLUME, MAX_MIC_PHONE_VOLUME);
    volume = volume > 100 ? 100 : volume;
    volume = volume < 0 ? 0 : volume;

    ALOGD("%s micVolume: %f, volume: %d\n", __func__, micVolume, volume);
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMicVolumeLevel(volume);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_MicVolumeLevel(volume);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_mic_volume(index, micVolume);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_mic_volume(index, micVolume);
}

void pbox_multi_echoMicMute(uint8_t index, bool mute, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMicMute(mute);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_mic_mute(index, mute);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_mic_mute(index, mute);
}

void pbox_multi_echoMicMux(uint8_t index, mic_mux_t mux, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMicMux(index, mux);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_MicMux(index, mux);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_micMux(index, mux);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_mic_mux(index, mux);
}

void pbox_multi_echoMicTreble(uint8_t index, float treble, display_t policy)
{
    int32_t volume =  TARGET2PERCENT(treble, MIN_TREBLE_VALUE, MAX_TREBLE_VALUE);
    volume = volume > 100 ? 100 : volume;
    volume = volume < 0 ? 0 : volume;

    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMicTreble(index, volume);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_MicTreble(index, volume);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_mic_treble(index, volume);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_mic_treble(index, treble);
}

void pbox_multi_echoMicBass(uint8_t index, float bass, display_t policy)
{
    int32_t volume =  TARGET2PERCENT(bass, MIN_BASS_VALUE, MAX_BASS_VALUE);
    volume = volume > 100 ? 100 : volume;
    volume = volume < 0 ? 0 : volume;

    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMicBass(index, volume);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_MicBass(index, volume);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_mic_bass(index, volume);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_mic_bass(index, bass);
}

void pbox_multi_echoMicReverb(uint8_t index, float reverb, display_t policy)
{
    int32_t volume =  TARGET2PERCENT(reverb, MIN_REVERB_VALUE, MAX_REVERB_VALUE);
    volume = volume > 100 ? 100 : volume;
    volume = volume < 0 ? 0 : volume;

    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMicReverb(index, volume);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_MicReverb(index, volume);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_mic_reverb(index, volume);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_mic_reverb(index, reverb);
}

void pbox_multi_echoAccompMusicLevel(uint32_t accomp_music_level, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayAccompMusicLevel(accomp_music_level);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_AccompMusicLevel(accomp_music_level);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_accompMusicLevel(accomp_music_level);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_accomp_music_level(accomp_music_level);
}

void pbox_multi_echoHumanMusicLevel(uint32_t human_music_level, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayHumanMusicLevel(human_music_level);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_HumanMusicLevel(human_music_level);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_humanMusicLevel(human_music_level);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_human_music_level(human_music_level);
}

void pbox_multi_echoReservLevel(uint32_t reserv_music_level, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayReservLevel(reserv_music_level);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_ReservLevel(reserv_music_level);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_reservLevel(reserv_music_level);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_reserv_music_level(reserv_music_level);
 }

void pbox_multi_echoVocalFadeoutSwitch(bool enable, uint32_t hlevel, uint32_t alevel, uint32_t rlevel, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayVocalFadeoutSwitch(enable, hlevel, alevel, rlevel);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_vocalFadeout_switch(enable, hlevel, alevel, rlevel);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_vocalFadeout_switch(enable);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_store_vocalFadeout_switch(enable);
}

void pbox_multi_echoStereoMode(stereo_mode_t stereo, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMusicStereoMode(stereo);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_MusicStereoMode(stereo);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_stereo_mode(stereo);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_stereo_mode(stereo);
}

void pbox_multi_echoPlacement(placement_t place, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMusicPlaceMode(place);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_MusicPlaceMode(place);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_placement(place);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_placement(place);
}

void pbox_multi_echoOutdoorMode(inout_door_t outdoor, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayMusicOutdoorMode(outdoor);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_MusicOutdoorMode(outdoor);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_inout_door(outdoor);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_outdoor_mode(outdoor);
}

void pbox_multi_echoEqMode(equalizer_t mode, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayEqMode(mode);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_MusicPlaceMode(mode);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_eq_mode(mode);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_eq_mode(mode);
}

void pbox_multi_echo_lightbar_volume(uint8_t level, display_t policy) {
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_lightbar_volume(level);
}

void pbox_multi_echo_lightbar_mode(uint8_t mode, display_t policy) {
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_lightbar_mode(mode);
}

void pbox_multi_echo_lightbar_power_onoff(uint8_t poweron, display_t policy) {
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_lightbar_power_onoff((bool)poweron);
}

void pbox_multi_echo_strobe_ctrl(uint8_t strobe, display_t policy) {
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_strobe_ctrl((bool)strobe);
}

void pbox_multi_echo_party_light_mode(uint8_t party, display_t policy) {
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_party_light_enable((bool)party);
}

void pbox_multi_echo_eq_bass_onoff(uint8_t bass, display_t policy) {
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_eq_bass_onoff(bass);
}

void pbox_multi_echoPoweron_status(bool status, display_t policy)
{
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_poweron(status);
}

void pbox_multi_echo_dsp_version(char* version, display_t policy)
{
    if(!version) return;

    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_dsp_version("v1.00");
}

void pbox_multi_echoInputSource(input_source_t inputSource, play_status_t status, display_t policy) {
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_inputSource(inputSource, status);
}

void pbox_multi_echoEcho3A(uint8_t index, bool enable, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayEcho3A(enable);

    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_music_echo3A(index, enable);
}

void pbox_multi_echoRevertMode(uint8_t index, pbox_revertb_t mode, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_displayRevertbMode(mode);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_revertMode(mode);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_echoRevert(index, mode);

    if (policy & STORAGE_DISP_MASK)
        pbox_app_music_store_recoder_revert(index, mode);
}

void pbox_multi_echoEnergyInfo(energy_info_t energy, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_dispplayEnergy(energy);

    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_energyInfo(energy);
}

void pbox_multi_echoUsbListupdate(uint32_t trackId, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_dispplayUsbListupdate(trackId);
}

void pbox_multi_echoUsbState(usb_state_t state, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_dispplayUsbState(state);

    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_usbState(state);
}

void pbox_multi_echobtState(btsink_state_t state, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_dispplaybtState(state);

    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_btState(state);
}

void pbox_multi_echoUacState(uac_role_t role, bool start, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
        pbox_app_lcd_dispplayUacState(start);
    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_uacState(start);
    if (policy & BTMCU_DISP_MASK)
        pbox_app_btsoc_echo_uacState(role, start);
}

void pbox_multi_echoUacFreq(uac_role_t role, uint32_t freq, display_t policy)
{
}

void pbox_multi_echoUacVolume(uac_role_t role, uint32_t volume, display_t policy)
{
    if (policy & LCD_DISPLAY_MASK)
    {
        if (role == UAC_ROLE_SPEAKER)
        {
            pbox_app_lcd_displayMusicVolumeLevel(volume);
        }
        else
        {
            pbox_app_lcd_displayMicVolumeLevel(volume);
        }
    }

    if (policy & LED_DISPLAY_MASK)
        pbox_app_led_uacVolume(volume);
}

void pbox_multi_echoUacMute(uac_role_t role, bool mute, display_t policy)
{
}

void pbox_multi_echoUacPpm(uac_role_t role, int32_t ppm, display_t policy)
{
}

#undef STORAGE_DISP_MASK
#undef BTMCU_DISP_MASK
#undef LCD_DISPLAY_MASK
#undef LED_DISPLAY_MASK