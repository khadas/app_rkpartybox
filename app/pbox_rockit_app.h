#ifndef _PTBOX_ROCKIT_APP_H_
#define _PTBOX_ROCKIT_APP_H_

#include <stdint.h>
#include <stdbool.h>
#include "pbox_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void pbox_app_rockit_create(void);

void pbox_app_rockit_destroy(void);

//void pbox_app_rockit_set_datasource(char *path, char *headers);

void pbox_app_rockit_start_audiocard_player(input_source_t source, int sampleFreq, int channel, const char *cardName);

void pbox_app_rockit_start_local_player(char *path, char *headers);

void pbox_app_rockit_pause_player(input_source_t source);

void pbox_app_rockit_stop_player(input_source_t source);

void pbox_app_rockit_resume_player(input_source_t source);

void pbox_app_rockit_get_music_current_postion(input_source_t source);

void pbox_app_rockit_get_player_duration(input_source_t source);

void pbox_app_rockit_set_player_loop(input_source_t source, bool loop);

void pbox_app_rockit_set_player_seek(input_source_t source, uint32_t mPosition);

void pbox_app_rockit_set_player_volume(input_source_t source, float volume);

void pbox_app_rockit_get_player_volume(input_source_t source);

void pbox_app_rockit_set_music_volume(input_source_t source, float volume);

void pbox_app_rockit_get_player_volume(input_source_t source);

void pbox_app_rockit_set_player_seperate(input_source_t source, pbox_vocal_t vocal);

void pbox_app_rockit_set_stereo_mode(input_source_t source, stereo_mode_t mode);

void pbox_app_rockit_set_outdoor_mode(input_source_t source, inout_door_t mode);

void pbox_app_rockit_set_placement(input_source_t source, placement_t mode);

void pbox_app_rockit_set_eq_mode(input_source_t source, equalizer_t mode);

void pbox_app_rockit_get_player_energy(uint8_t destMux, input_source_t source, uint8_t micMux, uint8_t guitarMux);

void pbox_app_rockit_start_recorder(input_source_t source, int sampleFreq, int channel, const char *cardName);

void pbox_app_rockit_stop_recorder(input_source_t source);

void pbox_app_rockit_set_mic_data(uint8_t index, mic_set_kind_t kind, mic_state_t micState);

void pbox_app_rockit_get_recoder_volume(void);

void pbox_app_rockit_init_tunning(void);

void pbox_app_rockit_post_sence_detect(input_source_t source, size_t scenes);

void pbox_app_rockit_play_notice_number(uint32_t number);

void pbox_app_rockit_reset_gender(input_source_t source);

void pbox_app_rockit_start_inout_detect(void);
//left, right
void pbox_app_rockit_start_doa_detect(int agent_role);

void pbox_app_rockit_start_doa_detect(int agent_role);

void pbox_app_rockit_stop_env_detect(void);

void pbox_app_rockit_set_uac_state(uac_role_t role, bool start);

void pbox_app_rockit_set_uac_freq(uac_role_t role, uint32_t freq);

void pbox_app_rockit_set_uac_volume(uac_role_t role, float volume);

void pbox_app_rockit_set_mute(uac_role_t role, bool mute);

void pbox_app_rockit_set_ppm(uac_role_t role, int32_t ppm);

bool maintask_rockit_fd_process(int fd);
#ifdef __cplusplus
}
#endif
#endif