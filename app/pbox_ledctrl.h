#ifndef __PBOX_LEDCTRL_H
#define __PBOX_LEDCTRL_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define PATH_LED "/sys/class/leds/"

#define FG_COLOR    1
#define BG_COLOR    2

#define RK_ECHO_SYSTEM_BOOTING                         "led_effect_system_booting"  //开机灯效
#define RK_ECHO_SYSTEM_BOOTC                           "led_effect_system_booting"
#define RK_ECHO_NET_CONNECT_RECOVERY                   "led_effect_net_connet_recovery"  //网络恢复
#define RK_ECHO_NET_CONNECT_WAITTING                   "led_effect_net_connet_waiting"	//等待网络连接
#define RK_ECHO_NET_CONNECTING                         "led_effect_net_conneting"	//正在连件网络
#define RK_ECHO_NET_CONNECT_FAIL                       "led_effect_net_connet_fail" //网络连接失败
#define RK_ECHO_NET_CONNECT_SUCCESS                    "led_effect_net_connet_success"	//连接网络成功
#define RK_ECHO_WAKEUP_WAITTING                        "led_effect_wakeup_waiting"	//唤醒灯效
#define RK_ECHO_TTS_THINKING                           "led_effect_tts_thinking"      //语音解析灯效
#define RK_ECHO_TTS_PLAYING                            "led_effect_tts_playing"//播放语音灯效
#define RK_ECHO_BT_PAIR_WAITTING                       "led_effect_bt_pair_waiting"   //等待蓝牙配对
#define RK_ECHO_BT_PAIRING                             "led_effect_bt_pairing" //蓝牙正在配对
#define RK_ECHO_BT_PAIR_FAIL                           "led_effect_bt_pair_fail" //蓝牙配对失败
#define RK_ECHO_BT_PAIR_SUCCESS                        "led_effect_bt_pair_success"  //蓝牙配对成功
#define RK_ECHO_VOLUME_LED                             "led_effect_volume_led"  //音量
#define RK_ECHO_MIC_MUTE                               "led_effect_mic_mute"  //禁麦
#define RK_ECHO_MIC_UNMUTE                             "led_effect_mic_unmute" //取消禁麦
#define RK_ECHO_ALARM                                  "led_effect_alarm"	//闹钟
#define RK_ECHO_UPGRADING                              "led_effect_upgrading"	//升级中
#define RK_ECHO_UPGRADE_END                            "led_effect_upgrade_progress"	 //升级完成
#define RK_ECHO_LED_OFF                                "led_effect_led_off"	 //关闭灯效
#define RK_ECHO_CHARGER                                "led_effect_charger"  //充电灯效
#define RK_ECHO_LOW_BATTERY                            "led_effect_low_battery" //低电量
#define RK_ECHO_PHONE                                  "led_effect_phone"
#define RK_ECHO_TIME                                   "led_effect_time"
#define RK_ECHO_TEST                                   "led_effect_test"
#define RK_ECHO_PLAY                                   "led_effect_play" //播放
#define RK_ECHO_PAUSE                                  "led_effect_pause" //暂停
#define RK_ECHO_PLAY_PREV                              "led_effect_play_prev" //上一首
#define RK_ECHO_PLAY_NEXT                              "led_effect_play_next" //下一首

typedef enum {
    RK_LIGHT_UNIT_LED,
    RK_LIGHT_UNIT_RGB,
    RK_LIGHT_UNIT_UNKNOW,
} pbox_light_unit_type_t;

typedef enum {
    RK_RGB_ORDER_RGB,
    RK_RGB_ORDER_RBG,
    RK_RGB_ORDER_GBR,
    RK_RGB_ORDER_GRB,
    RK_RGB_ORDER_BRG,
    RK_RGB_ORDER_BGR,
    RK_RGB_ORDER_UNKNOW,
} pbox_rgb_order_t;

struct light_effect_ctrl {
	pbox_light_unit_type_t unit_type;        //基本发光单元类型
	int unit_num;                            //基本发光单元个数
	int *position_mapp;                      //逻辑映射表
	int userspace_ctrl_init;
	int *unit_fd;                            //控制句柄表
	pbox_rgb_order_t rgb_order;              //rgb rbg gbr grb bgr grb
	int soundreactive_mute;

	int energy_total;
	int energy_total_average;
	int colors_wheel_index;
	int energy_total_data_record[3];
	int energy_data_index;
	int light_effect_drew;
};

struct led_effect {
	int period;               // 灯效周期，例如呼吸一次为3000ms.-1表示周期无限大
	int back_color;
	int fore_color;
	int start;                //需要控制的起始的led位置
	int num;                  //从start开始计算需要改变的led数量
	int scroll_num;           //跑马灯的灯个数
				  // internal data for calculate
	int actions_per_period;   // 每个period需要经过多少次action
	int led_effect_type;      //0:关闭灯效, 1：常亮 2:呼吸灯效,  3:淡进淡出 4:闪烁灯效 5:互换 6:跑马灯效  7:汇聚
};

struct effect_calcule_data {
	int leds_type;
	int *force_bright;
	int *back_bright;
	int *bright;
	int *step_bright;
	char data_valid;
	int steps_time;
};

int leds_ctrl_init(void);
void led_ctrl_exit(void);
int set_led_effect(char *led_effect_name);
void* led_handle_command(void *led_effect_name);


int led_userspace_ctrl_init(struct light_effect_ctrl * ctrl);
int led_userspace_ctrl_deinit(struct light_effect_ctrl * ctrl);
int userspace_set_rgb_color(struct light_effect_ctrl * ctrl, uint32_t rgb_index, uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif
#endif


