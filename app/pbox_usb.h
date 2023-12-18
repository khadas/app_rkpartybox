#ifndef _PBOX_USB_H_
#define _PBOX_USB_H_
#include <stdbool.h>
#include <stdint.h>
#include "pbox_common.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MUSIC_FILE_MP3,
    MUSIC_FILE_WAV,
    MUSIC_FILE_WMA,
    MUSIC_FILE_AAC,
    MUSIC_FILE_FLAC,
} music_format_t;

typedef enum {
    USB_CONNECTED,
    USB_SCANNING,
    USB_DISCONNECTED,
} usb_state_t;

typedef struct {
    usb_state_t usbState;
    char usbDiskName[MAX_APP_NAME_LENGTH+1];//reserved
} usb_disk_info_t;

typedef struct {
    music_format_t format;//reserved
    char fileName[MAX_APP_NAME_LENGTH+1];
} usb_music_file_t;

typedef enum {
    //command
    // may be usb part no command need.

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

int pbox_create_usb_task(void);
#ifdef __cplusplus
}
#endif
#endif