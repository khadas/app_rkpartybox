/************* UI include two part:*************************
***************************1. LED effect show **************
***************************2. LCD screnn display ***********
* mainly here, almost are commands to display something,
* when key pressed, touch screem pressed, or bt info feedback,
* we may call api in this page to display something.
************************************************************/
#ifndef _PBOX_MULTI_ECHO_H_
#define _PBOX_MULTI_ECHO_H_
#include <stdint.h>
#include <stdbool.h>
#include "pbox_common.h"
#include "pbox_app.h"
#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
void pbox_multi_echoGender(gender_t gender, display_t policy);
void pbox_multi_echoIsPlaying(bool play, display_t policy);
void pbox_multi_echoPrevNext(bool next, display_t policy);
void pbox_multi_echoTrackInfo(const char* title, const char* artist, display_t policy);
void pbox_multi_echoTrackPosition(bool durationOnly, unsigned int mCurrent, unsigned int mDuration, display_t policy);
void pbox_multi_echoMusicVolumeLevel(float volume, display_t policy);
void pbox_multi_echoMainVolumeLevel(float mainVolume, display_t policy);
void pbox_multi_echoAccompMusicLevel(uint32_t accomp_music_level, display_t policy);
void pbox_multi_echoHumanMusicLevel(uint32_t human_music_level, display_t policy);
void pbox_multi_echoReservLevel(uint32_t reserv_music_level, display_t policy);
void pbox_multi_echoVocalFadeoutSwitch(bool enable, uint32_t hlevel, uint32_t alevel, uint32_t rlevel, display_t policy);
void pbox_multi_echoEnergyInfo(energy_info_t energy, display_t policy);

void pbox_multi_echoMicVolumeLevel(uint8_t index, float micVolume, display_t policy);
void pbox_multi_echoMicMute(uint8_t index, bool mute, display_t policy);
void pbox_multi_echoEcho3A(uint8_t index, bool enable, display_t policy);
void pbox_multi_echoRevertMode(uint8_t index, pbox_revertb_t mode, display_t policy);
void pbox_multi_echoMicMux(uint8_t index, mic_mux_t mux, display_t policy);
void pbox_multi_echoMicTreble(uint8_t index, float treble, display_t policy);
void pbox_multi_echoMicBass(uint8_t index, float bass, display_t policy);
void pbox_multi_echoMicReverb(uint8_t index, float reverb, display_t policy);

void pbox_multi_echoUsbState(usb_state_t state, display_t policy);
void pbox_multi_echoUsbListupdate(uint32_t trackId, display_t policy);

void pbox_multi_echobtState(btsink_state_t state, display_t policy);

void pbox_multi_echoUacState(uac_role_t role, bool start, display_t policy);
void pbox_multi_echoUacFreq(uac_role_t role, uint32_t freq, display_t policy);
void pbox_multi_echoUacVolume(uac_role_t role, uint32_t volume, display_t policy);
void pbox_multi_echoUacMute(uac_role_t role, bool mute, display_t policy);
void pbox_multi_echoUacPpm(uac_role_t role, int32_t ppm, display_t policy);

void pbox_multi_echoStereoMode(stereo_mode_t stereo, display_t policy);
void pbox_multi_echoPlacement(placement_t place, display_t policy);
void pbox_multi_echoOutdoorMode(inout_door_t outdoor, display_t policy);
void pbox_multi_echoEqMode(equalizer_t mode, display_t policy);

void pbox_multi_echo_lightbar_volume(uint8_t level, display_t policy);
void pbox_multi_echo_lightbar_mode(uint8_t mode, display_t policy);
void pbox_multi_echo_lightbar_power_onoff(uint8_t poweron, display_t policy);
void pbox_multi_echo_strobe_ctrl(uint8_t strobe, display_t policy);
void pbox_multi_echo_party_light_mode(uint8_t party, display_t policy);
void pbox_multi_echo_eq_bass_onoff(uint8_t bass, display_t policy);

void pbox_multi_echoPoweron_status(bool status, display_t policy);
void pbox_multi_echo_dsp_version(char* version, display_t policy);
void pbox_multi_echoInputSource(input_source_t inputSource, play_status_t status, display_t policy);

#ifdef __cplusplus
}
#endif
#endif  // _PBOX_MULTI_ECHO_H_
