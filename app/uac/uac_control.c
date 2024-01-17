#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "uac_common_def.h"
#include "uac_control.h"

enum {
    UAC_ROLE_SPEAKER,
    UAC_ROLE_RECORDER,
    UAC_ROLE_NUM
};

static struct UACControl *gUAControl = NULL;
void uac_control_create(struct UACControl *uac) {
    gUAControl = uac;
}

void uac_role_change(int mode, bool start) {
    switch(mode) {
        case UAC_STREAM_RECORD: {
            gUAControl->uacRoleChange(UAC_ROLE_SPEAKER, start);
        } break;

        case UAC_STREAM_PLAYBACK: {
            gUAControl->uacRoleChange(UAC_ROLE_RECORDER, start);
        } break;
    }
}

void uac_set_sample_rate(int mode, int samplerate) {
    switch(mode) {
        case UAC_STREAM_RECORD: {
            gUAControl->uacSetSampleRate(UAC_ROLE_SPEAKER, samplerate);
        } break;

        case UAC_STREAM_PLAYBACK: {
            gUAControl->uacSetSampleRate(UAC_ROLE_RECORDER, samplerate);
        } break;
    }
}

void uac_set_volume(int mode, int volume) {
    switch(mode) {
        case UAC_STREAM_RECORD: {
            gUAControl->uacSetVolume(UAC_ROLE_SPEAKER, volume);
        } break;

        case UAC_STREAM_PLAYBACK: {
            gUAControl->uacSetVolume(UAC_ROLE_RECORDER, volume);
        } break;
    }
}
void uac_set_mute(int mode, bool mute) {
    switch(mode) {
        case UAC_STREAM_RECORD: {
            gUAControl->uacSetMute(UAC_ROLE_SPEAKER, mute);
        } break;

        case UAC_STREAM_PLAYBACK: {
            gUAControl->uacSetMute(UAC_ROLE_RECORDER, mute);
        } break;
    }
}

void uac_set_ppm(int mode, int ppm) {
    switch(mode) {
        case UAC_STREAM_RECORD: {
            gUAControl->uacSetPpm(UAC_ROLE_SPEAKER, ppm);
        } break;

        case UAC_STREAM_PLAYBACK: {
            gUAControl->uacSetPpm(UAC_ROLE_RECORDER, ppm);
        } break;
    }
}