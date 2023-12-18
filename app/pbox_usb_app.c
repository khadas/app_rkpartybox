#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/un.h>
#include "pbox_common.h"
#include "pbox_usb.h"

typedef void (*usb_event_handle)(const pbox_usb_msg_t*);
static void handleUsbChangeEvent(const pbox_usb_msg_t* msg);
static void handleUsbAudioFileAddEvent(const pbox_usb_msg_t* msg);

// Define a struct to associate opcodes with handles
typedef struct {
    pbox_usb_opcode_t opcode;
    usb_event_handle handle;
} usb_event_handle_t;

const usb_event_handle_t usbEventTable[] = {
    { PBOX_USB_DISK_CHANGE_EVT, handleUsbChangeEvent },
    { PBOX_USB_AUDIO_FILE_ADD_EVT, handleUsbAudioFileAddEvent },
    // Add other as needed...
};

void handleUsbChangeEvent(const pbox_usb_msg_t* msg) {
    usb_disk_info_t usbDiskState;
    usbDiskState.usbState = msg->usbDiskInfo.usbState;
    strncpy(usbDiskState.usbDiskName, msg->usbDiskInfo.usbDiskName, MAX_APP_NAME_LENGTH);

    printf("%s usbState: %d, file name[%s]\n", __func__, usbDiskState.usbState, usbDiskState.usbDiskName);
}

void handleUsbAudioFileAddEvent(const pbox_usb_msg_t* msg) {
    usb_music_file_t usbMusicFile;
    usbMusicFile.format = msg->usbMusicFile.format;
    strncpy(usbMusicFile.fileName, msg->usbMusicFile.fileName, MAX_APP_NAME_LENGTH);

    printf("%s format: %d, file name[%s]\n", __func__, usbMusicFile.format, usbMusicFile.fileName);
}

// Function to process an incoming pbox_usb_msg_t event
void maintask_usb_data_recv(const pbox_usb_msg_t* msg) {
    if (msg == NULL) {
        printf("Error: Null event message received.\n");
        return;
    }

    // Iterate over the usbEventTable array
    for (int i = 0; i < sizeof(usbEventTable) / sizeof(usbEventTable[0]); ++i) {
        if (usbEventTable[i].opcode == msg->msgId) {
            // Call the corresponding event handler
            if (usbEventTable[i].handle != NULL) {
                usbEventTable[i].handle(msg);
            }
            return; // Exit after handling the event
        }
    }

    printf("Warning: No handle found for event ID %d.\n", msg->msgId);
}

void maintask_usb_fd_process(int fd) {
    char buff[sizeof(pbox_usb_msg_t)] = {0};

    int ret = recvfrom(fd, buff, sizeof(buff), 0, NULL, NULL);
    if (ret <= 0) {
        if (ret == 0) {
            printf("%s: Connection closed\n", __func__);
        } else if (errno != EINTR) {
            perror("recvfrom");
        }
        return;
    }

    pbox_usb_msg_t *msg = (pbox_usb_msg_t *)buff;
    printf("%s: Socket received - type: %d, id: %d\n", __func__, msg->type, msg->msgId);

    if (msg->type != PBOX_EVT) {
        printf("%s: Invalid message type\n", __func__);
        return;
    }

    maintask_usb_data_recv(msg);
}