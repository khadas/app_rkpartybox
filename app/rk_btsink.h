#ifndef _RK_BTSINK_H
#define _RK_BTSINK_H
#include <stdbool.h>
#include <RkBtSink.h>
#include "pbox_common.h"

#ifdef __cplusplus
extern "C" {
#endif
#define RKBTSINK_SERVER_SOCKET_PATH "/tmp/rockchip_btsink_server"
#define RKBTSINK_CLIENT_SOCKET_PATH "/tmp/rockchip_btsink_client"

enum _rk_cmd_msg_t{
	//command id
	RK_BT_NULL,
	RK_BT_PLAY = 1,
	RK_BT_PAUSE,
	RK_BT_STOP,
	RK_BT_NEXT,
	RK_BT_PREV,
	RK_BT_VOL_UP,
	RK_BT_VOL_DOWN,
	RK_BT_ABS_VOL,
	RK_BT_PAIRABLE,
    RK_BT_CONNECTABLE,
	RK_BT_ON,
    RK_BT_OFF,
    RK_BT_START_BLUEALSA,
    RK_BT_START_BLUEALSA_APLAY,

	//message id
	BT_SINK_STATE =0x100,
	BT_SINK_A2DP_STATE,
	BT_SINK_MUSIC_FORMAT,
	BT_SINK_MUSIC_TRACK,
	BT_SINK_MUSIC_POSITIONS,
    BT_SINK_ADPTER_INFO,
};
typedef int rk_cmd_msg_t;

#define BT_MSG_DISCOVERABLE 1


typedef enum {
	BT_NONE,
    BT_TURNING_TRUNNING_OFF,
	BT_ON = (BT_TURNING_TRUNNING_OFF+1),
	BT_DISCONNECT = (BT_TURNING_TRUNNING_OFF+1),
	BT_CONNECTING,
	BT_CONNECTED,
} btsink_state_t;

typedef enum {
	A2DP_NONE =0,
	A2DP_IDLE =1,
	A2DP_DISCONNECT =1,
	A2DP_CONNECTING,
	A2DP_CONNECTED,
	A2DP_STREAMING,

} btsink_ad2p_state_t;

enum {
	RK_BT_CMD = 1,
	RK_BT_EVT,
};
typedef int rk_bt_msg_t;

typedef struct {
	rk_bt_msg_t type;
	rk_cmd_msg_t msgId;
	union {
		struct {
			char addr[24];
			char data[24];
		} btcmd;

		struct {
			char addr[24];
			union {
				btsink_state_t state;
				btsink_ad2p_state_t a2dpState;
                struct {
                    char title[MAX_NAME_LENGTH + 1];
                    char artist[MAX_NAME_LENGTH + 1];
                }track;
                struct {
                    unsigned int current;
                    unsigned int total;
                }positions;
				pbox_audioFormat_t audioFormat;
			};
		} btinfo;
		struct {
			char addr[24];
            unsigned int adpter_id;
			union {
				unsigned int discoverable;
			};
		} adpter;
	};
} bt_msg_t;

int bt_sink_send_cmd(rk_cmd_msg_t command, char *data, int len);
int run_btsink_server(void);
bool isBtA2dpConnected(void);
bool isBtA2dpStreaming(void);
void *btsink_watcher(void *arg);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
