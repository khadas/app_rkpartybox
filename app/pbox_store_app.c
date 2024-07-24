#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "param.h"
#include "os_minor_type.h"
#include "pbox_common.h"
#include "pbox_app.h"
#include "rk_utils.h"

static int ramdata_ditry = 0;

int pbox_app_music_store_volume(float volume) {
    ramdata_ditry++;
    int ret =  rk_param_set_float("ui:mainVolume", volume);

    ALOGD("%s:%f ret:%d \n", __func__, volume, ret);
    return ret;
}

int pbox_app_music_store_music_volume(float volume) {
    ramdata_ditry++;
    ALOGD("%s:%f \n", __func__, volume);
    return rk_param_set_float("ui:musicVolume", volume);
}

int pbox_app_music_store_accomp_music_level(uint32_t level) {
    ramdata_ditry++;
    return rk_param_set_int("ui:accomLevel", (int)level);
}

int pbox_app_music_store_human_music_level(uint32_t level) {
    ramdata_ditry++;
    return rk_param_set_int("ui:humanLevel", (int)level);
}

int pbox_app_music_store_reserv_music_level(uint32_t level) {
    ramdata_ditry++;
    return rk_param_set_int("ui:reservLevel", (int)level);
}

int pbox_app_music_store_stereo_mode(stereo_mode_t stereo) {
    ramdata_ditry++;
    return rk_param_set_int("ui:stereo", (int)(stereo));
}

int pbox_app_store_vocalFadeout_switch(bool split) {
    ramdata_ditry++;
    return rk_param_set_bool("ui:vocalSplit", split);
}

int pbox_app_music_store_outdoor_mode(inout_door_t outdoor) {
    ramdata_ditry++;
    return rk_param_set_int("ui:outdoor", (int)outdoor);
}

int pbox_app_music_store_placement(placement_t place) {
    ramdata_ditry++;
    return rk_param_set_int("ui:placement", (int)place);
}

int pbox_app_music_store_eq_mode(equalizer_t mode) {
    ramdata_ditry++;
    return rk_param_set_int("ui:eqmode", (int)mode);
}

int pbox_app_music_store_mic_mux(uint8_t index, mic_mux_t mux) {
    char param_name[20];
    ramdata_ditry++;
    snprintf(param_name, sizeof(param_name), "mic%d:micMux", index);
    return rk_param_set_int(param_name, (int)mux);
}

int pbox_app_music_store_mic_volume(uint8_t index, float volume) {
    char param_name[20];

    ramdata_ditry++;
    snprintf(param_name, sizeof(param_name), "mic%d:micVolume", index);
    int ret =  rk_param_set_float(param_name, volume);
    ALOGD("%s: %f, ret:%d \n", __func__, volume, ret);

    return ret;
}

int pbox_app_music_store_echo_3a(uint8_t index, bool enable) {
    char param_name[20];
    ramdata_ditry++;
    snprintf(param_name, sizeof(param_name), "mic%d:echo3a", index);
    return rk_param_set_bool(param_name, enable);
}

int pbox_app_music_store_mic_mute(uint8_t index, bool mute) {
    char param_name[20];
    ramdata_ditry++;
    snprintf(param_name, sizeof(param_name), "mic%d:micmute", index);
    return rk_param_set_bool(param_name, mute);
}

int pbox_app_music_store_recoder_revert(uint8_t index, pbox_revertb_t reverbMode) {
    char param_name[20];
    ramdata_ditry++;
    snprintf(param_name, sizeof(param_name), "mic%d:reverbMode", index);
    return rk_param_set_int(param_name, reverbMode);
}

int pbox_app_music_store_mic_treble(uint8_t index, float treble) {
    char param_name[20];
    ramdata_ditry++;
    snprintf(param_name, sizeof(param_name), "mic%d:micTreble", index);
    return rk_param_set_float(param_name, treble);
}

int pbox_app_music_store_mic_bass(uint8_t index, float bass) {
    char param_name[20];
    ramdata_ditry++;
    snprintf(param_name, sizeof(param_name), "mic%d:micBass", index);
    return rk_param_set_float(param_name, bass);
}

int pbox_app_music_store_mic_reverb(uint8_t index, float reverb) {
    char param_name[20];
    ramdata_ditry++;
    snprintf(param_name, sizeof(param_name), "mic%d:micReverb", index);
    return rk_param_set_float(param_name, reverb);
}

int pbox_app_ui_load(void) {
    float volume;
    os_env_get_float("init_vol", &volume, DEFAULT_VOLUME);
    pboxUIdata->mainVolumeLevel = rk_param_get_float("ui:mainVolume", MAX_MAIN_VOLUME);
    pboxUIdata->musicVolumeLevel = rk_param_get_float("ui:musicVolume", volume);
    pboxUIdata->accomLevel = (uint32_t)rk_param_get_int("ui:accomLevel", 100);
    pboxUIdata->humanLevel = (uint32_t)rk_param_get_int("ui:humanLevel", DEFAULT_HUMAN_LEVEL);
    pboxUIdata->reservLevel = (uint32_t)rk_param_get_int("ui:reservLevel", 100);
    pboxUIdata->vocalSplit = (bool)rk_param_get_bool("ui:vocalSplit", false);

    pboxUIdata->placement = (placement_t)rk_param_get_int("ui:placement", PLACE_VERT);
    pboxUIdata->stereo = (stereo_mode_t)rk_param_get_int("ui:stereo", MODE_STEREO);
    pboxUIdata->outdoor = (inout_door_t)rk_param_get_int("ui:outdoor", OUTDOOR);
    pboxUIdata->eqmode = (equalizer_t)rk_param_get_int("ui:eqmode", EQ_POP);

    for(int i = 0; i < hal_get_mic_guitar_num(); i++) {
        char param_name[20]; // Make sure this is large enough to hold the full parameter name
        sprintf(param_name, "mic%d:micmute", i);
        pboxUIdata->micData[i].micmute = rk_param_get_bool(param_name, false);

        // Apply similar fix for the rest of the parameters
        sprintf(param_name, "mic%d:micMux", i);
        pboxUIdata->micData[i].micMux = rk_param_get_int(param_name, pboxUIdata->micData[i].micMux);

        sprintf(param_name, "mic%d:micVolume", i);
        pboxUIdata->micData[i].micVolume = rk_param_get_float(param_name, 0);

        sprintf(param_name, "mic%d:micTreble", i);
        pboxUIdata->micData[i].micTreble = rk_param_get_float(param_name, 0);

        sprintf(param_name, "mic%d:micBass", i);
        pboxUIdata->micData[i].micBass = rk_param_get_float(param_name, 0);

        sprintf(param_name, "mic%d:micReverb", i);
        pboxUIdata->micData[i].micReverb = rk_param_get_float(param_name, 50);

        sprintf(param_name, "mic%d:echo3a", i);
        pboxUIdata->micData[i].echo3a = rk_param_get_bool(param_name, true);

        sprintf(param_name, "mic%d:reverbMode", i);
        pboxUIdata->micData[i].reverbMode = rk_param_get_int(param_name, PBOX_REVERT_KTV);
    }
    return 0;
}

int pbox_app_data_save(void) {
    int ret;
    if(!ramdata_ditry) return 0;

    ret = rk_param_save();

    //ALOGD("%s ret:%d, dirty:%d\n", __func__, ret, ramdata_ditry);
    if(ret == 0) {
        ramdata_ditry = 0;
    }

    return ret;
}

int pbox_app_data_deinit(void) {
    return rk_param_deinit();
}

int pbox_app_ui_init(const char *ini_path) {
    int ret = -1;
    char path[128];
    LOG_WARN("%s: %s\n", __func__, ini_path);

    assert(ini_path);
    ret = rk_param_init((char *)ini_path);

    if(ret != 0) {
        int ret = os_copy_file("/etc/pbox/pbox-factory.ini", ini_path);
        ret = rk_param_init((char *)ini_path);
    }

    LOG_WARN("%s ret:%d\n", __func__, ret);
    return ret;
}