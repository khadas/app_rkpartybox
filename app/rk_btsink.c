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

#define PRINT_FLAG_ERR "[RK_BT_ERROR]"
#define PRINT_FLAG_SUCESS "[RK_BT_SUCESS]"

int unix_socket_bt_notify_msg(void *info, int length)
{
	unix_socket_notify_msg(PBOX_MAIN_BT, info, length);
}

int bt_sink_notify_btstate(btsink_state_t state)
{
	rk_bt_msg_t msg = {0};
	msg.type = RK_BT_EVT;
	msg.msgId = BT_SINK_STATE;
	msg.btinfo.state = state;

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
	//printf("%s recv msg rack: %s[%p] %s[%p]\n", __func__, msg.btinfo.track.title, msg.btinfo.track.title, msg.btinfo.track.artist, msg.btinfo.track.artist);

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
    printf("FUNC:%s sampleFreq:%d, channel=%d!\n", __func__, sampleFreq, channel);
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

static RkBtContent bt_content;
static void bt_test_state_cb(RkBtRemoteDev *rdev, RK_BT_STATE state)
{
	switch (state) {
	//BASE STATE
	case RK_BT_STATE_TURNING_ON:
		printf("++ RK_BT_STATE_TURNING_ON\n");
		break;
	case RK_BT_STATE_INIT_ON:
		printf("++ RK_BT_STATE_INIT_ON\n");
		bt_content.init = true;
		//rk_bt_set_power(true);
		bt_sink_notify_btstate(BT_INIT_ON);
		break;
	case RK_BT_STATE_INIT_OFF:
		printf("++ RK_BT_STATE_INIT_OFF\n");
		bt_content.init = false;
		bt_sink_notify_btstate(BT_NONE);
		break;

	//SCAN STATE
	case RK_BT_STATE_SCAN_NEW_REMOTE_DEV:
		if (rdev->paired)
			printf("+ PAIRED_DEV: [%s|%d]:%s:%s\n", rdev->remote_address, rdev->rssi,
					rdev->remote_address_type, rdev->remote_alias);
		else
			printf("+ SCAN_NEW_DEV: [%s|%d]:%s:%s\n", rdev->remote_address, rdev->connected,
					rdev->remote_address_type, rdev->remote_alias);
		break;
	case RK_BT_STATE_SCAN_CHG_REMOTE_DEV:
		printf("+ SCAN_CHG_DEV: [%s|%d]:%s:%s|%s\n", rdev->remote_address, rdev->rssi,
				rdev->remote_address_type, rdev->remote_alias, rdev->change_name);
		if (!strcmp(rdev->change_name, "UUIDs")) {
			for (int index = 0; index < 36; index++) {
				if (!strcmp(rdev->remote_uuids[index], "NULL"))
					break;
				printf("\tUUIDs: %s\n", rdev->remote_uuids[index]);
			}
		} else if (!strcmp(rdev->change_name, "Icon")) {
			printf("\tIcon: %s\n", rdev->icon);
		} else if (!strcmp(rdev->change_name, "Class")) {
			printf("\tClass: 0x%x\n", rdev->cod);
		} else if (!strcmp(rdev->change_name, "Modalias")) {
			printf("\tModalias: %s\n", rdev->modalias);
		}
		break;
	case RK_BT_STATE_SCAN_DEL_REMOTE_DEV:
		printf("+ SCAN_DEL_DEV: [%s]:%s:%s\n", rdev->remote_address,
				rdev->remote_address_type, rdev->remote_alias);
		break;

	//LINK STATE
	case RK_BT_STATE_CONNECTED:
	case RK_BT_STATE_DISCONN:
		printf("+ %s [%s|%d]:%s:%s\n", rdev->connected ? "STATE_CONNECT" : "STATE_DISCONN",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		bt_sink_notify_btstate((state == RK_BT_STATE_CONNECTED) ? BT_CONNECTED:BT_DISCONNECT);
		break;
	case RK_BT_STATE_PAIRED:
	case RK_BT_STATE_PAIR_NONE:
		printf("+ %s [%s|%d]:%s:%s\n", rdev->paired ? "STATE_PAIRED" : "STATE_PAIR_NONE",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		break;
	case RK_BT_STATE_BONDED:
	case RK_BT_STATE_BOND_NONE:
		printf("+ %s [%s|%d]:%s:%s\n", rdev->bonded ? "STATE_BONDED" : "STATE_BOND_NONE",
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
		printf("+ STATE_FAILED [%s|%d]:%s:%s reason: %s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias,
				rdev->change_name);
		break;

	//MEDIA A2DP SOURCE
	case RK_BT_STATE_SRC_ADD:
	case RK_BT_STATE_SRC_DEL:
		printf("+ STATE SRC MEDIA %s [%s|%d]:%s:%s\n",
				(state == RK_BT_STATE_SRC_ADD) ? "ADD" : "DEL",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		printf("+ codec: %s, freq: %s, chn: %s\n",
					rdev->media.codec == 0 ? "SBC" : "UNKNOW",
					rdev->media.sbc.frequency == 1 ? "48K" : "44.1K",
					rdev->media.sbc.channel_mode == 1 ? "JOINT_STEREO" : "STEREO");
		break;

	//MEDIA AVDTP TRANSPORT
	case RK_BT_STATE_TRANSPORT_VOLUME:
		printf("+ STATE AVDTP TRASNPORT VOLUME[%d] [%s|%d]:%s:%s\n",
				rdev->media.volume,
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		break;
	case RK_BT_STATE_TRANSPORT_IDLE:
		printf("+ STATE AVDTP TRASNPORT IDLE [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		break;
	case RK_BT_STATE_TRANSPORT_PENDING:
		printf("+ STATE AVDTP TRASNPORT PENDING [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		break;
	case RK_BT_STATE_TRANSPORT_ACTIVE:
		printf("+ STATE AVDTP TRASNPORT ACTIVE [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		bt_sink_notify_a2dpstate(A2DP_STREAMING);
		break;
	case RK_BT_STATE_TRANSPORT_SUSPENDING:
		printf("+ STATE AVDTP TRASNPORT SUSPEND [%s|%d]:%s:%s\n",
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
		printf("+ STATE SINK MEDIA %s [%s|%d]:%s:%s\n",
				(state == RK_BT_STATE_SINK_ADD) ? "ADD" : "DEL",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		printf("+ codec: %s, freq: %d, chn: %d\n",
					rdev->media.codec == 0 ? "SBC" : "UNKNOW",
					freq,
					channel);
		if(state == RK_BT_STATE_SINK_ADD) {
			bt_sink_notify_pcm_format(freq, channel);
			bt_sink_notify_a2dpstate(A2DP_CONNECTED);
		} else {
			bt_sink_notify_a2dpstate(A2DP_IDLE);
		}
		break;
	case RK_BT_STATE_SINK_PLAY:
		printf("+ STATE SINK PLAYER PLAYING [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		bt_sink_notify_a2dpstate(A2DP_STREAMING);
		break;
	case RK_BT_STATE_SINK_STOP:
		printf("+ STATE SINK PLAYER STOP [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		bt_sink_notify_a2dpstate(A2DP_CONNECTED);
		break;
	case RK_BT_STATE_SINK_PAUSE:
		printf("+ STATE SINK PLAYER PAUSE [%s|%d]:%s:%s\n",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);

		bt_sink_notify_a2dpstate(A2DP_CONNECTED);
		break;
    case RK_BT_STATE_SINK_TRACK:
        printf("+ STATE SINK TRACK INFO [%s|%d]:%s:%s track[%s]-[%s]\n",
            rdev->remote_address,
            rdev->rssi,
            rdev->remote_address_type,
            rdev->remote_alias,
            rdev->title,
            rdev->artist);
        bt_sink_notify_avrcp_track(rdev->title, rdev->artist);
    break;
    case RK_BT_STATE_SINK_POSITION:
        printf("+ STATE SINK TRACK POSITION:[%s|%d]:%s:%s [%u-%u]\n",
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
		printf("RK_BT_STATE_ADAPTER_BLE_ADV_START successful\n");
		break;
	case RK_BT_STATE_ADAPTER_BLE_ADV_STOP:
		bt_content.ble_content.ble_advertised = false;
		printf("RK_BT_STATE_ADAPTER_BLE_ADV_STOP successful\n");
		break;

	//ADAPTER STATE
	case RK_BT_STATE_ADAPTER_NO_DISCOVERYABLED:
		bt_content.discoverable = false;
        bt_sink_notify_adapter_discoverable(false);
		printf("RK_BT_STATE_ADAPTER_NO_DISCOVERYABLED successful\n");
		break;
	case RK_BT_STATE_ADAPTER_DISCOVERYABLED:
		bt_content.discoverable = true;
        bt_sink_notify_adapter_discoverable(true);
		printf("RK_BT_STATE_ADAPTER_DISCOVERYABLED successful\n");
		break;
	case RK_BT_STATE_ADAPTER_NO_PAIRABLED:
		bt_content.pairable = false;
		printf("RK_BT_STATE_ADAPTER_NO_PAIRABLED successful\n");
		break;
	case RK_BT_STATE_ADAPTER_PAIRABLED:
		bt_content.pairable = true;
		printf("RK_BT_STATE_ADAPTER_PAIRABLED successful\n");
		break;
	case RK_BT_STATE_ADAPTER_NO_SCANNING:
		bt_content.scanning = false;
		printf("RK_BT_STATE_ADAPTER_NO_SCANNING successful\n");
		break;
	case RK_BT_STATE_ADAPTER_SCANNING:
		bt_content.scanning = true;
		printf("RK_BT_STATE_ADAPTER_SCANNING successful\n");
		break;
	case RK_BT_STATE_ADAPTER_POWER_ON:
		bt_content.power = true;
		printf("RK_BT_STATE_ADAPTER_POWER_ON successful\n");
		break;
	case RK_BT_STATE_ADAPTER_POWER_OFF:
		bt_content.power = false;
		printf("RK_BT_STATE_ADAPTER_POWER_OFF successful\n");
		break;
	case RK_BT_STATE_COMMAND_RESP_ERR:
		printf("RK_BT_STATE CMD ERR!!!\n");
		break;
	case RK_BT_STATE_DEL_DEV_OK:
		if (rdev != NULL)
			printf("+ RK_BT_STATE_DEL_DEV_OK: %s:%s:%s\n",
				rdev->remote_address,
				rdev->remote_address_type,
				rdev->remote_alias);
		break;
	default:
		if (rdev != NULL)
			printf("+ DEFAULT STATE %d: %s:%s:%s RSSI: %d [CBP: %d:%d:%d]\n", state,
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
	printf("RK BT VERSION: %s-[%d-%s-%d:%s]\n", rk_bt_version(), year, month, day, __TIME__);
}

static bool bt_test_audio_server_cb(void)
{
	printf("%s bt_content.profile: 0x%x:0x%x:0x%x\n", __func__, bt_content.profile,
			(bt_content.profile & PROFILE_A2DP_SINK_HF),
			(bt_content.profile & PROFILE_A2DP_SOURCE_AG));
	if (bt_content.bluealsa == true) {
		//use bluealsa
		char rsp[64];
		exec_command("pactl list modules | grep bluetooth", rsp, 64);
		if (rsp[0]) {
			exec_command_system("pactl unload-module module-bluetooth-policy");
			exec_command_system("pactl unload-module module-bluetooth-discover");
		}

		exec_command_system("killall bluealsa bluealsa-aplay");

		if ((bt_content.profile & PROFILE_A2DP_SINK_HF) == PROFILE_A2DP_SINK_HF) {
			exec_command_system("bluealsa -S --profile=a2dp-sink --profile=hfp-hf &");
			//Sound Card: default
			exec_command_system("bluealsa-aplay -S --profile-a2dp 00:00:00:00:00:00 &");
		} else if ((bt_content.profile & PROFILE_A2DP_SOURCE_AG) == PROFILE_A2DP_SOURCE_AG) {
			exec_command_system("bluealsa -S --profile=a2dp-source --profile=hfp-ag --a2dp-volume &");
		}
	}

	return true;
}

static bool bt_test_vendor_cb(bool enable)
{
	int times = 100;

	if (enable) {
		//vendor
		//broadcom
		if (get_ps_pid("brcm_patchram_plus1"))
			kill_task("brcm_patchram_plus1");

		//realtek
		if (get_ps_pid("rtk_hciattach"))
			kill_task("rtk_hciattach");

		//The hci0 start to init ...
		if (!access("/usr/bin/wifibt-init.sh", F_OK))
			exec_command_system("/usr/bin/wifibt-init.sh start_bt");
		else if (!access("/usr/bin/bt_init.sh", F_OK))
			exec_command_system("/usr/bin/bt_init.sh");

		//wait hci0 appear
		while (times-- > 0 && access("/sys/class/bluetooth/hci0", F_OK)) {
			usleep(100 * 1000);
		}

		if (access("/sys/class/bluetooth/hci0", F_OK) != 0) {
			printf("The hci0 init failure!\n");
			return false;
		}

		/* ensure bluetoothd running */
		/*
		 * DEBUG: vim /etc/init.d/S40bluetooth, modify BLUETOOTHD_ARGS="-n -d"
		 */
		if (access("/etc/init.d/S40bluetooth", F_OK) == 0)
			exec_command_system("/etc/init.d/S40bluetooth restart");
		else if (access("/etc/init.d/S40bluetoothd", F_OK) == 0)
			exec_command_system("/etc/init.d/S40bluetoothd restart");

		//or
		//exec_command_system("/usr/libexec/bluetoothd -n -P battery");
		//or debug
		//exec_command_system("/usr/libexec/bluetoothd -n -P battery -d");
		//exec_command_system("hcidump xxx or btmon xxx");

		//check bluetoothd
		times = 100;
		while (times-- > 0 && !(get_ps_pid("bluetoothd"))) {
			usleep(100 * 1000);
		}

		if (!get_ps_pid("bluetoothd")) {
			printf("The bluetoothd boot failure!\n");
			return false;
		}
	} else {
		//CLEAN
		exec_command_system("hciconfig hci0 down");
		exec_command_system("/etc/init.d/S40bluetooth stop");

		//vendor deinit
		if (get_ps_pid("brcm_patchram_plus1"))
			kill_task("killall brcm_patchram_plus1");
		if (get_ps_pid("rtk_hciattach"))
			kill_task("killall rtk_hciattach");

		//audio server deinit
		if (get_ps_pid("bluealsa"))
			kill_task("bluealsa");
		if (get_ps_pid("bluealsa-alay"))
			kill_task("bluealsa-alay");
	}

	return true;
}

static int bt_restart_a2dp_sink(bool onlyAplay)
{
	char ret_buff[1024];

	kill_task("pulseaudio");
    //msleep(100);
    if(!onlyAplay)
	    kill_task("bluealsa");

	kill_task("bluealsa-aplay");
	msleep(10);

    if(!onlyAplay) {
		run_task("bluealsa", "bluealsa --profile=a2dp-sink --a2dp-volume --initial-volume=70 --a2dp-force-audio-cd &");
    }

	run_task("bluealsa-aplay", "bluealsa-aplay --profile-a2dp --pcm=plughw:7,0,0 00:00:00:00:00:00 &");
	exec_command_system("hciconfig hci0 class 0x240404");

	return 0;
}

static void *btsink_server(void *arg)
{
	char bt_name[64];
	unsigned int addr[6] = {0};

	pthread_setname_np(pthread_self(), "pbox_btserver");

	exec_command("hciconfig hci0 | awk '/BD Address:/ {print $3}'", bt_name, sizeof(bt_name));
	if(bt_name[0]) {
		if (sscanf(bt_name, "%2X:%2X:%2X:%2X:%2X:%2X",
				&addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]) != 6) {
			fprintf(stderr, "Failed to parse Bluetooth address.\n");
			return (void*)-1;
		}
	}

    printf("%s thread: %lu\n", __func__, (unsigned long)pthread_self());

	printf("%s \n", __func__);
	memset(&bt_content, 0, sizeof(RkBtContent));

	//BREDR CLASS BT NAME
	if(bt_name[0]) {
		sprintf(bt_name, "rk-partybox-%02X", addr[5]);
		bt_content.bt_name = bt_name;
	}
	else
		bt_content.bt_name = "rk-partybox";

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

	char buff[sizeof(rk_bt_msg_t)] = {0};

    int sockfd = create_udp_socket(SOCKET_PATH_BTSINK_SERVER);

	while(true) {
		memset(buff, 0, sizeof(buff));
		int ret = recvfrom(sockfd, buff, sizeof(buff), 0, NULL, NULL);
		if (ret <= 0)
			continue;

		rk_bt_msg_t *msg = (rk_bt_msg_t *)buff;
		printf("%s sock recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

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
    			case RK_BT_PAIRABLE:{
					//exec_command_system("hciconfig hci0 class 0x240404");
    				rk_bt_set_discoverable(true);
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
    				bt_restart_a2dp_sink(0);
                } break;
    			case RK_BT_START_BLUEALSA_APLAY:{
    				bt_restart_a2dp_sink(1);
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
        printf("btsink server start failed\n");
    }

    ret = pthread_create(&tid_watch, NULL, btsink_watcher, NULL);
    if (ret < 0)
    {
        printf("btsink watcher start failed\n");
    }

    return ret;
}