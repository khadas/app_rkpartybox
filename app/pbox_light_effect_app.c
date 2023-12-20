#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pbox_common.h"
#include "pbox_light_effect.h"
#include "pbox_light_effect_app.h"

void pbox_app_led_PlayPause(bool play) {
	//printf("%s PlayPause: %d\n", __func__, play);
	//if (play)
	//	pbox_light_effect_send_cmd(RK_ECHO_PLAY_EVT, NULL, NULL);
	//else
	//	pbox_light_effect_send_cmd(RK_ECHO_PAUSE_EVT, NULL, NULL);
}

void pbox_app_led_PrevNext(bool next) {
	//printf("%s next: %d\n", __func__, next);
}

void pbox_app_led_TrackInfo(const char* title, const char* artist) {

}

void pbox_app_led_TrackPosition(uint32_t mCurrent, uint32_t mDuration) {

	//printf("%s mCurrent: %d mDuration: %d\n", __func__, mCurrent, mDuration);
}

void pbox_app_led_MainVolumeLevel(uint32_t mainVolume) {
	//printf("%s mainVolumeLevel: %d\n", __func__, mainVolume);
}

void pbox_app_led_MicVolumeLevel(uint32_t micVolume) {
	//printf("%s volume: %d\n", __func__, micVolume);
}

void pbox_app_led_AccompMusicLevel(uint32_t accomp_music_level) {
	//printf("%s accompMusicLevel: %d\n", __func__, accomp_music_level);
}

void pbox_app_led_HumanMusicLevel(uint32_t human_music_level) {
	//printf("%s humanMusicLevel: %d\n", __func__, human_music_level);
}

void pbox_app_led_ReservLevel(uint32_t Reserv_music_level) {
	//printf("%s ReservLevel: %d\n", __func__, Reserv_music_level);
}

void pbox_app_led_MusicSeparateSwitch(bool enable, uint32_t hlevel, uint32_t mlevel, uint32_t rlevel) {

}

void pbox_app_led_energyInfo(energy_info_t energy) {

	pbox_light_effect_send_cmd(PBOX_LIGHT_EFFECT_SOUNDREACTIVE_EVT, (void*)&energy, sizeof(energy));
}

void pbox_app_led_echo3A(bool enable) {

}

void pbox_app_led_revertMode(pbox_revertb_t mode) {

}
