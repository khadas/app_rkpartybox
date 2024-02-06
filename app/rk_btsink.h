#ifndef _RK_BTSINK_H_
#define _RK_BTSINK_H_
#include <stdbool.h>
#include <RkBtSink.h>
#include "pbox_common.h"
#ifdef __cplusplus
extern "C" {
#endif

#define BT_SINK_ADPTER_DISCOVERABLE 1
#define BT_SINK_ADPTER_ADDRESS		2
#define BT_SINK_ADPTER_NAME 		3
/**
 * Convenient macro for getting "on the stack" array size. */
#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))

typedef enum _rk_cmd_msg_t{
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
	RK_BT_START_BLUEALSA_ONLY,
	RK_BT_START_BLUETOOTH,
	RK_BT_LOCAL_UPDATE,

	//message id
	BT_SINK_STATE =0x100,
	BT_SINK_NAME,
	BT_SINK_A2DP_STATE,
	BT_SINK_MUSIC_FORMAT,
	BT_SINK_MUSIC_TRACK,
	BT_SINK_MUSIC_POSITIONS,
    BT_SINK_ADPTER_INFO,
	BT_SINK_VENDOR_EVT,
} rk_bt_opcode_t;

typedef enum {
	BT_NONE,
    BT_TURNING_TRUNNING_OFF,
	BT_INIT_ON,
	BT_DISCONNECT,
	BT_IDLE = BT_DISCONNECT,
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

typedef enum {
	RK_BT_CMD = 1,
	RK_BT_EVT,
} bt_msg_t;

typedef struct {
	int sampingFreq;
	int channel;
} bt_audio_format_t;

typedef struct {
	bt_msg_t type;
	rk_bt_opcode_t msgId;
	union {
		struct {
			char addr[24];
			char data[24];
		} btcmd;
		struct {
			char addr[24];
			char remote_name[MAX_NAME_LENGTH + 1];
			union {
				btsink_state_t state;
				btsink_ad2p_state_t a2dpState;
                struct {
                    char title[MAX_NAME_LENGTH + 1];
                    char artist[MAX_NAME_LENGTH + 1];
                } track;
                struct {
                    uint32_t current;
                    uint32_t total;
                }positions;
				bt_audio_format_t audioFormat;
				struct {
					unsigned int adpter_id;
					union {
						unsigned int discoverable;
					};
				} adpter;
				bool enable;
			};
		} btinfo;
	};
	int media_volume;
} rk_bt_msg_t;

bool isBtA2dpConnected(void);
bool isBtA2dpStreaming(void);
void *btsink_watcher(void *arg);
int pbox_create_bttask(void);
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
