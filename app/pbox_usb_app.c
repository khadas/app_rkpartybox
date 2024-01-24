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
static void handleUacRoleStateEvent(const pbox_usb_msg_t* msg);
static void handleUacSampleRateEvent(const pbox_usb_msg_t* msg);
static void handleUacVolumeEvent(const pbox_usb_msg_t* msg);
static void handleUacMuteEvent(const pbox_usb_msg_t* msg);
static void handleUsbPpmEvent(const pbox_usb_msg_t* msg);

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

void pbox_app_uac_restart(void) {
    pbox_usb_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_UAC_RESTART,
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
    { PBOX_USB_DISK_CHANGE_EVT,     handleUsbChangeEvent        },
    { PBOX_USB_AUDIO_FILE_ADD_EVT,  handleUsbAudioFileAddEvent  },

    #if ENABLE_UAC
    { PBOX_USB_UAC_ROLE_CHANGE_EVT, handleUacRoleStateEvent     },
    { PBOX_USB_UAC_SAMPLE_RATE_EVT, handleUacSampleRateEvent    },
    { PBOX_USB_UAC_VOLUME_EVT,      handleUacVolumeEvent        },
    { PBOX_USB_UAC_MUTE_EVT,        handleUacMuteEvent          },
    { PBOX_USB_UAC_PPM_EVT,         handleUsbPpmEvent           },
    #endif
    // Add other as needed...
};

void handleUacRoleStateEvent(const pbox_usb_msg_t* msg) {
    uac_role_t role = msg->uac.uac_role;
    bool start = msg->uac.state;
    pbox_app_uac_state_change(role, start, DISP_All);
}

void handleUacSampleRateEvent(const pbox_usb_msg_t* msg) {
    uac_role_t role = msg->uac.uac_role;
    uint32_t freq = msg->uac.sampleFreq;
    pbox_app_uac_freq_change(role, freq, DISP_All);
}

void handleUacVolumeEvent(const pbox_usb_msg_t* msg) {
    uac_role_t role = msg->uac.uac_role;
    uint32_t volume = msg->uac.volume;
    pbox_app_uac_volume_change(role, volume, DISP_All);
}

void handleUacMuteEvent(const pbox_usb_msg_t* msg) {
    uac_role_t role = msg->uac.uac_role;
    bool mute = msg->uac.mute;
    pbox_app_uac_mute_change(role, mute, DISP_All);
}

void handleUsbPpmEvent(const pbox_usb_msg_t* msg) {
    uac_role_t role = msg->uac.uac_role;
    int32_t ppm = msg->uac.ppm;
    pbox_app_uac_ppm_change(role, ppm, DISP_All);
}

char* pbox_app_usb_get_title(uint32_t trackId) {
    if(trackId < pboxTrackdata->track_num)
        return pboxTrackdata->track_list[trackId].title;
    return NULL;
}

#define pbox_free(a) do { if(a) {free(a); a = NULL;}} while(0)

bool isUsbDiskConnected(void) {
    return (pboxUsbdata->usbState != USB_DISCONNECTED);
}

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

            pbox_app_usb_state_change(usbDiskState, DISP_All);
            pbox_app_usb_list_update(pboxTrackdata->track_id, DISP_All);
            pbox_app_switch_next_input_source(SRC_USB, DISP_All);
        } break;

        case USB_CONNECTED: {
            pbox_app_usb_start_scan(DISP_All);
            strncpy(&pboxUsbdata->usbDiskName[0], msg->usbDiskInfo.usbDiskName, MAX_APP_NAME_LENGTH);
            pboxUsbdata->usbDiskName[MAX_APP_NAME_LENGTH] = 0;
            printf("%s usbState: %d, usb name[%s]\n", __func__, usbDiskState, pboxUsbdata->usbDiskName);
            pbox_app_usb_state_change(usbDiskState, DISP_All);
        } break;

        case USB_SCANNED: {
            pbox_app_usb_list_update(pboxTrackdata->track_id, DISP_All);
            if(!isBtConnected()&&isUsbDiskConnected())
                pbox_app_switch_to_input_source(SRC_USB, DISP_All);
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
    int ret = recv(fd, buff, sizeof(buff), 0);

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
