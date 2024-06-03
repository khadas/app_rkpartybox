#ifndef _PBOX_BT_SOC_APP_H_
#define _PBOX_BT_SOC_APP_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
void maintask_btsoc_fd_process(int fd);

void pbox_app_btsoc_echo_dsp_version(char *dspver);
void pbox_app_btsoc_echo_main_volume(float volume);
void pbox_app_btsoc_echo_music_volume(float level);
void pbox_app_btsoc_echo_placement(placement_t placement);
void pbox_app_btsoc_echo_inout_door(inout_door_t inout);
void pbox_app_btsoc_echo_eq_mode(equalizer_t mode);
void pbox_app_btsoc_echo_poweron(bool poweron);
void pbox_app_btsoc_echo_stereo_mode(stereo_mode_t mode);
void pbox_app_btsoc_echo_human_voice_fadeout(bool fadeout);
void pbox_app_btsoc_echo_input_source_with_playing_status(input_source_t source, play_status_t status);

#define pbox_app_btsoc_echo_uacState(...)           ((void) 0)
#define pbox_app_btsoc_echo_echoRevert(...)         ((void) 0)
#define pbox_app_btsoc_echo_music_echo3A(...)       ((void) 0)
#define pbox_app_btsoc_echo_inputSource(...)       	((void) 0)
#define pbox_app_btsoc_echo_vocalFadeout_switch(...)		((void) 0)
#define pbox_app_btsoc_echo_reservLevel(...)		((void) 0)
#define pbox_app_btsoc_echo_humanMusicLevel(...)	((void) 0)
#define pbox_app_btsoc_echo_accompMusicLevel(...)	((void) 0)
#define pbox_app_btsoc_echo_mic_reverb(...)			((void) 0)
#define pbox_app_btsoc_echo_mic_bass(...)			((void) 0)
#define pbox_app_btsoc_echo_mic_treble(...)			((void) 0)
#define pbox_app_btsoc_echo_micMux(...)				((void) 0)
#define pbox_app_btsoc_echo_mic_mute(...)			((void) 0)
#define pbox_app_btsoc_echo_mic_volume(...)			((void) 0)
#define pbox_app_btsoc_echo_PrevNext(...)			((void) 0)
#define pbox_app_btsoc_echo_playPause(...)			((void) 0)

#ifdef __cplusplus
}
#endif
#endif