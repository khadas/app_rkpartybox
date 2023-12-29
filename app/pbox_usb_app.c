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
#include <sys/socket.h>
#include "pbox_common.h"
#include "pbox_usb.h"
#include "pbox_app.h"
#include "pbox_socket.h"
#include "pbox_socketpair.h"
#include "pbox_btsink_app.h"

typedef void (*usb_event_handle)(const pbox_usb_msg_t*);
static void handleUsbChangeEvent(const pbox_usb_msg_t* msg);
static void handleUsbAudioFileAddEvent(const pbox_usb_msg_t* msg);

int unix_socket_usb_send(void *info, int length)
{
	return unix_socket_send_cmd(PBOX_CHILD_USBDISK, info, length);
}

void pbox_app_usb_startScan(void) {
    pbox_usb_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_USB_START_SCAN,
    };
    printf("%s\n", __func__);
    unix_socket_usb_send(&msg, sizeof(pbox_usb_msg_t));
}

void pbox_app_usb_pollState(void) {
    pbox_usb_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_USB_POLL_STATE,
    };
    printf("%s\n", __func__);
    unix_socket_usb_send(&msg, sizeof(pbox_usb_msg_t));
}

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

char* pbox_app_usb_get_title(uint32_t trackId) {//(pboxTrackdata->track_id)
    if(trackId < pboxTrackdata->track_num)
        return pboxTrackdata->track_list[trackId].title;
    return NULL;
}

#define pbox_free(a) do { if(a) {free(a); a = NULL;}} while(0)

void handleUsbChangeEvent(const pbox_usb_msg_t* msg) {
    usb_state_t usbDiskState= msg->usbDiskInfo.usbState;
    if ( pboxUsbdata->usbState == usbDiskState) {
        return;
    }

    pboxUsbdata->usbState = usbDiskState;
    switch(usbDiskState) {
        case USB_DISCONNECTED: {
            for (int i = 0; i < pboxTrackdata->track_num; i++) {
                pbox_free(pboxTrackdata->track_list[i].title);
                pbox_free(pboxTrackdata->track_list[i].artist);
                pboxTrackdata->track_list[i].duration = 0;
            }
            pboxTrackdata->track_num = 0;
            pboxTrackdata->track_id = 0;
            if (getBtSinkState() != BT_CONNECTED)
                pbox_app_music_stop(DISP_All);
        } break;

        case USB_CONNECTED: {
            pbox_app_usb_startScan();
            strncpy(&pboxUsbdata->usbDiskName[0], msg->usbDiskInfo.usbDiskName, MAX_APP_NAME_LENGTH);
            pboxUsbdata->usbDiskName[MAX_APP_NAME_LENGTH] = 0;
            printf("%s usbState: %d, usb name[%s]\n", __func__, usbDiskState, pboxUsbdata->usbDiskName);
        } break;

        case USB_SCANNED: {

        } break;

        case USB_SCANNING: {
            printf("%s USB_SCANNING\n", __func__);
            for (int i = 0; i < pboxTrackdata->track_num; i++) {
                pbox_free(pboxTrackdata->track_list[i].title);
                pbox_free(pboxTrackdata->track_list[i].artist);
                pboxTrackdata->track_list[i].duration = 0;
            }
            pboxTrackdata->track_num = 0;
            pboxTrackdata->track_id = 0;
        } break;
    }
    pbox_multi_displayUsbState(usbDiskState, DISP_All);
}

void handleUsbAudioFileAddEvent(const pbox_usb_msg_t* msg) {
    usb_music_file_t usbMusicFile = msg->usbMusicFile;

    char **pTitle = &(pboxTrackdata->track_list[pboxTrackdata->track_num].title);
    int len = strlen(msg->usbMusicFile.fileName);
    *pTitle = malloc(len + 1);
    if(*pTitle) {
        strncpy(*pTitle, msg->usbMusicFile.fileName, len);
        (*pTitle)[len] = 0;
        printf("adding[%d]:%s, len=%d\n", 
                pboxTrackdata->track_num, pboxTrackdata->track_list[pboxTrackdata->track_num].title, len);
    }

    if (pboxTrackdata->track_num < TRACK_MAX_NUM)
        pboxTrackdata->track_num++;
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
#if ENABLE_UDP_CONNECTION_LESS
    int ret = recvfrom(fd, buff, sizeof(buff), 0, NULL, NULL);
#else
    int ret = recv(fd, buff, sizeof(buff), 0);
#endif
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
