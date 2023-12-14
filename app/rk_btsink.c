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
#include "rk_btsink.h"
#include "rk_utils.h"
#include "pbox_common.h"
#include "pbox_socket.h"

#define PRINT_FLAG_ERR "[RK_BT_ERROR]"
#define PRINT_FLAG_SUCESS "[RK_BT_SUCESS]"

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

int bt_sink_notify_avrcp_position(unsigned int posistion, unsigned int total)
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

int unix_socket_bt_notify_msg(void *info, int length)
{
	unix_socket_notify_msg(PBOX_MAIN_BT, info, length);
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
		rk_bt_set_power(true);
		bt_sink_notify_btstate(BT_ON);
		break;
	case RK_BT_STATE_INIT_OFF:
		printf("++ RK_BT_STATE_INIT_OFF\n");
		bt_content.init = false;
		bt_sink_notify_btstate(BT_NONE);
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
		//rk_bt_set_discoverable(true);
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
		break;

	//MEDIA AVDTP TRANSPORT
	case RK_BT_STATE_TRANSPORT_VOLUME:
		printf("+ STATE AVDTP TRASNPORT VOLUME[%d] [%s|%d]:%s:%s\n",
				rdev->volume,
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
		printf("+ STATE SINK MEDIA %s [%s|%d]:%s:%s\n",
				(state == RK_BT_STATE_SINK_ADD) ? "ADD" : "DEL",
				rdev->remote_address,
				rdev->rssi,
				rdev->remote_address_type,
				rdev->remote_alias);
		if(state == RK_BT_STATE_SINK_ADD) {
			bt_sink_notify_a2dpstate(A2DP_CONNECTED);
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

	//ADAPTER STATE
	case RK_BT_STATE_ADAPTER_NO_DISCOVERYABLED:
		bt_content.discoverable = false;
        //bt_sink_notify_adapter_info(BT_SINK_ADPTER_DISCOVERABLE, 0);
		printf("RK_BT_STATE_ADAPTER_NO_DISCOVERYABLED successful\n");
		break;
	case RK_BT_STATE_ADAPTER_DISCOVERYABLED:
		bt_content.discoverable = true;
        //bt_sink_notify_adapter_info(BT_SINK_ADPTER_DISCOVERABLE, 1);
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
		rk_bt_set_discoverable(true);
		break;
	case RK_BT_STATE_ADAPTER_POWER_OFF:
		bt_content.power = false;
		printf("RK_BT_STATE_ADAPTER_POWER_OFF successful\n");
		break;
	case RK_BT_STATE_COMMAND_RESP_ERR:
		printf("RK_BT_STATE CMD ERR!!!\n");
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
    	exec_command_system("bluealsa --profile=a2dp-sink --a2dp-volume --initial-volume=70 --a2dp-force-audio-cd &");
    	msleep(500);

    	if (get_ps_pid_new("bluealsa")< 0) {
    		printf("start a2dp sink profile failed!\n");
    		return -1;
    	}
    }

#if 1
	exec_command_system("bluealsa-aplay --profile-a2dp --pcm=plughw:7,0,0 00:00:00:00:00:00 &");
    msleep(100);
	if (get_ps_pid_new("bluealsa-aplay")< 0) {
		printf("start a2dp sink profile failed!\n");
		return -1;
	}

#endif

	//exec_command_system("hciconfig hci0 class 0x240404");
	//msleep(100);

	return 0;
}

static void *btsink_server(void *arg)
{
	pthread_setname_np(pthread_self(), "pbox_btserver");
    printf("%s thread: %lu\n", __func__, (unsigned long)pthread_self());

	printf("%s \n", __func__);
	memset(&bt_content, 0, sizeof(RkBtContent));

	//BREDR CLASS BT NAME
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
	bt_content.profile.a2dp_sink_hfp_hf = true;
	bt_content.profile.a2dp_source_hfp_ag = !bt_content.profile.a2dp_sink_hfp_hf;

	// enable ble
	bt_content.profile.ble = false;

	rk_bt_register_state_callback(bt_test_state_cb);
	//rk_bt_register_bond_callback(bt_test_bond_state_cb);

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
                #if 0
    			case RK_BT_VOL_DOWN:
    				rkbtsinkData.volume--;
    				if(rkbtsinkData.volume < 0)
    					rkbtsinkData.volume = 0;
    				printf("%s, bt volume:%d", __func__, rkbtsinkData.volume);
    				rk_bt_sink_set_volume(volumeTable[rkbtsinkData.volume]);
    				break;
    			case RK_BT_VOL_UP:
    				rkbtsinkData.volume++;
    				if(rkbtsinkData.volume>=sizeof(volumeTable)/sizeof(int))
    					rkbtsinkData.volume = sizeof(volumeTable)/sizeof(int) -1;
    				printf("%s, bt volume:%d", __func__, rkbtsinkData.volume);
    				rk_bt_sink_set_volume(volumeTable[rkbtsinkData.volume]);
    				break;
    			case RK_BT_ABS_VOL: {
    				int vol = (int)(msg->btcmd.data[0]);
    				rkbtsinkData.volume=vol/10;
    				if(rkbtsinkData.volume > 10)
    					rkbtsinkData.volume = 10;
    				else if(rkbtsinkData.volume < 0)
    					rkbtsinkData.volume = 0;
    				printf("%s, abs volume:%d", __func__, rkbtsinkData.volume);
    				rk_bt_sink_set_volume(volumeTable[rkbtsinkData.volume]);
    			} break;
                #endif
    			case RK_BT_PAIRABLE:{
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

