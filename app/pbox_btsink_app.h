#ifndef _PTBOX_ROCKIT_APP_H_
#define _PTBOX_ROCKIT_APP_H_
#include <stdint.h>
#include <stdbool.h>
#include "rk_btsink.h"
#include "pbox_common.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef rk_bt_msg_t     pbox_bt_msg_t;
typedef rk_bt_opcode_t  pbox_bt_opcode_t;

int getBtDiscoverable (void);

btsink_state_t getBtSinkState(void);

void setBtSinkState(btsink_state_t state);

bool isBtA2dpConnected(void);

bool isBtA2dpStreaming(void);

int bt_sink_send_cmd(rk_bt_opcode_t command, char *data, int len);

void maintask_bt_fd_process(int fd);

#ifdef __cplusplus
}
#endif
#endif