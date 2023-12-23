#include "pbox_keyscan_app.h"
#include <stdio.h>
#include <errno.h>
#include <linux/input.h>
#include <string.h>
#include <pthread.h>
#include "pbox_app.h"
#include "pbox_common.h"

pthread_mutex_t key_mutex;
pbox_keyevent_msg_t key_event[MAX_KEY_BUFFERED];
void keyscan_data_recv();

struct dot_key support_keys [] =
{
    /*短按*/
    {KEY_PLAY, 0, 0, 1, 0, pbox_app_key_set_playpause},
    {KEY_VOLUMEUP, 0, 0, 1, 0, pbox_app_key_set_volume_up},/*VOL_UP*/
    {KEY_VOLUMEDOWN, 0, 0, 1, 0, pbox_app_key_set_volume_down},/*VOL_DOWN*/
    {KEY_MODE, 0, 0, 1, 0 , pbox_app_key_set_echo_3a},
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

void maintask_keyscan_fd_process(int fd) {
    char buff[sizeof(pbox_keyevent_msg_t)] = {0};
    int ret = recvfrom(fd, buff, sizeof(buff), 0, NULL, NULL);
    int i = 0;
    if ((ret == 0) || (ret < 0 && (errno != EINTR))) {
        printf("%s ret:%d , error:%d\n", __func__, ret, errno);
        return;
    }
    pbox_keyevent_msg_t *msg = (pbox_keyevent_msg_t *)buff;
    printf("%s sock recv: key code: %d, press ? %d\n", __func__, msg->key_code, msg->is_key_valid);

    pthread_mutex_lock(&key_mutex);
    if (msg->is_key_valid) {
        for(i = 0; i < MAX_KEY_BUFFERED; i++) {
            if (key_event[i].key_code == 0) {
                key_event[i].key_code = msg->key_code;
                key_event[i].key_code_b = 0;
                key_event[i].is_long_press = msg->is_long_press;
                key_event[i].is_key_valid = msg->is_key_valid;
                break;
            }
        }
    }

    if (i == MAX_KEY_BUFFERED -1)
        printf("Can't find valid buffer, you should increase the value of MAX_KEY_BUFFERED\n");
    pthread_mutex_unlock(&key_mutex);
    //if (msg->is_key_valid)
        //init_time(0, keyscan_data_recv);
    return;
}

int pbox_app_key_set_playpause()
{
    printf("pbox_app_key_set_playpause =====!\n");
    if (pboxUIdata->play_status == IDLE || pboxUIdata->play_status == _STOP || pboxUIdata->play_status == _PAUSE)
        pbox_app_music_resume(DISP_All);
    else
        pbox_app_music_pause(DISP_All);
    return 1;
}

int pbox_app_key_set_volume_up()
{
    printf("---pbox_app_key_set_volume_up =====!\n");
    pbox_app_music_volume_up(DISP_All);
    return 1;
}

int pbox_app_key_set_volume_down()
{
    printf("---pbox_app_key_set_volume_down =====!\n");
    pbox_app_music_volume_down(DISP_All);
    return 1;
}

int pbox_app_key_set_mic()
{
    printf("pbox_app_key_set_mic =====!\n");
    if (pboxUIdata->mMute)
        pbox_app_music_set_mic_mute(false, DISP_All);
    else
        pbox_app_music_set_mic_mute(true, DISP_All);
    return 0;
}

int pbox_app_key_set_echo_3a() {
    printf("pbox_app_key_set_echo_3a =====!\n");
    if (pboxUIdata->echo3A)
        pbox_app_music_set_echo_3a(false, DISP_All);
    else
        pbox_app_music_set_echo_3a(true, DISP_All);
    return 0;
}

int enter_long_playpause_mode()
{
    printf("enter_long_playpause_mode\n");
    return 1;
}

int long_volume_step_up()
{
    printf("---long_volume_step_up--\n");
    return 1;
}

int long_volume_step_down()
{

    printf("---long_volume_step_down\n");
    return 1;
}


int enter_long_key_mode() {
    printf("enter_long_key_mode =====!\n");
    return 0;
}

/*recovery*/
int enter_recovery_mode() {
    printf("enter_recovery_mode\n");
	return 0;
}

int enter_combain_mode() {
    printf("enter_combain_mode\n");
	return 0;
}

int pbox_key_music_album_next() {
    printf("pbox_key_music_album_next =====!\n");
    pbox_app_music_album_next(true, DISP_All);
    return 0;
}

int enter_double_voldown_mode() {
    printf("enter_double_voldown_mode =====!\n");
    return 0;
}

int enter_double_volup_mode() {
    printf("enter_double_volup_mode =====!\n");
    return 0;
}

int enter_double_key_mode() {
    printf("enter_double_key_mode =====!\n");
    return 0;
}

int enter_double_mic_mode() {
    printf("enter_double_mic_mode =====!\n");
    return 0;
}

void *event_process_thread_ex(void * arg)
{
    int i, j , k, m;
    while(1) {
        pthread_mutex_lock(&key_mutex);
        for(i = 0; i < MAX_KEY_BUFFERED; i++) {
            for(j = 0; j < support_keys_size; j++) {
                if(key_event[i].key_code == support_keys[j].key_code && key_event[i].key_code_b == support_keys[j].key_code_b
                && key_event[i].is_long_press == support_keys[j].is_long_press) {
                    if (key_event[i].is_long_press == 0) {
                        int double_click_tmp = key_event[i].key_code;
                        for(k = 0;  k < support_keys_size; k++){
                            if(double_click_tmp == support_keys[k].key_code && support_keys[k].is_long_press == 4)
                            break;
                        }
                        if(k < support_keys_size){
                            pthread_mutex_unlock(&key_mutex);
                            usleep(KEY_DOUBLE_CLICK_PERIOD);
                            int hit_count =0;
                            for(m = 0; m < MAX_KEY_BUFFERED; m++) {
                            //printf("key_event[%d],key_code=%d,key_code_b=%d,is_key_valid=%d,is_long_press=%d\n",m,key_event[m].key_code, key_event[m].key_code_b,key_event[m].is_key_valid, key_event[m].is_long_press);
                                if(key_event[m].key_code == double_click_tmp){
                                    printf("-----get pressed key %d,key=%d--\n",m,key_event[m].key_code);
                                    memset(&key_event[m], 0, sizeof(pbox_keyevent_msg_t));//清除按键
                                    hit_count++;
                                }
                            }
                            if(hit_count >= 2){//double hit
                                printf("------get double click----\n");
                                if(support_keys[k].key_process){
                                    support_keys[k].key_process();
                                    break;
                                }
                            }
                        }
                    }
                    if(support_keys[j].key_process){
                        support_keys[j].key_process();
                        //printf("clear keyevent %d\n", i);
                        memset(&key_event[i], 0, sizeof(pbox_keyevent_msg_t));//清除按键
                        break;
                    } else {
                        printf("key_process NULL \n");
                    }
                    if(j == support_keys_size){
                        printf("Unhandled key values\n");
                    }
                }
            }
        }
        pthread_mutex_unlock(&key_mutex);
        usleep(100 * 1000);
    }
}

int pbox_create_KeyProcessTask(void)
{
    pthread_t evt_process;
    int err;

    pthread_mutex_init(&key_mutex, NULL);
    err = pthread_create(&evt_process, NULL, &event_process_thread_ex, NULL);
    if (err != 0)
        printf("cant creat thread event_read_thread_ex");
    return err;
}
