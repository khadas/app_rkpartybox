#ifndef _PTBOX_BTSINK_APP_H_
#define _PTBOX_BTSINK_APP_H_

#include <stdint.h>
#include <stdbool.h>
#include "pbox_common.h"
#include "rk_btsink.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef rk_bt_msg_t     pbox_bt_msg_t;
typedef rk_bt_opcode_t  pbox_bt_opcode_t;

int getBtDiscoverable(void);
btsink_state_t getBtSinkState(void);
char *getBtRemoteName(void);
void setBtSinkState(btsink_state_t state);
bool isBtA2dpConnected(void);
bool isBtA2dpStreaming(void);
void pbox_btsink_playPause(bool play);
void pbox_btsink_music_next(bool next);
void pbox_btsink_volume_up(bool up);
void pbox_btsink_pair_enable(bool on);
void pbox_btsink_onoff(bool on);
void pbox_btsink_start_only_aplay(bool only);
void pbox_btsink_a2dp_stop(void);
void maintask_bt_fd_process(int fd);

#ifdef __cplusplus
}
#endif
#endif