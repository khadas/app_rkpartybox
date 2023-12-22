#ifndef _PBOX_USB_H_
#define _PBOX_USB_H_
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

    //event
    PBOX_USB_DISK_CHANGE_EVT = 0x100,
    PBOX_USB_AUDIO_FILE_ADD_EVT,
} pbox_usb_opcode_t;

typedef struct {
    pbox_msg_t type;
    pbox_usb_opcode_t msgId;
    union {
        usb_disk_info_t usbDiskInfo;
        usb_music_file_t usbMusicFile;
    };
} pbox_usb_msg_t;

void usb_pbox_notify_audio_file_added(music_format_t format, char *fileName);
int pbox_create_usb_task(void);
#ifdef __cplusplus
}
#endif
#endif