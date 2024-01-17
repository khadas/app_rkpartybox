#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stddef.h>
#include "uac_common_def.h"
#include "uac_uevent.h"
#include "uac_control.h"
/*
 * case 1:
 * the UAC1 uevent when pc/remote close(play sound of usb close)
 *
 * strs[0] = ACTION=change
 * strs[1] = DEVPATH=/devices/virtual/u_audio/UAC1_Gadget 0   // UAC2_Gadget
 * strs[2] = SUBSYSTEM=u_audio
 * strs[3] = USB_STATE=SET_INTERFACE
 * strs[4] = STREAM_DIRECTION=OUT
 * strs[5] = STREAM_STATE=OFF
 *
 *
 * case 2:
 * the UAC1 uevent when pc/remote play start(play sound of usb open)
 *
 * strs[0] = ACTION=change
 * strs[1] = DEVPATH=/devices/virtual/u_audio/UAC1_Gadget 0
 * strs[2] = SUBSYSTEM=u_audio
 * strs[3] = USB_STATE=SET_INTERFACE
 * strs[4] = STREAM_DIRECTION=OUT
 * strs[5] = STREAM_STATE=ON
 *
 *
 * case 3:
 * the UAC1 uevent when pc/remote capture start(record sound of usb open)
 *
 * strs[0] = ACTION=change
 * strs[1] = DEVPATH=/devices/virtual/u_audio/UAC1_Gadget 0
 * strs[2] = SUBSYSTEM=u_audio
 * strs[3] = USB_STATE=SET_INTERFACE
 * strs[4] = STREAM_DIRECTION=IN
 * strs[5] = STREAM_STATE=ON
 *
 *
 * case 4:
 * the UAC1 uevent when pc/remote capture stop(record sound of usb open)
 *
 * strs[0] = ACTION=change
 * strs[1] = DEVPATH=/devices/virtual/u_audio/UAC1_Gadget 0
 * strs[2] = SUBSYSTEM=u_audio
 * strs[3] = USB_STATE=SET_INTERFACE
 * strs[4] = STREAM_DIRECTION=IN
 * strs[5] = STREAM_STATE=OFF
 *
 *
 * case 5:
 * the UAC1 uevent
 *
 * strs[0] = ACTION=change
 * strs[1] = DEVPATH=/devices/virtual/u_audio/UAC1_Gadget 0
 * strs[2] = SUBSYSTEM=u_audio
 * strs[3] = USB_STATE=SET_SAMPLE_RATE
 * strs[4] = STREAM_DIRECTION=IN
 * strs[5] = SAMPLE_RATE=48000
 */
#define UAC_UEVENT_ADB            "SUBSYSTEM=android_usb"
#define UAC_UEVENT_AUDIO            "SUBSYSTEM=u_audio"
#define UAC_UEVENT_SET_INTERFACE    "USB_STATE=SET_INTERFACE"
#define UAC_UEVENT_SET_SAMPLE_RATE  "USB_STATE=SET_SAMPLE_RATE"
#define UAC_UEVENT_SET_VOLUME       "USB_STATE=SET_VOLUME"
#define UAC_UEVENT_SET_MUTE         "USB_STATE=SET_MUTE"
#define UAC_UEVENT_SET_AUDIO_CLK    "USB_STATE=SET_AUDIO_CLK"

#define UAC_STREAM_DIRECT           "STREAM_DIRECTION="
#define UAC_STREAM_STATE            "STREAM_STATE="
#define UAC_SAMPLE_RATE             "SAMPLE_RATE="
#define UAC_SET_VOLUME              "VOLUME="
#define UAC_SET_MUTE                "MUTE="
#define UAC_PPM                     "PPM="

// remote device/pc->our device
#define UAC_REMOTE_PLAY     "OUT"

// our device->remote device/pc
#define UAC_REMOTE_CAPTURE  "IN"

// sound card is opened
#define UAC_STREAM_START    "ON"

// sound card is closed
#define UAC_STREAM_STOP     "OFF"

bool compare(const char* dst, const char* srt) {
    if ((dst == NULL) || (srt == NULL))
        return false;

    if (!strncmp(dst, srt, strlen(srt))) {
        return true;
    }

    return false;
}

void audio_play(const struct _uevent *uevent) {
    char *direct = uevent->strs[UAC_KEY_DIRECTION];
    char *status = uevent->strs[UAC_KEY_STREAM_STATE];

    if (compare(direct, UAC_STREAM_DIRECT) && compare(status, UAC_STREAM_STATE)) {
        char* device = &direct[strlen(UAC_STREAM_DIRECT)];
        char* state  = &status[strlen(UAC_STREAM_STATE)];
        // remote device/pc open/close usb sound card to write data
        if (compare(device, UAC_REMOTE_PLAY)) {
            if (compare(UAC_STREAM_START, state)) {
                // stream start, we need to open usb card to record datas
                printf("remote device/pc start to play data to us, we need to open usb to capture datas\n");
                uac_role_change(UAC_STREAM_RECORD, true);
                //uac_pbox_notify_host_play_state(true);
            } else if (compare(UAC_STREAM_STOP, state)) {
                printf("remote device/pc stop to play data to us, we need to stop capture datas\n");
                uac_role_change(UAC_STREAM_RECORD, false);
                //uac_pbox_notify_host_play_state(false);
            }
        } else if (compare(device, UAC_REMOTE_CAPTURE)) {
            // our device->remote device/pc
            if (compare(UAC_STREAM_START, state)) {
                // stream start, we need to open usb card to record datas
                printf("remote device/pc start to record from us, we need to open usb to send datas\n");
                uac_role_change(UAC_STREAM_PLAYBACK, true);
                //uac_pbox_notify_host_record_state(true);
            } else if (compare(UAC_STREAM_STOP, state)) {
                printf("remote device/pc stop to record from us, we need to stop write datas to usb\n");
                uac_role_change(UAC_STREAM_PLAYBACK, false);
                //uac_pbox_notify_host_record_state(false);
            }
        }
    }
}

void audio_set_samplerate(const struct _uevent *uevent) {
    char *direct = uevent->strs[UAC_KEY_DIRECTION];
    char *samplerate = uevent->strs[UAC_KEY_SAMPLE_RATE];
    printf("%s: %s\n", __FUNCTION__, direct);
    printf("%s: %s\n", __FUNCTION__, samplerate);
    if (compare(direct, UAC_STREAM_DIRECT)) {
        char* device = &direct[strlen(UAC_STREAM_DIRECT)];
        char* rate  = &samplerate[strlen(UAC_SAMPLE_RATE)];
        int sampleRate = atoi(rate);
        if (compare(device, UAC_REMOTE_PLAY)) {
            printf("set samplerate %d to usb record\n", sampleRate);
            uac_set_sample_rate(UAC_STREAM_RECORD, sampleRate);
        } else if (compare(device, UAC_REMOTE_CAPTURE)) {
            printf("set samplerate %d to usb playback\n", sampleRate);
            uac_set_sample_rate(UAC_STREAM_PLAYBACK, sampleRate);
        }
    }
}

/*
 * strs[0] = ACTION=change
 * strs[1] = DEVPATH=/devicges/virtual/u_audio/UAC1_Gadgeta 0
 * strs[2] = SUBSYSTEM=u_audio
 * strs[3] = USB_STATE=SET_VOLUME
 * strs[4] = STREAM_DIRECTION=OUT
 * strs[5] = VOLUME=0x7FFF
 *    index       db
 *   0x7FFF:   127.9961
 *   ......
 *   0x0100:   1.0000
 *   ......
 *   0x0002:   0.0078
 *   0x0001:   0.0039
 *   0x0000:   0.0000
 *   0xFFFF:  -0.0039
 *   0xFFFE:  -0.0078
 *   ......
 *   0xFE00:  -1.0000
 *   ......
 *   0x8002:  -127.9922
 *   0x8001:  -127.9961
 *
 */
void audio_set_volume(const struct _uevent *uevent) {
    char *direct = uevent->strs[UAC_KEY_DIRECTION];
    char *volumeStr = uevent->strs[UAC_KEY_VOLUME];
    int   unit  = 0x100;
    printf("direct = %s volume = %s\n", direct, volumeStr);
    if (compare(direct, UAC_STREAM_DIRECT)) {
        char* device = &direct[strlen(UAC_STREAM_DIRECT)];
        short volume = 0;
        int volume_t = 0;
        float db     = 0;
        sscanf(volumeStr, "VOLUME=0x%x", &volume_t);
        volume = (short)volume_t;
        db = volume/(float)unit;
        double precent = pow(10, db/10);
        int  precentInt = (int)(precent*100);
        printf("set db = %f, precent = %lf, precentInt = %d\n", db, precent, precentInt);
        if (compare(device, UAC_REMOTE_PLAY)) {
            printf("set volume %d  to usb record\n", precentInt);
            uac_set_volume(UAC_STREAM_RECORD, precentInt);
        } else if (compare(device, UAC_REMOTE_CAPTURE)) {
            printf("set volume %d  to usb playback\n", precentInt);
            uac_set_volume(UAC_STREAM_PLAYBACK, precentInt);
        }
    }
}

/*
 * strs[0] = ACTION=change
 * strs[1] = DEVPATH=/devices/virtual/u_audio/UAC1_Gadget 0
 * strs[2] = SUBSYSTEM=u_audio
 * strs[3] = USB_STATE=SET_MUTE
 * strs[4] = STREAM_DIRECTION=OUT
 * strs[5] = MUTE=1
*/
void audio_set_mute(const struct _uevent *uevent) {
    char *direct = uevent->strs[UAC_KEY_DIRECTION];
    char *muteStr = uevent->strs[UAC_KEY_MUTE];
    printf("direct = %s mute = %s\n", direct, muteStr);

    if (compare(direct, UAC_STREAM_DIRECT)) {
        char* device = &direct[strlen(UAC_STREAM_DIRECT)];
        int mute = 0;
        sscanf(muteStr, "MUTE=%d", &mute);
        if (compare(device, UAC_REMOTE_PLAY)) {
            printf("set mute = %d to usb record\n", mute);
            uac_set_mute(UAC_STREAM_RECORD, mute);
        } else if (compare(device, UAC_REMOTE_CAPTURE)) {
            printf("set mute = %d to usb playback\n", mute);
            uac_set_mute(UAC_STREAM_PLAYBACK, mute);
        }
    }
}

/*
 * strs[0] = ACTION=change
 * strs[1] = DEVPATH=/devices/virtual/u_audio/UAC1_Gadget 0
 * strs[2] = SUBSYSTEM=u_audio
 * strs[3] = USB_STATE=SET_AUDIO_CLK
 * strs[4] = PPM=-21
 * strs[5] = SEQNUM=1573
 */
void audio_set_ppm(const struct _uevent *uevent) {
    char *ppmStr = uevent->strs[UAC_KEY_PPM];

    if (compare(ppmStr, UAC_PPM)) {
        int  ppm = 0;
        sscanf(ppmStr, "PPM=%d", &ppm);
        uac_set_ppm(UAC_STREAM_RECORD, ppm);
        uac_set_ppm(UAC_STREAM_PLAYBACK, ppm);
    }
}


void audio_event(const struct _uevent *uevent) {
    char *event, *direct, *status, *keys;
    if(compare(uevent->strs[UAC_KEY_AUDIO], UAC_UEVENT_ADB)) {
        keys = uevent->strs[UAC_KEY_AUDIO];
        event = uevent->strs[UAC_KEY_USB_STATE];
        direct = uevent->strs[UAC_KEY_DIRECTION];
        status = uevent->strs[UAC_KEY_STREAM_STATE];
        printf("+++++++++++++++++++++++++\n");
        printf("keys = %s\n", keys);
        printf("event = %s\n", event);
        printf("direct = %s\n", direct);
        printf("status = %s\n", status);
        return;
    }

    if (!compare(uevent->strs[UAC_KEY_AUDIO], UAC_UEVENT_AUDIO))
        return;

    keys = uevent->strs[UAC_KEY_AUDIO];
    event = uevent->strs[UAC_KEY_USB_STATE];
    direct = uevent->strs[UAC_KEY_DIRECTION];
    status = uevent->strs[UAC_KEY_STREAM_STATE];
    printf("+++++++++++++++++++++++++\n");
    printf("keys = %s\n", keys);
    printf("event = %s\n", event);
    printf("direct = %s\n", direct);
    printf("status = %s\n", status);
    if ((event == NULL) || (direct == NULL) || (status == NULL)) {
        return;
    }

    bool setInterface = compare(event, UAC_UEVENT_SET_INTERFACE);
    bool setSampleRate = compare(event, UAC_UEVENT_SET_SAMPLE_RATE);
    bool setVolume = compare(event, UAC_UEVENT_SET_VOLUME);
    bool setMute = compare(event, UAC_UEVENT_SET_MUTE);
    bool setClk = compare(event, UAC_UEVENT_SET_AUDIO_CLK);
    if (!setInterface && !setSampleRate && !setVolume && !setMute && !setClk) {
        return;
    }

    if (setInterface) {
        printf("uevent---------------audio_play\n");
        audio_play(uevent);
    } else if(setSampleRate) {
        printf("uevent---------------audio_set_samplerate\n");
        audio_set_samplerate(uevent);
    } else if(setVolume) {
        printf("uevent---------------setVolume\n");
        audio_set_volume(uevent);
    } else if(setMute) {
        printf("uevent---------------setMute\n");
        audio_set_mute(uevent);
    }  else if(setClk) {
        printf("uevent---------------setClk\n");
        audio_set_ppm(uevent);
    }
}