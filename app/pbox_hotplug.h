#ifndef _PBOX_HOTPLUG_H_
#define _PBOX_HOTPLUG_H_
#include <stdbool.h>
#include <stdint.h>
#include "pbox_common.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    //command
    PBOX_USB_POLL_STATE,
    PBOX_USB_START_SCAN,
    PBOX_UAC_RESTART,

    //event
    PBOX_USB_DISK_CHANGE_EVT = 0x100,
    PBOX_USB_AUDIO_FILE_ADD_EVT,
    PBOX_USB_UAC_ROLE_CHANGE_EVT,
    PBOX_USB_UAC_SAMPLE_RATE_EVT,
    PBOX_USB_UAC_VOLUME_EVT,
    PBOX_USB_UAC_MUTE_EVT,
    PBOX_USB_UAC_PPM_EVT,
} pbox_usb_opcode_t;

typedef struct {
    pbox_msg_t type;
    pbox_usb_opcode_t msgId;
    union {
        usb_disk_info_t usbDiskInfo;
        usb_music_file_t usbMusicFile;
        uac_t uac;
    };
} pbox_usb_msg_t;

void uac_pbox_notify_role_change(uint32_t role, bool start);
void uac_pbox_notify_host_sample_rate(uint32_t role, uint32_t rate);
void uac_pbox_notify_host_volume(uint32_t role, uint32_t volume);
void uac_pbox_notify_host_mute(uint32_t role, bool on);
void uac_pbox_notify_host_ppm(uint32_t role, int32_t ppm);

void usb_pbox_notify_audio_file_added(music_format_t format, char *fileName);
int pbox_create_hotplug_dev_task(void);
#ifdef __cplusplus
}
#endif
#endif
