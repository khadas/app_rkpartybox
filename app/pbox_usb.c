#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include "pbox_common.h"
#include "pbox_usb.h"
#include "pbox_usb_scan.h"
#include "pbox_socket.h"

static void handleUsbStartScanCmd(const pbox_usb_msg_t* msg);

typedef void (*usb_cmd_handle_t)(const pbox_usb_msg_t*);

typedef struct {
    pbox_usb_opcode_t opcode;
    usb_cmd_handle_t handler;
} UsbCmdHandler_t;

usb_state_t usb_server_state = USB_DISCONNECTED;

int unix_socket_usb_notify(void *info, int length) {
    return unix_socket_notify_msg(PBOX_MAIN_USBDISK, info, length);
}

void usb_pbox_notify_state_changed(usb_state_t state, char *diskName) {
    pbox_usb_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_USB_DISK_CHANGE_EVT,
    };
    msg.usbDiskInfo.usbState = state;
    if(diskName)
    strncpy(msg.usbDiskInfo.usbDiskName, diskName, MAX_APP_NAME_LENGTH);
    msg.usbDiskInfo.usbDiskName[MAX_APP_NAME_LENGTH] = 0;

    unix_socket_usb_notify(&msg, sizeof(pbox_usb_msg_t));
}

void usb_pbox_notify_audio_file_added(music_format_t format, char *fileName) {
    pbox_usb_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_USB_AUDIO_FILE_ADD_EVT,
    };
    msg.usbMusicFile.format = format;
    strncpy(msg.usbMusicFile.fileName, fileName, MAX_APP_NAME_LENGTH);
    msg.usbMusicFile.fileName[MAX_APP_NAME_LENGTH] = 0;
    printf("%s format:%d, name:%s\n", __func__, format, fileName);
    unix_socket_usb_notify(&msg, sizeof(pbox_usb_msg_t));
}

const UsbCmdHandler_t usb_event_handlers[] = {
    { PBOX_USB_START_SCAN, handleUsbStartScanCmd },
};

// Function to handle the loop mode command
void handleUsbStartScanCmd(const pbox_usb_msg_t* msg) {
    //bool loop = msg->loop;
    printf("%s\n", __func__);
    usb_pbox_notify_state_changed(USB_SCANNING, "USB_DISK");
    uint64_t time = time_get_os_boot_ms();
    scan_dir(MUSIC_PATH, 3, usb_pbox_notify_audio_file_added);
    usb_pbox_notify_state_changed(USB_SCANNED, "USB_DISK");
    time = time_get_os_boot_ms() - time;
    printf("%s scan finish, used time:%d\n", __func__, time/1000);
}

// Function to process an incoming pbox_usb_msg_t event
void process_pbox_usb_cmd(const pbox_usb_msg_t* msg) {
    if (msg == NULL) {
        printf("Error: Null event message received.\n");
        return;
    }

    // Iterate over the LcdCmdHandlers array
    for (int i = 0; i < sizeof(usb_event_handlers)/sizeof(usb_event_handlers[0]); i++) {
        if (usb_event_handlers[i].opcode == msg->msgId) {
            if (usb_event_handlers[i].handler != NULL) {
                usb_event_handlers[i].handler(msg);
            }
            return;
        }
    }

    printf("Warning: No handler found for event ID %d.\n", msg->msgId);
}

pthread_t usb_server_task_id;
static void *pbox_usb_server(void *arg)
{
    char buff[sizeof(pbox_usb_msg_t)] = {0};
    pbox_usb_msg_t *msg;
    pthread_setname_np(pthread_self(), "pbox_usb");

    int sock_fd = create_udp_socket(SOCKET_PATH_USB_SERVER);
    if (sock_fd < 0) {
        perror("Failed to create UDP socket");
        return (void *)-1;
    }
/*
    #define USB_DETECT_PATH "path_to_usb_detect_kernel_file"
    int usb_detect_fd = open(USB_DETECT_PATH, O_RDONLY);
*/
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sock_fd, &read_fds);

    while(true) {
        fd_set read_set = read_fds;

        int result = select(sock_fd+1, &read_set, NULL, NULL, NULL);
        if (result < 0) {
            if (errno != EINTR) {
                perror("select failed");
                break;
            }
            continue; // Interrupted by signal, restart select
        } else if (result == 0) {
            printf("select timeout or no data\n");
            continue;
        }

        int ret = recvfrom(sock_fd, buff, sizeof(buff), 0, NULL, NULL);
        if (ret <= 0) {
            if (ret == 0) {
                printf("Socket closed\n");
                break;
            } else {
                perror("recvfrom failed");
                continue;
            }
        }

        pbox_usb_msg_t *msg = (pbox_usb_msg_t *)buff;
        if(msg->type == PBOX_EVT)
            continue;

        process_pbox_usb_cmd(msg);
        //process and scan new usb disk insert event.
        //adding code.
    }
}

int pbox_create_usb_task(void) {
    int ret;

    ret = pthread_create(&usb_server_task_id, NULL, pbox_usb_server, NULL);
    if (ret < 0)
    {
        printf("usb server start failed\n");
    }

    return ret;
}

