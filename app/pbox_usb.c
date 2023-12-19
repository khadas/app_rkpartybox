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

int unix_socket_usb_notify(void *info, int length) {
    return unix_socket_notify_msg(PBOX_MAIN_USBDISK, info, length);
}

void usb_pbox_notify_state_changed(usb_state_t state, char *diskName) {
    pbox_usb_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_USB_DISK_CHANGE_EVT,
    };
    msg.usbDiskInfo.usbState = state;
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

    unix_socket_usb_notify(&msg, sizeof(pbox_usb_msg_t));
}

pthread_t usb_server_task_id;
static void *pbox_usb_server(void *arg)
{
    char buff[sizeof(pbox_usb_msg_t)] = {0};
    pbox_usb_msg_t *msg;
    pthread_setname_np(pthread_self(), "pbox_usb");

    #if 0 //may no need to create server for recv data from main task.
    int sock_fd = create_udp_socket(SOCKET_PATH_LVGL_SERVER);
    if (sock_fd < 0) {
        perror("Failed to create UDP socket");
        return (void *)-1;
    }
    #endif

    #define USB_DETECT_PATH "path_to_usb_detect_kernel_file"
    int usb_detect_fd = open(USB_DETECT_PATH, O_RDONLY);
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(usb_detect_fd, &read_fds);

    while(true) {
        fd_set read_set = read_fds;

        int result = select(usb_detect_fd+1, &read_set, NULL, NULL, NULL);
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

        //process and scan new usb disk insert event.
        //adding code.
    }
}

int pbox_create_usb_task(void) {
    int ret;

    //ret = pthread_create(&usb_server_task_id, NULL, pbox_usb_server, NULL);
    if (ret < 0)
    {
        printf("usb server start failed\n");
    }

    return ret;
}

