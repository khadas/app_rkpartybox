#include <stdio.h>
#include <errno.h>
#include <linux/input.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include "pbox_keyscan_app.h"
#include "pbox_app.h"
#include "pbox_common.h"
#if ENABLE_SARAADC == 0
void keyscan_data_recv();

static int pbox_app_key_set_playpause();
static int pbox_app_key_set_volume_up();
static int pbox_app_key_set_volume_down();
static int pbox_app_key_set_mic();
static int pbox_app_key_set_echo_3a();
static int pbox_app_key_switch_input_source(void);

static int enter_long_playpause_mode();
static int long_volume_step_down();
static int long_volume_step_up();
static int enter_long_key_mode();

static int enter_recovery_mode();
static int enter_combain_mode();

static int pbox_key_music_album_next();
static int enter_double_voldown_mode();
static int enter_double_volup_mode();
static int enter_double_key_mode();
static int enter_double_mic_mode();

struct dot_key support_keys [] =
{
    /*短按*/
    {KEY_PLAY, 0, 0, 1, 0, pbox_app_key_set_playpause},
    {KEY_VOLUMEUP, 0, 0, 1, 0, pbox_app_key_set_volume_up},/*VOL_UP*/
    {KEY_VOLUMEDOWN, 0, 0, 1, 0, pbox_app_key_set_volume_down},/*VOL_DOWN*/
    {KEY_MODE, 0, 0, 1, 0 , pbox_app_key_switch_input_source},
    {KEY_PLAYPAUSE, 0, 0, 1,  0, pbox_app_key_set_playpause},
    {KEY_MICMUTE, 0, 0, 1, 0,  pbox_app_key_set_mic},/*MIC_MUTE*/

    /*长按> 3s */
   	{KEY_PLAY, 0, 1, 1, 0, enter_long_playpause_mode},
    {KEY_VOLUMEDOWN, 0, 1, 1, 0, long_volume_step_down},/*VOL_DOWN*/
    {KEY_VOLUMEUP, 0, 1, 1, 0, long_volume_step_up},/*VOL_UP*/
    {KEY_MODE, 0, 1, 1, 0 ,enter_long_key_mode},
    {KEY_PLAYPAUSE, 1, 0, 1,  0, enter_long_playpause_mode},

    /*长按> 10s */
    {KEY_MODE, 0, 2, 1, 0, enter_recovery_mode},/*10s长按进recovery*/ 

    /*组合键*/
    {KEY_PLAY, KEY_MODE, 3, 1, 0, enter_combain_mode},       //adc 不支持组合键

    /*双击*/
    {KEY_PLAY, 0, 4, 1, 0 , pbox_key_music_album_next},
    //{KEY_VOLUMEDOWN, 0, 4, 1, 0 , enter_double_voldown_mode},
    //{KEY_VOLUMEUP, 0, 4, 1, 0 , enter_double_volup_mode},
    //{KEY_MODE, 0, 4, 1, 0 , enter_double_key_mode},
    //{KEY_MICMUTE, 0, 4, 1, 0 , enter_double_mic_mode},
};

const size_t support_keys_size = sizeof(support_keys) / sizeof(struct dot_key);

void maintask_keyevent_data_recv(pbox_keyevent_msg_t *msg)
{
    int j;

    for(j = 0; j < support_keys_size; j++) {
        if(msg->key_code == support_keys[j].key_code && msg->key_code_b == support_keys[j].key_code_b
        && msg->is_long_press == support_keys[j].is_long_press && msg->is_key_valid) {
            if(support_keys[j].key_process){
                support_keys[j].key_process();
                break;
            } else {
                ALOGI("key_process NULL \n");
            }
            if(j == support_keys_size){
                ALOGD("Unhandled key values\n");
            }
        }
    }
}

void maintask_keyscan_fd_process(int fd) {
    char buff[sizeof(pbox_keyevent_msg_t)] = {0};
    int ret = recv(fd, buff, sizeof(buff), 0);
    int i = 0;
    if ((ret == 0) || (ret < 0 && (errno != EINTR))) {
        ALOGE("%s ret:%d , error:%d\n", __func__, ret, errno);
        return;
    }
    pbox_keyevent_msg_t *msg = (pbox_keyevent_msg_t *)buff;
    ALOGI("%s sock recv: key code: %d, valid ? %d\n", __func__, msg->key_code, msg->is_key_valid);

    maintask_keyevent_data_recv(msg);
    return;
}

int pbox_app_key_set_playpause(void)
{
    ALOGD("pbox_app_key_set_playpause =====!\n");
    if (pboxUIdata->play_status == IDLE || pboxUIdata->play_status == _STOP || pboxUIdata->play_status == _PAUSE)
        pbox_app_music_resume(DISP_All);
    else
        pbox_app_music_pause(DISP_All);
    return 1;
}

int pbox_app_key_set_volume_up(void)
{
    ALOGD("---pbox_app_key_set_volume_up =====!\n");
    pbox_app_music_volume_up(DISP_All|DISP_FS);
    return 1;
}

int pbox_app_key_set_volume_down(void)
{
    ALOGD("---pbox_app_key_set_volume_down =====!\n");
    pbox_app_music_volume_down(DISP_All|DISP_FS);
    return 1;
}

int pbox_app_key_set_mic(void)
{
    ALOGD("pbox_app_key_set_mic =====!\n");
    if (pboxUIdata->micData[0].micmute)
        pbox_app_music_set_mic_mute(0, false, DISP_All);
    else
        pbox_app_music_set_mic_mute(0, true, DISP_All);
    return 0;
}

int pbox_app_key_set_echo_3a(void) {
    ALOGD("pbox_app_key_set_echo_3a =====!\n");
    if (pboxUIdata->micData[0].echo3a)
        pbox_app_music_set_echo_3a(0, false, DISP_All);
    else
        pbox_app_music_set_echo_3a(0, true, DISP_All);
    return 0;
}

int pbox_app_key_switch_input_source(void) {
    input_source_t dest = pboxData->inputDevice;
    pboxUIdata->autoSource = false;

    for (int i = 0; i< SRC_NUM; i++) {
        if(input_priority[i] == pboxData->inputDevice) {
            dest = input_priority[(i+1)%SRC_NUM];
            break;
        }
    }

    ALOGI("%s change [%d->%d]=====!\n", __func__, pboxData->inputDevice, dest);
    if(dest != pboxData->inputDevice) {
        pbox_app_switch_to_input_source(dest, DISP_All);
        return 0;
    }
    return -1;
}

int enter_long_playpause_mode(void)
{
    ALOGD("enter_long_playpause_mode\n");
    return 1;
}

int long_volume_step_up(void)
{
    ALOGD("---long_volume_step_up--\n");
    return 1;
}

int long_volume_step_down(void)
{

    ALOGD("---long_volume_step_down\n");
    return 1;
}


int enter_long_key_mode(void) {
    ALOGD("enter_long_key_mode =====!\n");
    return 0;
}

/*recovery*/
int enter_recovery_mode(void) {
    ALOGD("enter_recovery_mode\n");
	return 0;
}

int enter_combain_mode(void) {
    ALOGD("enter_combain_mode\n");
	return 0;
}

int pbox_key_music_album_next(void) {
    ALOGD("pbox_key_music_album_next =====!\n");
    pbox_app_music_album_next(true, DISP_All);
    return 0;
}

int enter_double_voldown_mode(void) {
    ALOGD("enter_double_voldown_mode =====!\n");
    return 0;
}

int enter_double_volup_mode(void) {
    ALOGD("enter_double_volup_mode =====!\n");
    return 0;
}

int enter_double_key_mode(void) {
    ALOGD("enter_double_key_mode =====!\n");
    return 0;
}

int enter_double_mic_mode(void) {
    ALOGD("enter_double_mic_mode =====!\n");
    return 0;
}

#endif