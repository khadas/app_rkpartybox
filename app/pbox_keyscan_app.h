#ifndef _PTBOX_KEYSCAN_APP_H_
#define _PTBOX_KEYSCAN_APP_H_
#include <stdint.h>
#include <stdbool.h>
#include "pbox_common.h"
#include <sys/time.h>
#include <linux/input.h>
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_KEY_BUFFERED	8

#define KEY_DOUBLE_CLICK_PERIOD        (300*1000)   //双击键间隔时间，单位US
#define KEY_LONG_PRESS_PREIOD           3000        //长按时间间隔，单位ms
#define KEY_SWITCH_MSEC                 6000
#define KEY_VERY_LONG_PRESS_PERIOD      10000       //超长按时间间隔，单位ms

typedef struct {
    int key_code;
    int key_code_b;
    int is_long_press;
    int is_key_valid;
struct timeval time;
} pbox_keyevent_msg_t;

struct dot_key
{
    int key_code;
    int key_code_b;
    int is_long_press;//0表示短按，1表示大于3s的长按，2表示大于10s的超长按，3表示长按组合键，4表示快速双击
    int is_key_valid;
    int is_combain_key; //combian key
    /*
     * pre_alexa_mode is used if only one key to switch from different mode, such as wifi/bt/mic_mute mode.
     * we can define the previous alexa mode as ALEXA_WIFI_MODE if we want to press one key to switch from
     * wifi mode to other mode.
     * if pre_alexa_mode defined as ALEXA_INVALID_MODE, means this value is not used
     */
    int (*key_process)(void);
    struct timeval time;
};

void maintask_keyscan_fd_process(int fd);
int  pbox_create_KeyProcessTask(void);

int pbox_app_key_set_playpause();
int pbox_app_key_set_volume_up();
int pbox_app_key_set_volume_down();
int pbox_app_key_set_mic();
int pbox_app_key_set_echo_3a();

int enter_long_playpause_mode();
int long_volume_step_down();
int long_volume_step_up();
int enter_long_key_mode();

int enter_recovery_mode();
int enter_combain_mode();

int pbox_key_music_album_next();
int enter_double_voldown_mode();
int enter_double_volup_mode();
int enter_double_key_mode();
int enter_double_mic_mode();

#ifdef __cplusplus
}
#endif
#endif