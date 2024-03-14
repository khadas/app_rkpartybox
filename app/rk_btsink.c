#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <RkBtBase.h>
#include <RkBtSink.h>
#include "pbox_common.h"
#include "rk_btsink.h"
#include "rk_utils.h"
#include "pbox_socket.h"
#include "pbox_socketpair.h"

#define PRINT_FLAG_ERR "[RK_BT_ERROR]"
#define PRINT_FLAG_SUCESS "[RK_BT_SUCESS]"

int unix_socket_bt_notify_msg(void *info, int length)
{
	unix_socket_notify_msg(PBOX_MAIN_BT, info, length);
}

int bt_sink_notify_vendor_state(bool enable)
{
	rk_bt_msg_t msg = {0};
	msg.type = RK_BT_EVT;
	msg.msgId = BT_SINK_VENDOR_EVT;
	msg.btinfo.enable = enable;

	unix_socket_bt_notify_msg(&msg, sizeof(rk_bt_msg_t));
}

int bt_sink_notify_btstate(btsink_state_t state, char *name)
{
	rk_bt_msg_t msg = {0};
	msg.type = RK_BT_EVT;
	msg.msgId = BT_SINK_STATE;
	msg.btinfo.state = state;
	if(name)
		strncpy(msg.btinfo.remote_name, name, MAX_NAME_LENGTH);
	msg.btinfo.remote_name[MAX_NAME_LENGTH] = 0;

	unix_socket_bt_notify_msg(&msg, sizeof(rk_bt_msg_t));
}

int bt_sink_notify_btname(char *name)
{
	rk_bt_msg_t msg = {0};
	msg.type = RK_BT_EVT;
	msg.msgId = BT_SINK_NAME;
	if(name)
		strncpy(msg.btinfo.remote_name, name, MAX_NAME_LENGTH);
	msg.btinfo.remote_name[MAX_NAME_LENGTH] = 0;

	unix_socket_bt_notify_msg(&msg, sizeof(rk_bt_msg_t));
}

int bt_sink_notify_a2dpstate(btsink_ad2p_state_t state)
{
	rk_bt_msg_t msg = {0};
	msg.type = RK_BT_EVT;
	msg.msgId = BT_SINK_A2DP_STATE;
	msg.btinfo.a2dpState = state;

	unix_socket_bt_notify_msg(&msg, sizeof(rk_bt_msg_t));
}

int bt_sink_notify_avrcp_track(char *tile, char *artist)
{
	rk_bt_msg_t msg = {0};
	msg.type = RK_BT_EVT;
	msg.msgId = BT_SINK_MUSIC_TRACK;
	msg.btinfo.track.title[0] = ' ';
	msg.btinfo.track.artist[0] = ' ';
	if(tile[0] != 0) {
		memset(&msg.btinfo.track.title[0], 0, MAX_NAME_LENGTH);
		strncpy(&msg.btinfo.track.title[0], tile, MIN(strlen(tile), MAX_NAME_LENGTH));
		msg.btinfo.track.title[MAX_NAME_LENGTH] = 0;
	}
	if(artist[0] != 0) {
		memset(&msg.btinfo.track.artist[0], 0, MAX_NAME_LENGTH);
		strncpy(&msg.btinfo.track.artist[0], artist, MIN(strlen(artist), MAX_NAME_LENGTH));
		msg.btinfo.track.artist[MAX_NAME_LENGTH] = 0;
	}
	//ALOGD("%s recv msg rack: %s[%p] %s[%p]\n", __func__, msg.btinfo.track.title, msg.btinfo.track.title, msg.btinfo.track.artist, msg.btinfo.track.artist);

	unix_socket_bt_notify_msg(&msg, sizeof(rk_bt_msg_t));
}

int bt_sink_notify_adapter_discoverable(bool discoverable) {
    rk_bt_msg_t msg = {0};
    msg.type = RK_BT_EVT;
    msg.msgId = BT_SINK_ADPTER_INFO;
	msg.btinfo.adpter.adpter_id = BT_SINK_ADPTER_DISCOVERABLE;
     msg.btinfo.adpter.discoverable= discoverable;

    unix_socket_bt_notify_msg(&msg, sizeof(rk_bt_msg_t));
}


int bt_sink_notify_avrcp_position(uint32_t posistion, uint32_t total)
{
    rk_bt_msg_t msg = {0};
    msg.type = RK_BT_EVT;
    msg.msgId = BT_SINK_MUSIC_POSITIONS;
    msg.btinfo.positions.current = posistion;
    msg.btinfo.positions.total = total;

    unix_socket_bt_notify_msg(&msg, sizeof(rk_bt_msg_t));
}

int bt_sink_notify_pcm_format(int sampleFreq, int channel)
{
	rk_bt_msg_t msg = {0};
	msg.type = RK_BT_EVT;
	msg.msgId = BT_SINK_MUSIC_FORMAT;
	msg.btinfo.audioFormat.sampingFreq = sampleFreq;
    msg.btinfo.audioFormat.channel = channel;
    ALOGD("FUNC:%s sampleFreq:%d, channel=%d!\n", __func__, sampleFreq, channel);
	unix_socket_bt_notify_msg(&msg, sizeof(rk_bt_msg_t));
}

static unsigned int a2dp_codec_lookup_frequency(uint16_t capability_value) {
	#define SBC_SAMPLING_FREQ_16000         (1 << 3)
	#define SBC_SAMPLING_FREQ_32000         (1 << 2)
	#define SBC_SAMPLING_FREQ_44100         (1 << 1)
	#define SBC_SAMPLING_FREQ_48000         (1 << 0)
	struct a2dp_sampling_freq {
		unsigned int frequency;
		uint16_t value;
	} static const a2dp_sbc_samplings[] = {
		{ 16000, SBC_SAMPLING_FREQ_16000 },
		{ 32000, SBC_SAMPLING_FREQ_32000 },
		{ 44100, SBC_SAMPLING_FREQ_44100 },
		{ 48000, SBC_SAMPLING_FREQ_48000 },
	};

	for (int i = 0; i < ARRAYSIZE(a2dp_sbc_samplings); i++)
		if (capability_value == a2dp_sbc_samplings[i].value)
			return a2dp_sbc_samplings[i].frequency;

	return 0;
}

static unsigned int a2dp_codec_lookup_channels(uint16_t capability_value) {
	#define SBC_CHANNEL_MODE_MONO           (1 << 3)
	#define SBC_CHANNEL_MODE_DUAL_CHANNEL   (1 << 2)
	#define SBC_CHANNEL_MODE_STEREO         (1 << 1)
	#define SBC_CHANNEL_MODE_JOINT_STEREO   (1 << 0)

	struct a2dp_channel_mode {
		unsigned int channels;
		uint16_t value;
	} static const a2dp_sbc_channels[] = {
		{ 1, SBC_CHANNEL_MODE_MONO },
		{ 2, SBC_CHANNEL_MODE_DUAL_CHANNEL },
		{ 2, SBC_CHANNEL_MODE_STEREO },
		{ 2, SBC_CHANNEL_MODE_JOINT_STEREO },
	};

	for (int i = 0; i < ARRAYSIZE(a2dp_sbc_channels); i++)
			if (capability_value == a2dp_sbc_channels[i].value)
				return a2dp_sbc_channels[i].channels;

	return 0;
}

static void bt_sink_notify_volume(uint32_t volume)
{
	rk_bt_msg_t msg = {0};
	msg.type = RK_BT_EVT;
	msg.msgId = RK_BT_ABS_VOL;
	msg.media_volume = volume;

	unix_socket_bt_notify_msg(&msg, sizeof(rk_bt_msg_t));
}

static RkBtContent bt_content;
static void bt_test_state_cb(RkBtRemoteDev *rdev, RK_BT_STATE state)
{
	ALOGD("%s state:%d\n", __func__, state);
	switch (state) {
	//BASE STATE
	case RK_BT_STATE_TURNING_ON:
		ALOGI("++ RK_BT_STATE_TURNING_ON\n");
		break;
	case RK_BT_STATE_INIT_ON:
		ALOGI("++ RK_BT_STATE_INIT_ON=%d\n", rk_bt_is_powered_on());
		bt_content.init = true;
		//rk_bt_set_power(true);
		if(rk_bt_is_powered_on()) {
			bt_content.power = true;
			bt_sink_notify_btstate(BT_INIT_ON, NULL);
		}
		break;
	case RK_BT_STATE_INIT_OFF:
		ALOGI("++ RK_BT_STATE_INIT_OFF\n");
		bt_content.init = false;
		bt_sink_notify_btstate(BT_NONE, NULL);
		break;

	//SCAN STATE
	case RK_BT_STATE_SCAN_NEW_REMOTE_DEV:
		if (rdev->paired)
			ALOGI("+ PAIRED_DEV: [%s|%d]:%s:%s\n", rdev->remote_address, rdev->rssi,
					rdev->remote_address_type, rdev->remote_alias);
		else
			ALOGI("+ SCAN_NEW_DEV: [%s|%d]:%s:%s\n", rdev->remote_address, rdev->connected,
					rdev->remote_address_type, rdev->remote_alias);
		break;
	case RK_BT_STATE_SCAN_CHG_REMOTE_DEV:
		ALOGI("+ SCAN_CHG_DEV: [%s|%d]:%s:%s|%s\n", rdev->remote_address, rdev->rssi,
				rdev->remote_address_type, rdev->remote_alias, rdev->change_name);
		if (!strcmp(rdev->change_name, "UUIDs")) {
			for (int index = 0; index < 36; index++) {
				if (!strcmp(rdev->remote_uuids[index], "NULL"))
					break;
				ALOGI("\tUUIDs: %s\n", rdev->remote_uuids[index]);
			}
		} else if (!strcmp(rdev->change_name, "Icon")) {
			ALOGI("\tIcon: %s\n", rdev->icon);
		} else if (!strcmp(rdev->change_name, "Class")) {
			ALOGI("\tClass: 0x%x\n", rdev->cod);
		} else if (!strcmp(rdev->change_name, "Modalias")) {
			ALOGI("\tModalias: %s\n", rdev->modalias);
		}
		break;
	case RK_BT_STATE_SCAN_DEL_REMOTE_DEV:
		ALOGI("+ SCAN_DEL_DEV: [%s]:%s:%s\n", rdev->remote_address,
				rdev->remote_address_type, rdev->remote_alias);
		break;

	//LINK STATE
	case RK_BT_STATE_CONNECTED:
	case RK_BT_STATE_DISCONN:
		ALOGI("+ %s [%s|%d]:%s:%s\n", rdev->connected ? "STATE_CONNECT" : "STATE_DISCONN",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		//bt_sink_notify_btname(rdev->remote_alias);
		bt_sink_notify_btstate((state == RK_BT_STATE_CONNECTED) ? BT_CONNECTED:BT_DISCONNECT, rdev->remote_alias);
		break;
	case RK_BT_STATE_PAIRED:
	case RK_BT_STATE_PAIR_NONE:
		ALOGI("+ %s [%s|%d]:%s:%s\n", rdev->paired ? "STATE_PAIRED" : "STATE_PAIR_NONE",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		break;
	case RK_BT_STATE_BONDED:
	case RK_BT_STATE_BOND_NONE:
		ALOGI("+ %s [%s|%d]:%s:%s\n", rdev->bonded ? "STATE_BONDED" : "STATE_BOND_NONE",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		break;
	case RK_BT_STATE_BOND_FAILED:
	case RK_BT_STATE_PAIR_FAILED:
	case RK_BT_STATE_DISCONN_FAILED:
	case RK_BT_STATE_CONNECT_FAILED:
	case RK_BT_STATE_DEL_DEV_FAILED:
		ALOGI("+ STATE_FAILED [%s|%d]:%s:%s reason: %s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias,
				rdev->change_name);
		break;

	//MEDIA A2DP SOURCE
	case RK_BT_STATE_SRC_ADD:
	case RK_BT_STATE_SRC_DEL:
		ALOGI("+ STATE SRC MEDIA %s [%s|%d]:%s:%s\n",
				(state == RK_BT_STATE_SRC_ADD) ? "ADD" : "DEL",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		ALOGI("+ codec: %s, freq: %s, chn: %s\n",
					rdev->media.codec == 0 ? "SBC" : "UNKNOW",
					rdev->media.sbc.frequency == 1 ? "48K" : "44.1K",
					rdev->media.sbc.channel_mode == 1 ? "JOINT_STEREO" : "STEREO");
		break;

	//MEDIA AVDTP TRANSPORT
	case RK_BT_STATE_TRANSPORT_VOLUME:
		ALOGI("+ STATE AVDTP TRASNPORT VOLUME[%d] [%s|%d]:%s:%s\n",
				rdev->media.volume,
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		bt_sink_notify_volume(rdev->media.volume);
		break;
	case RK_BT_STATE_TRANSPORT_IDLE:
		ALOGI("+ STATE AVDTP TRASNPORT IDLE [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		break;
	case RK_BT_STATE_TRANSPORT_PENDING:
		ALOGI("+ STATE AVDTP TRASNPORT PENDING [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		break;
	case RK_BT_STATE_TRANSPORT_ACTIVE:
		ALOGI("+ STATE AVDTP TRASNPORT ACTIVE [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		bt_sink_notify_a2dpstate(A2DP_STREAMING);
		break;
	case RK_BT_STATE_TRANSPORT_SUSPENDING:
		ALOGI("+ STATE AVDTP TRASNPORT SUSPEND [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		break;

	//MEDIA A2DP SINK
	case RK_BT_STATE_SINK_ADD:
	case RK_BT_STATE_SINK_DEL:
		unsigned int freq = a2dp_codec_lookup_frequency(rdev->media.sbc.frequency);
		unsigned int channel = a2dp_codec_lookup_channels(rdev->media.sbc.channel_mode);
		ALOGI("+ STATE SINK MEDIA %s [%s|%d]:%s:%s\n",
				(state == RK_BT_STATE_SINK_ADD) ? "ADD" : "DEL",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		ALOGI("+ codec: %s, freq: %d, chn: %d\n",
					rdev->media.codec == 0 ? "SBC" : "UNKNOW",
					freq,
					channel);
		if(state == RK_BT_STATE_SINK_ADD) {
			bt_sink_notify_btstate(BT_CONNECTED, rdev->remote_alias);
			bt_sink_notify_pcm_format(freq, channel);
			bt_sink_notify_a2dpstate(A2DP_CONNECTED);
		} else {
			bt_sink_notify_a2dpstate(A2DP_IDLE);
		}
		break;
	case RK_BT_STATE_SINK_PLAY:
		ALOGI("+ STATE SINK PLAYER PLAYING [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		bt_sink_notify_a2dpstate(A2DP_STREAMING);
		break;
	case RK_BT_STATE_SINK_STOP:
		ALOGI("+ STATE SINK PLAYER STOP [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		bt_sink_notify_a2dpstate(A2DP_CONNECTED);
		break;
	case RK_BT_STATE_SINK_PAUSE:
		ALOGI("+ STATE SINK PLAYER PAUSE [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);

		bt_sink_notify_a2dpstate(A2DP_CONNECTED);
		break;
    case RK_BT_STATE_SINK_TRACK:
        ALOGI("+ STATE SINK TRACK INFO [%s|%d]:%s:%s track[%s]-[%s]\n",
            rdev->remote_address,
            rdev->rssi,
            rdev->remote_address_type,
            rdev->remote_alias,
            rdev->title,
            rdev->artist);
        bt_sink_notify_avrcp_track(rdev->title, rdev->artist);
    break;
    case RK_BT_STATE_SINK_POSITION:
        ALOGI("+ STATE SINK TRACK POSITION:[%s|%d]:%s:%s [%u-%u]\n",
                rdev->remote_address,
                rdev->rssi,
                rdev->remote_address_type,
                rdev->remote_alias,
                rdev->player_position,
                rdev->player_total_len);
        bt_sink_notify_avrcp_position(rdev->player_position, rdev->player_total_len);
    break;

	//ADV
	case RK_BT_STATE_ADAPTER_BLE_ADV_START:
		bt_content.ble_content.ble_advertised = true;
		ALOGI("RK_BT_STATE_ADAPTER_BLE_ADV_START successful\n");
		break;
	case RK_BT_STATE_ADAPTER_BLE_ADV_STOP:
		bt_content.ble_content.ble_advertised = false;
		ALOGI("RK_BT_STATE_ADAPTER_BLE_ADV_STOP successful\n");
		break;

	//ADAPTER STATE
	case RK_BT_STATE_ADAPTER_NO_DISCOVERYABLED:
		bt_content.discoverable = false;
        bt_sink_notify_adapter_discoverable(false);
		ALOGI("RK_BT_STATE_ADAPTER_NO_DISCOVERYABLED successful\n");
		break;
	case RK_BT_STATE_ADAPTER_DISCOVERYABLED:
		bt_content.discoverable = true;
        bt_sink_notify_adapter_discoverable(true);
		ALOGI("RK_BT_STATE_ADAPTER_DISCOVERYABLED successful\n");
		break;
	case RK_BT_STATE_ADAPTER_NO_PAIRABLED:
		bt_content.pairable = false;
		ALOGI("RK_BT_STATE_ADAPTER_NO_PAIRABLED successful\n");
		break;
	case RK_BT_STATE_ADAPTER_PAIRABLED:
		bt_content.pairable = true;
		ALOGI("RK_BT_STATE_ADAPTER_PAIRABLED successful\n");
		break;
	case RK_BT_STATE_ADAPTER_NO_SCANNING:
		bt_content.scanning = false;
		ALOGI("RK_BT_STATE_ADAPTER_NO_SCANNING successful\n");
		break;
	case RK_BT_STATE_ADAPTER_SCANNING:
		bt_content.scanning = true;
		ALOGI("RK_BT_STATE_ADAPTER_SCANNING successful\n");
		break;
	case RK_BT_STATE_ADAPTER_POWER_ON:
		ALOGI("RK_BT_STATE_ADAPTER_POWER_ON successful power=%d\n", bt_content.power);
		if(!bt_content.power) {
			bt_content.power = true;
			bt_sink_notify_btstate(BT_INIT_ON, NULL);
		} break;
	case RK_BT_STATE_ADAPTER_POWER_OFF:
		bt_content.power = false;
		ALOGI("RK_BT_STATE_ADAPTER_POWER_OFF successful\n");
		break;
	case RK_BT_STATE_COMMAND_RESP_ERR:
		ALOGI("RK_BT_STATE CMD ERR!!!\n");
		break;
	case RK_BT_STATE_DEL_DEV_OK:
		if (rdev != NULL)
			ALOGI("+ RK_BT_STATE_DEL_DEV_OK: %s:%s:%s\n",
				rdev->remote_address,
				rdev->remote_address_type,
				rdev->remote_alias);
		break;
	default:
		if (rdev != NULL)
			ALOGI("+ DEFAULT STATE %d: %s:%s:%s RSSI: %d [CBP: %d:%d:%d]\n", state,
				rdev->remote_address,
				rdev->remote_address_type,
				rdev->remote_alias,
				rdev->rssi,
				rdev->connected,
				rdev->paired,
				rdev->bonded);
		break;
	}
}

void bt_test_version(char *data)
{
	int day, year;
	char month[4];
	const char *dateString = __DATE__;
	sscanf(dateString, "%s %d %d", month, &day, &year);
	ALOGW("RK BT VERSION: %s-[%d-%s-%d:%s]\n", rk_bt_version(), year, month, day, __TIME__);
}

static bool bt_test_audio_server_cb(void)
{
	ALOGD("%s\n", __func__);
	return true;
}

static char bt_name[32] = {0};
static void btsink_config_name(void) {
	unsigned int addr[6] = {0};

	exec_command("hciconfig hci0 | awk '/BD Address:/ {print $3}'", bt_name, sizeof(bt_name));
	if(bt_name[0]) {
		if (sscanf(bt_name, "%2X:%2X:%2X:%2X:%2X:%2X",
				&addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]) != 6) {
			fprintf(stderr, "Failed to parse Bluetooth address.\n");
			return (void*)-1;
		}
	}

	ALOGD("%s thread: %lu\n", __func__, (unsigned long)pthread_self());
	memset(&bt_content, 0, sizeof(RkBtContent));

	//BREDR CLASS BT NAME
	if(bt_name[0]) {
		sprintf(bt_name, "rk-partybox-%02X", addr[5]);
		bt_content.bt_name = bt_name;
	}
	else {
		bt_content.bt_name = "partybox";
	}
	ALOGW("%s name:%s\n", __func__, bt_content.bt_name);
}

static bool bt_test_vendor_cb(bool enable)
{
	ALOGD("%s enable:%d\n", __func__, enable);
	bt_sink_notify_vendor_state(enable);

	return true;
}

static void bt_restart_bluealsa_only(void) {
	ALOGD("%s\n", __func__);
	kill_task("pulseaudio");
	if(!get_ps_pid("bluealsa")) {
		run_task("bluealsa", "bluealsa -S --profile=a2dp-sink &");
		rk_setRtPrority(get_ps_pid("bluealsa"), SCHED_RR, 9);
	}
}

static int bt_restart_a2dp_sink(bool onlyAplay)
{
	char ret_buff[1024];

	ALOGD("%s onlyAplay:%d\n", __func__, onlyAplay);
	kill_task("pulseaudio");

	if(!onlyAplay) {
		bool btsnoop = false;
        if (!access("/userdata/cfg/rkwifibt_stack.conf", F_OK)) {
                exec_command("cat /userdata/cfg/rkwifibt_stack.conf | grep BtSnoopLogOutput", ret_buff, 1024);
                if(ret_buff[0]&&strstr(ret_buff, "BtSnoopLogOutput:true")) {
                    btsnoop = true;
                }
        }
		if(btsnoop) {
			kill_task("hcidump");
			exec_command_system("hcidump -i hci0 -w /data/btsnoop.log &");
		}

		if(!get_ps_pid("bluealsa")) {
			run_task("bluealsa", "bluealsa -S --profile=a2dp-sink &");
			rk_setRtPrority(get_ps_pid("bluealsa"), SCHED_RR, 9);
		}
	}

	if(!get_ps_pid("bluealsa-aplay")) {
		#if ENABLE_RK_ROCKIT
		run_task("bluealsa-aplay", "bluealsa-aplay -S --profile-a2dp --pcm=plughw:7,0,0 00:00:00:00:00:00 &");
		//run_task("bluealsa-aplay", "bluealsa-aplay -S --profile-a2dp --pcm-buffer-time 800000 --pcm=plughw:7,0,0 00:00:00:00:00:00 &");
		#else
		run_task("bluealsa-aplay", "bluealsa-aplay -S --profile-a2dp --pcm=plughw:0,0 00:00:00:00:00:00 &");
		#endif
		rk_setRtPrority(get_ps_pid("bluealsa-aplay"), SCHED_RR, 9);
	}
	return 0;
}

static void bt_vendor_set_enable(bool enable) {
	int times = 100;
	ALOGD("%s enable:%d\n", __func__, enable);
}

static void *btsink_server(void *arg)
{
	char buff[sizeof(rk_bt_msg_t)] = {0};
	int sockfd = get_server_socketpair_fd(PBOX_SOCKPAIR_BT);

	pthread_setname_np(pthread_self(), "pbox_btserver");
    ALOGW("%s thread: %lu\n", __func__, (unsigned long)pthread_self());
	memset(&bt_content, 0, sizeof(RkBtContent));
	btsink_config_name();
	//BLE NAME
	bt_content.ble_content.ble_name = "RBLE";
	//IO CAPABILITY
	bt_content.io_capability = IO_CAPABILITY_DISPLAYYESNO;
	/*
	 * Only one can be enabled
	 * a2dp sink and hfp-hf
	 * a2dp source and hfp-ag
	 */
	// enable ble
	bt_content.profile = PROFILE_A2DP_SINK_HF;
	bt_content.bluealsa = true;

	rk_bt_register_state_callback(bt_test_state_cb);
	rk_bt_register_vendor_callback(bt_test_vendor_cb);
	rk_bt_register_audio_server_callback(bt_test_audio_server_cb);

	//default state
	bt_content.init = false;
	rk_bt_init(&bt_content);

	while(true) {
		memset(buff, 0, sizeof(buff));
		int ret = recv(sockfd, buff, sizeof(buff), 0);
		if (ret <= 0)
			continue;

		rk_bt_msg_t *msg = (rk_bt_msg_t *)buff;
		ALOGD("%s sock recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

		if (msg->type == RK_BT_CMD)
		{
			switch (msg->msgId) {
    			case RK_BT_PLAY:
    				rk_bt_sink_media_control("play");
    				break;
    			case RK_BT_PAUSE:
    				rk_bt_sink_media_control("pause");
    				break;
    			case RK_BT_STOP:
    				rk_bt_sink_media_control("stop");
    				break;
    			case RK_BT_NEXT:
    				rk_bt_sink_media_control("next");
    				break;
    			case RK_BT_PREV:
    				rk_bt_sink_media_control("previous");
    				break;
				case RK_BT_PAIRABLE: {
					rk_bt_set_discoverable(true);
				} break;
				case RK_BT_LOCAL_UPDATE: {
					char ret_buff[64];
					btsink_config_name();
					sprintf(ret_buff, "hciconfig hci0 name %s", bt_content.bt_name);
					exec_command_system(ret_buff);
					exec_command_system("hciconfig hci0 class 0x240404");
                } break;
                case RK_BT_CONNECTABLE: {
    				rk_bt_set_pairable(true);
                } break;
    			case RK_BT_ON:{
                    rk_bt_register_state_callback(bt_test_state_cb);
    				rk_bt_init(&bt_content);
                } break;
    			case RK_BT_OFF:{
    				rk_bt_deinit();
                } break;
    			case RK_BT_START_BLUEALSA:{
    				bt_restart_a2dp_sink(false);
                } break;
    			case RK_BT_START_BLUEALSA_APLAY:{
    				bt_restart_a2dp_sink(true);
                } break;
				case RK_BT_START_BLUEALSA_ONLY: {
					bt_restart_bluealsa_only();
				} break;
				case RK_BT_START_BLUETOOTH: {
					bool enable = msg->btinfo.enable;
					bt_vendor_set_enable(enable);
				}
				case RK_BT_ABS_VOL:{
					rk_bt_sink_set_volume(msg->media_volume);
                } break;
			}
		}
	}

    close(sockfd);
fail:
    return (void*)0;
}

int pbox_create_bttask(void)
{
    pthread_t tid_server, tid_watch;
    int ret;

    ret = pthread_create(&tid_server, NULL, btsink_server, NULL);
    if (ret < 0)
    {
        ALOGE("btsink server start failed\n");
    }

    //ret = pthread_create(&tid_watch, NULL, btsink_watcher, NULL);
    if (ret < 0)
    {
        ALOGE("btsink watcher start failed\n");
    }

    return ret;
}
