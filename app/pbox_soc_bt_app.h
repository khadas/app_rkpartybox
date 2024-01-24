#ifndef _PBOX_BT_SOC_APP_H_
#define _PBOX_BT_SOC_APP_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
void maintask_btsoc_fd_process(int fd);

void pbox_app_btsoc_reply_dsp_version(char *dspver);
void pbox_app_btsoc_reply_main_volume(uint32_t volume);
void pbox_app_btsoc_reply_placement(uint32_t placement);
void pbox_app_btsoc_reply_inout_door(inout_door_t inout);
void pbox_app_btsoc_reply_poweron(bool poweron);
void pbox_app_btsoc_reply_stereo_mode(stereo_mode_t mode);
void pbox_app_btsoc_reply_human_split(uint32_t level);
void pbox_app_btsoc_reply_input_source_with_playing_status(input_source_t source, play_status_t status);
void pbox_app_btsoc_reply_accom_level(uint32_t level);

#ifdef __cplusplus
}
#endif
#endif