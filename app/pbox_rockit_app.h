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

void pbox_app_rockit_set_datasource(char *path, char *headers);

void pbox_app_rockit_start_BTplayer(int sampleFreq, int channel,  const char *cardName);

void pbox_app_rockit_stop_BTplayer(void);

void pbox_app_rockit_start_player(void);

void pbox_app_rockit_pause_player(void);

void pbox_app_rockit_stop_player(void);

void pbox_app_rockit_resume_player(void);

void pbox_app_rockit_get_music_current_postion(void);

void pbox_app_rockit_get_player_duration(void);

void pbox_app_rockit_set_player_loop(bool loop);

void pbox_app_rockit_set_player_seek(uint32_t mPosition);

void pbox_app_rockit_set_player_volume(uint32_t volume);

void pbox_app_rockit_get_player_volume(void);

void pbox_app_rockit_set_player_seperate(bool enable , uint32_t hlevel, uint32_t mlevel, uint32_t rlevel);

void pbox_app_rockit_get_player_energy(void);

void pbox_app_rockit_start_recorder(void);

void pbox_app_rockit_stop_recorder(void);

void pbox_app_rockit_set_recoder_volume(uint32_t volume);

void pbox_app_rockit_get_recoder_volume(void);

void pbox_app_rockit_set_recoder_revert(pbox_revertb_t reverbMode);

void pbox_app_rockit_set_recoder_mute(bool mute);

void pbox_app_rockit_set_recoder_3A(uint32_t echo3A_On);

void pbox_app_rockit_set_uac_state(uac_role_t role, bool start);

void pbox_app_rockit_set_uac_freq(uac_role_t role, uint32_t freq);

void pbox_app_rockit_set_uac_volume(uac_role_t role, uint32_t volume);

void pbox_app_rockit_set_mute(uac_role_t role, bool mute);

void pbox_app_rockit_set_ppm(uac_role_t role, int32_t ppm);

bool maintask_rockit_fd_process(int fd);
#ifdef __cplusplus
}
#endif
#endif