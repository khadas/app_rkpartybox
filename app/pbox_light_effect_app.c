#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "pbox_common.h"
#include "pbox_light_effect.h"
#include "pbox_light_effect_app.h"
#include "pbox_app.h"

void pbox_app_led_displayGender(gender_t gender) {
	ALOGD("%s: %d\n", __func__, gender);
}

void pbox_app_led_PlayPause(bool play) {
	ALOGD("%s PlayPause: %d\n", __func__, play);
	//soundreactive_mute_set(!play);// it shouldn't call another thread's func here directly...
	if (play) {
		pbox_light_effect_send_cmd(RK_ECHO_PLAY_EVT, NULL, sizeof(NULL));
	} else {
		pbox_light_effect_send_cmd(RK_ECHO_PAUSE_EVT, NULL, sizeof(NULL));
	}
}

void pbox_app_led_PrevNext(bool next) {
	ALOGD("%s next: %d\n", __func__, next);

	//soundreactive_mute_set(true);// it shouldn't call another thread's func here directly...
	if (next) {
		pbox_light_effect_send_cmd(RK_ECHO_PLAY_NEXT_EVT, NULL, sizeof(NULL));
	} else {
		pbox_light_effect_send_cmd(RK_ECHO_PLAY_PREV_EVT, NULL, sizeof(NULL));
	}
}

void pbox_app_led_MusicVolumeLevel(uint32_t mVolume) {
	ALOGD("%s mainVolumeLevel: %d\n", __func__, mVolume);

	pbox_light_effect_send_cmd(RK_ECHO_VOLUME_LED_EVT, (void*)&mVolume, sizeof(uint32_t));
}

void pbox_app_led_MicVolumeLevel(uint32_t micVolume) {
	ALOGD("%s volume: %d\n", __func__, micVolume);
	pbox_light_effect_send_cmd(RK_ECHO_MIC_MUTE_EVT, (void*)&micVolume, sizeof(uint32_t));

}

void pbox_app_led_AccompMusicLevel(uint32_t accomp_music_level) {
	ALOGD("%s accompMusicLevel: %d\n", __func__, accomp_music_level);
}

void pbox_app_led_HumanMusicLevel(uint32_t human_music_level) {
	ALOGD("%s humanMusicLevel: %d\n", __func__, human_music_level);
}

void pbox_app_led_ReservLevel(uint32_t Reserv_music_level) {
	ALOGD("%s ReservLevel: %d\n", __func__, Reserv_music_level);
}

void pbox_app_led_energyInfo(energy_info_t energy) {
	pbox_light_effect_send_cmd(PBOX_LIGHT_EFFECT_SOUNDREACTIVE_EVT, (void*)&energy, sizeof(energy));
}

void pbox_app_led_btState(btsink_state_t mode) {

	ALOGD("%s btsink_state_t: %d\n", __func__, mode);
	switch (mode)
	{
		case APP_BT_DISCONNECT:
			pbox_light_effect_send_cmd(RK_ECHO_BT_PAIR_FAIL_EVT, NULL, sizeof(NULL));
			break;
		case APP_BT_CONNECTING:
			pbox_light_effect_send_cmd(RK_ECHO_BT_PAIRING_EVT, NULL, sizeof(NULL));
			break;
		case APP_BT_CONNECTED:
			pbox_light_effect_send_cmd(RK_ECHO_BT_PAIR_SUCCESS_EVT, NULL, sizeof(NULL));
			break;
	}
}

void pbox_app_led_startup_effect(void) {
	pbox_light_effect_send_cmd(RK_ECHO_LED_OFF_EVT, NULL, sizeof(NULL));
	pbox_light_effect_send_cmd(RK_ECHO_SYSTEM_BOOTING_EVT, NULL, sizeof(NULL));
}
