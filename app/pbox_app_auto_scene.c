#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "hal_partybox.h"
#include "os_minor_type.h"
#include "pbox_app.h"

#define INIT_SHAKE      0
#define INOUT_TOSHAKE   1
#define INOUT_SHAKING   2
#define INOUT_SHAKED    3
#define STEREO_TOSHAKE  4
#define STEREO_SHAKING  5
#define STEREO_SHAKED   6
#define STEREO_PARTNER_WAIT_DONE 7
#define ALL_SHAKED     8

int shake_stage = INIT_SHAKE;
int pbox_app_scene_state_machine(void) {
    int inout_shake_res, stereo_shake_res;
    static bool is_shaked;
    switch (shake_stage) {
        case INIT_SHAKE: {
            shake_stage = INOUT_TOSHAKE;
            is_shaked = false;
            printf("%s stage:[%s]->[%s]\n", __func__, CSTR(INIT_SHAKE), CSTR(INOUT_TOSHAKE));
        } break;

        case INOUT_TOSHAKE: {
            pbox_app_post_inout_detect(DISP_All);
            shake_stage = INOUT_SHAKING;
            printf("%s stage:[%s]->[%s]\n", __func__, CSTR(INOUT_TOSHAKE), CSTR(INOUT_SHAKING));
        }

        case INOUT_SHAKING: {
            inout_shake_res = pbox_app_get_and_clear_inout_value();
            if(inout_shake_res == INDOOR || inout_shake_res == OUTDOOR) {
                printf("%s inout_shake_res :%s, stage:%d\n", __func__, inout_shake_res == INDOOR? CSTR(INDOOR):CSTR(OUTDOOR), shake_stage);
                shake_stage = INOUT_SHAKED;
                printf("%s stage:[%s]->[%s]\n", __func__, CSTR(INOUT_SHAKING), CSTR(INOUT_SHAKED));
                pbox_app_post_stop_detect();
                pbox_app_music_set_outdoor_mode(inout_shake_res, DISP_All);
            }
        } break;

        case INOUT_SHAKED: {
            shake_stage = ALL_SHAKED;
            is_shaked = true;
            printf("%s stage:[%s]->[%s]\n", __func__, CSTR(INOUT_SHAKED), CSTR(ALL_SHAKED));
        } break;
    }

    switch (shake_stage) {
        case INOUT_SHAKING: {
            pbox_app_post_get_sence_value(pboxData->inputDevice, BIT(ENV_REVERB));
        } break;

        case ALL_SHAKED: {
        } break;
    }
}