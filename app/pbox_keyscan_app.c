#include <stdio.h>
#include <errno.h>
#include <linux/input.h>
#include <string.h>
#include <sys/socket.h>
#include "pbox_keyscan.h"
#include "pbox_keyscan_app.h"
#include "pbox_app.h"
#include "pbox_common.h"

static int pbox_app_key_set_playpause(float);
static int pbox_app_key_set_volume_up(float);
static int pbox_app_key_set_volume_down(float);
static int pbox_app_key_set_mic(float);
static int pbox_app_key_set_echo_3a(float);
static int pbox_app_key_switch_input_source(float);
static int pbox_app_key_set_positive(float);
static int pbox_app_key_set_negative(float);

static int enter_long_playpause_mode(float);
static int long_volume_step_down(float);
static int long_volume_step_up(float);
static int enter_long_key_mode(float);

static int enter_recovery_mode(float);
static int enter_combain_mode(float);

static int pbox_key_music_album_next(float);
static int enter_double_voldown_mode(float);
static int enter_double_volup_mode(float);
static int enter_double_key_mode(float);
static int enter_double_mic_mode(float);
static int pbox_app_knob_set_mic1_bass(float);
static int pbox_app_knob_set_mic1_treble(float);
static int pbox_app_knob_set_mic1_reverb(float);
static int pbox_app_knob_set_mic2_bass(float);
static int pbox_app_knob_set_mic2_treble(float);
static int pbox_app_knob_set_mic2_reverb(float);
static int pbox_app_knob_set_mic1_volume(float);
static int pbox_app_knob_set_mic2_volume(float);
static int pbox_app_key_switch_vocal_level(float);
static int pbox_app_key_switch_guitar_level(float);
static int pbox_app_key_switch_vocal_or_guitar(float);
static int pbox_app_key_switch_vocal_sperate(float);
static int pbox_app_key_switch_eq_mode(float);

const struct dot_key support_keys [] =
{
    /*key           keyb    press_type vaild comb    func                */
    /*短按*/
    {HKEY_PLAY,     0,      K_SHORT,      1, 0, pbox_app_key_set_playpause},
    {HKEY_VOLUP,    0,      K_SHORT,      1, 0, pbox_app_key_set_volume_up},/*VOL_UP*/
    {HKEY_VOLDOWN,  0,      K_SHORT,      1, 0, pbox_app_key_set_volume_down},/*VOL_DOWN*/
    {HKEY_MODE,     0,      K_SHORT,      1, 0, pbox_app_key_switch_input_source},
    {HKEY_MIC1MUTE, 0,      K_SHORT,      1, 0, pbox_app_key_switch_vocal_sperate},
    {HKEY_GPIO_KEY1, 0,     K_SHORT,      1, 0, pbox_app_key_set_echo_3a},
    {HKEY_GPIO_KEY2, 0,     K_SHORT,      1, 0, pbox_app_key_set_mic},/*MIC_MUTE*/
    {HKEY_ROTARY_POS, 0,    K_SHORT,      1, 0, pbox_app_key_set_positive},
    {HKEY_ROTARY_NEG, 0,    K_SHORT,      1, 0, pbox_app_key_set_negative},

    /*长按> 3s */
    {HKEY_PLAY,     0,      K_LONG,       1, 0, enter_long_playpause_mode},
    {HKEY_VOLDOWN,  0,      K_LONG,       1, 0, long_volume_step_down},/*VOL_DOWN*/
    {HKEY_VOLUP,    0,      K_LONG,       1, 0, pbox_app_key_switch_eq_mode},/*VOL_UP*/
    {HKEY_MODE,     0,      K_LONG,       1, 0, enter_long_key_mode},
    {HKEY_MIC1MUTE, 0,      K_LONG,       1, 0, pbox_app_key_switch_vocal_or_guitar},

    /*长按> 10s */
    {HKEY_MODE,     0,      K_VLONG,      1, 0, enter_recovery_mode},/*10s长按进recovery*/ 

    /*组合键*/
    {HKEY_PLAY,     KEY_MODE,K_COMB,       1, 0, enter_combain_mode},       //adc 不支持组合键

    /*双击*/
    {HKEY_PLAY,     0,      K_DQC,        1, 0, pbox_key_music_album_next},
  //{HKEY_VOLDOWN,  0,      K_DQC,        1, 0, enter_double_voldown_mode},
  //{HKEY_VOLUP,    0,      K_DQC,        1, 0, enter_double_volup_mode},
  //{HKEY_MODE,     0,      K_DQC,        1, 0, enter_double_key_mode},
  //{HKEY_MIC1MUTE, 0,      K_DQC,        1, 0, enter_double_mic_mode},

  /*knob*/
    {HKEY_MIC1BASS, 0,      K_KNOB,       1, 0, pbox_app_knob_set_mic1_bass},
    {HKEY_MIC1TREB, 0,      K_KNOB,       1, 0, pbox_app_knob_set_mic1_treble},
    {HKEY_MIC1REVB, 0,      K_KNOB,       1, 0, pbox_app_knob_set_mic1_reverb},
    {HKEY_MIC2BASS, 0,      K_KNOB,       1, 0, pbox_app_knob_set_mic2_bass},
    {HKEY_MIC2TREB, 0,      K_KNOB,       1, 0, pbox_app_knob_set_mic2_treble},
    {HKEY_MIC2REVB, 0,      K_KNOB,       1, 0, pbox_app_knob_set_mic2_reverb},
    {HKEY_MIC1_VOL, 0,      K_KNOB,       1, 0, pbox_app_knob_set_mic1_volume},
    {HKEY_MIC2_VOL, 0,      K_KNOB,       1, 0, pbox_app_knob_set_mic2_volume},
};

const size_t support_keys_size = sizeof(support_keys) / sizeof(struct dot_key);

void maintask_keyevent_data_recv(pbox_keyevent_msg_t *msg)
{
    int j;

    for(j = 0; j < support_keys_size; j++) {
        if(msg->key_code == support_keys[j].key_code && msg->key_code_b == support_keys[j].key_code_b
        && msg->press_type == support_keys[j].press_type && msg->is_key_valid) {
            if(support_keys[j].key_process){
                support_keys[j].key_process(msg->value);
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

int pbox_app_key_set_playpause(float reserved)
{
    ALOGD("%s status:%d =====!\n", __func__, pboxUIdata->play_status);
    if (pboxUIdata->play_status == IDLE || pboxUIdata->play_status == _STOP || pboxUIdata->play_status == _PAUSE)
        pbox_app_music_resume(DISP_All);
    else
        pbox_app_music_pause(DISP_All);
    return 1;
}

int pbox_app_key_set_volume_up(float reserved)
{
    ALOGD("---pbox_app_key_set_volume_up =====!\n");
    pbox_app_music_volume_up(DISP_All|DISP_FS);
    return 1;
}

int pbox_app_key_set_volume_down(float reserved)
{
    ALOGD("---pbox_app_key_set_volume_down =====!\n");
    pbox_app_music_volume_down(DISP_All|DISP_FS);
    return 1;
}

int pbox_app_key_switch_vocal_or_guitar(float reserved) {
    ALOGD("---%s =====!\n", __func__);
    //pboxData.vocal = !pboxData.vocal;
    pbox_app_switch_vocal_lib();
}

int pbox_app_key_switch_vocal_sperate(float reserved) {
    if(pboxUIdata->vocallib != VOCAL_GUITAR)
        pbox_app_music_human_vocal_level_cycle(DISP_All);
    else
        pbox_app_music_guitar_vocal_level_cycle(DISP_All);
}

int pbox_app_key_switch_eq_mode(float reserved) {
    equalizer_t mode = (++pboxUIdata->eqmode)%EQ_NUM;
    pbox_app_music_set_eq_mode(mode, DISP_All);
}

int pbox_app_key_switch_guitar_level(float reserved) {
    pbox_app_music_guitar_vocal_level_cycle(DISP_All);
}

int pbox_app_key_set_mic(float reserved)
{
    ALOGD("pbox_app_key_set_mic =====!\n");
    if (pboxUIdata->micData[0].micmute)
        pbox_app_music_set_mic_mute(0, false, DISP_All);
    else
        pbox_app_music_set_mic_mute(0, true, DISP_All);
    return 0;
}

int pbox_app_key_set_positive(float reserved)
{
    ALOGD("pbox_app_key_set_positive =====!\n");
    pbox_app_main_volume_up(DISP_All);
    return 0;
}

int pbox_app_key_set_negative(float reserved)
{
    ALOGD("pbox_app_key_set_negative =====!\n");
    pbox_app_main_volume_down(DISP_All);
    return 0;
}

int pbox_app_key_set_echo_3a(float reserved) {
    ALOGD("%s now:%d=====!\n", __func__, pboxUIdata->micData[0].echo3a);
    for(int i = 0; i < MIC_NUM; i++) {
        if (pboxUIdata->micData[0].echo3a)
            pbox_app_music_set_echo_3a(i, false, DISP_All);
        else
            pbox_app_music_set_echo_3a(i, true, DISP_All);
    }
    return 0;
}

int pbox_app_key_switch_input_source(float reserved) {
    input_source_t dest = pboxData->inputDevice;
    pboxUIdata->autoSource = false;

    for (int i = 0; i < SRC_NUM; i++) {
        if(!input_order_config[i].enable) {
            continue;
        }
        if(input_order_config[i].source == pboxData->inputDevice) {
            do {
                dest = input_order_config[++i%SRC_NUM].source;
            } while(!input_order_config[i%SRC_NUM].enable);
            break;
        }
    }

    ALOGW("%s change: [%s->%s]\n", __func__, getInputSourceString(pboxData->inputDevice), getInputSourceString(dest));
    if(dest != pboxData->inputDevice) {
        pbox_app_switch_to_input_source(dest, DISP_All);
        return 0;
    }
    return -1;
}

int pbox_app_knob_set_mic1_bass(float bass) {
    pbox_app_music_set_mic_bass(0,  bass, DISP_All);
    return true;
}

int pbox_app_knob_set_mic1_treble(float treble) {
    pbox_app_music_set_mic_treble(0,  treble, DISP_All);
    return true;
}

int pbox_app_knob_set_mic1_reverb(float reverb) {
    pbox_app_music_set_mic_reverb(0,  reverb, DISP_All);
    return true;
}

int pbox_app_knob_set_mic2_bass(float bass) {
    pbox_app_music_set_mic_bass(1,  bass, DISP_All);
    return true;
}

int pbox_app_knob_set_mic2_treble(float treble) {
    pbox_app_music_set_mic_treble(1,  treble, DISP_All);
    return true;
}

int pbox_app_knob_set_mic2_reverb(float reverb) {
    pbox_app_music_set_mic_reverb(1,  reverb, DISP_All);
    return true;
}

int pbox_app_knob_set_mic1_volume(float volume) {
    pbox_app_music_set_mic_volume(0,  volume, DISP_All);
    return true;
}

int pbox_app_knob_set_mic2_volume(float volume) {
    pbox_app_music_set_mic_volume(1,  volume, DISP_All);
    return true;
}

int enter_long_playpause_mode(float arg)
{
    ALOGD("enter_long_playpause_mode\n");
    return 1;
}
int long_volume_step_up(float arg)
{
    ALOGD("---long_volume_step_up--\n");
    return 1;
}

int long_volume_step_down(float arg)
{
    pboxData->dispMicEngery = !pboxData->dispMicEngery;
    ALOGW("%s---dispMicEngery:%d\n", __func__, pboxData->dispMicEngery);
    return 1;
}

int enter_long_key_mode(float arg) {
    ALOGD("enter_long_key_mode =====!\n");
    return 0;
}

/*recovery*/
int enter_recovery_mode(float arg) {
    ALOGD("enter_recovery_mode\n");
    return 0;
}

int enter_combain_mode(float arg) {
    ALOGD("enter_combain_mode\n");
    return 0;
}

int pbox_key_music_album_next(float arg) {
    ALOGD("pbox_key_music_album_next =====!\n");
    pbox_app_music_album_next(true, DISP_All);
    return 0;
}

int enter_double_voldown_mode(float arg) {
    ALOGD("enter_double_voldown_mode =====!\n");
    return 0;
}

int enter_double_volup_mode(float arg) {
    ALOGD("enter_double_volup_mode =====!\n");
    return 0;
}

int enter_double_key_mode(float arg) {
    ALOGD("enter_double_key_mode =====!\n");
    return 0;
}

int enter_double_mic_mode(float arg) {
    ALOGD("enter_double_mic_mode =====!\n");
    return 0;
}
