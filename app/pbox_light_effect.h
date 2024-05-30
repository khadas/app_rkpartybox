#ifndef __PBOX_LIGHT_EFFECT__
#define __PBOX_LIGHT_EFFECT__

#include "pbox_common.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <fcntl.h>



typedef enum {
    //command
    PBOX_LIGHT_EFFECT_SOUNDREACTIVE_EVT,
    PBOX_LIGHT_EFFECT_EVT,

    RK_ECHO_SYSTEM_BOOTING_EVT,               //"led_effect_system_booting"  //开机灯效
    RK_ECHO_SYSTEM_BOOTC_EVT,                 //"led_effect_system_booting"
    RK_ECHO_NET_CONNECT_RECOVERY_EVT,         //"led_effect_net_connet_recovery"  //网络恢复
    RK_ECHO_NET_CONNECT_WAITTING_EVT,         //"led_effect_net_connet_waiting"	//等待网络连接
    RK_ECHO_NET_CONNECTING_EVT,               //"led_effect_net_conneting"	//正在连件网络
    RK_ECHO_NET_CONNECT_FAIL_EVT,             //"led_effect_net_connet_fail" //网络连接失败
    RK_ECHO_NET_CONNECT_SUCCESS_EVT,          //"led_effect_net_connet_success"	//连接网络成功
    RK_ECHO_WAKEUP_WAITTING_EVT,              //"led_effect_wakeup_waiting"	//唤醒灯效
    RK_ECHO_TTS_THINKING_EVT,                 //"led_effect_tts_thinking"      //语音解析灯效
    RK_ECHO_TTS_PLAYING_EVT,                  //"led_effect_tts_playing"//播放语音灯效
    RK_ECHO_BT_PAIR_WAITTING_EVT,             //"led_effect_bt_pair_waiting"   //等待蓝牙配对
    RK_ECHO_BT_PAIRING_EVT,                   //"led_effect_bt_pairing" //蓝牙正在配对
    RK_ECHO_BT_PAIR_FAIL_EVT,                 //"led_effect_bt_pair_fail" //蓝牙配对失败
    RK_ECHO_BT_PAIR_SUCCESS_EVT,              //"led_effect_bt_pair_success"  //蓝牙配对成功
    RK_ECHO_VOLUME_LED_EVT,                   //"led_effect_volume_led"  //音量
    RK_ECHO_MIC_MUTE_EVT,                     //"led_effect_mic_mute"  //禁麦
    RK_ECHO_MIC_UNMUTE_EVT,                   //"led_effect_mic_unmute" //取消禁麦
    RK_ECHO_ALARM_EVT,                        //"led_effect_alarm"	//闹钟
    RK_ECHO_UPGRADING_EVT,                    //"led_effect_upgrading"	//升级中
    RK_ECHO_UPGRADE_END_EVT,                  //"led_effect_upgrade_progress"	 //升级完成
    RK_ECHO_LED_OFF_EVT,                      //"led_effect_led_off"	 //关闭灯效
    RK_ECHO_CHARGER_EVT,                      //"led_effect_charger"  //充电灯效
    RK_ECHO_LOW_BATTERY_EVT,                  //"led_effect_low_battery" //低电量
    RK_ECHO_PHONE_EVT,                        //"led_effect_phone"
    RK_ECHO_TIME_EVT,                         //"led_effect_time"
    RK_ECHO_TEST_EVT,
    RK_ECHO_PLAY_EVT,                         //"led_effect_play" //播放
    RK_ECHO_PAUSE_EVT,                        //"led_effect_pause" //暂停
    RK_ECHO_PLAY_PREV_EVT,                    //"led_effect_play_prev" //上一首
    RK_ECHO_PLAY_NEXT_EVT,                    //"led_effect_play_next" //下一首
} pbox_light_effect_opcode_t;

enum {
	RK_LIGHT_EFFECT_CMD = 1,
};

typedef int rk_light_effect_msg_t;

typedef struct {
	int size;
	energy_t energykeep[10];
} energy_data_t;

typedef struct {
    pbox_msg_t type;
    pbox_light_effect_opcode_t msgId;
    energy_data_t energy_data;
    int volume;
    int mic_volume;
} pbox_light_effect_msg_t;

int pbox_create_lightEffectTask(void);
int pbox_stop_light_effect(void);

int pbox_light_effect_send_cmd(pbox_light_effect_opcode_t command, void *data, int len);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
