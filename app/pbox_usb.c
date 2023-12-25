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
#include <mntent.h>
#include <libudev.h>
#include "pbox_common.h"
#include "pbox_usb.h"
#include "pbox_usb_scan.h"
#include "pbox_socket.h"

static void handleUsbStartScanCmd(const pbox_usb_msg_t* msg);
static void handleUsbPollStateCmd(const pbox_usb_msg_t* msg);

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

bool is_device_mounted(const char *dev_path) {
    FILE *mnt_file;
    struct mntent *mnt;
    bool mounted = false;

    mnt_file = setmntent("/proc/mounts", "r");
    if (mnt_file == NULL) {
        return false;
    }

    while ((mnt = getmntent(mnt_file)) != NULL) {
        if (strcmp(mnt->mnt_fsname, dev_path) == 0) {
            mounted = true;
            break;
        }
    }

    endmntent(mnt_file);
    return mounted;
}

bool is_usb_drive_connected() {
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *entry;
    bool is_connected = false;

    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "cann't create udev\n");
        exit(EXIT_FAILURE);
    }

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(entry, devices) {
        const char *path;
        struct udev_device *dev, *parent;

        path = udev_list_entry_get_name(entry);
        dev = udev_device_new_from_syspath(udev, path);
        parent = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

        if (parent != NULL) {
            const char *devtype = udev_device_get_devtype(dev);
            if ((strcmp(devtype, "disk") == 0 || strcmp(devtype, "partition") == 0) &&
                is_device_mounted(udev_device_get_devnode(dev))) {
                is_connected = true;
                udev_device_unref(dev);
                break;
            }
        }
        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    printf("%s time:%u, is_connected:%d\n", __func__, time(NULL), is_connected);
    return is_connected;
}

const UsbCmdHandler_t usb_event_handlers[] = {
    { PBOX_USB_POLL_STATE, handleUsbPollStateCmd},
    { PBOX_USB_START_SCAN, handleUsbStartScanCmd},
};

void handleUsbStartScanCmd(const pbox_usb_msg_t* msg) {
    printf("%s\n", __func__);
    printf("%s: connected:%d\n", __func__, is_usb_drive_connected());
    if(!is_usb_drive_connected()) {
        usb_pbox_notify_state_changed(USB_DISCONNECTED, NULL);
        return;
    }
    usb_pbox_notify_state_changed(USB_SCANNING, MUSIC_PATH);
    uint64_t time = time_get_os_boot_ms();

    scan_dir(MUSIC_PATH, 3, usb_pbox_notify_audio_file_added);
    usb_pbox_notify_state_changed(USB_SCANNED, MUSIC_PATH);
    time = time_get_os_boot_ms() - time;
    printf("%s scan finish, used time:%d\n", __func__, time/1000);
}

void handleUsbPollStateCmd(const pbox_usb_msg_t* msg) {
    printf("%s: connected:%d\n", __func__, is_usb_drive_connected());
    if(!is_usb_drive_connected()) {
        usb_pbox_notify_state_changed(USB_DISCONNECTED, NULL);
        return;
    }
    usb_pbox_notify_state_changed(USB_CONNECTED, MUSIC_PATH);
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

#define USB_UDP_SOCKET 0
#define USB_DEV_DETECT 1
#define USB_FD_NUM     2

pthread_t usb_server_task_id;
static void *pbox_usb_server(void *arg)
{
    int usb_fds[USB_FD_NUM] = {-1, -1};
    char buff[sizeof(pbox_usb_msg_t)] = {0};
    pbox_usb_msg_t *msg;
    struct udev *udev;

    pthread_setname_np(pthread_self(), "pbox_usb");

    usb_fds[USB_UDP_SOCKET] = create_udp_socket(SOCKET_PATH_USB_SERVER);
    if (usb_fds[USB_UDP_SOCKET] < 0) {
        perror("Failed to create UDP socket");
        return (void *)-1;
    }

    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "cann't create udev\n");
        return (void *)-1;
    }

    struct udev_monitor *mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", "usb_device");
    udev_monitor_enable_receiving(mon);
    usb_fds[USB_DEV_DETECT] = udev_monitor_get_fd(mon);

    int max_fd = (usb_fds[0] > usb_fds[1]) ? usb_fds[0] : usb_fds[1];

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(usb_fds[0], &read_fds);
    FD_SET(usb_fds[1], &read_fds);
    struct timeval tv = {
        .tv_sec = 1,
        .tv_usec = 0,
    };
    uint32_t pollRetry = 0;
    bool isUsbInsert = 0, isConnectReported = 0;

    while(true) {
        fd_set read_set = read_fds;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int result = select(max_fd+1, &read_set, NULL, NULL, &tv);
        if (result < 0) {
            if (errno != EINTR) {
                perror("select failed\n");
                break;
            }
            continue; // Interrupted by signal, restart select
        } else if (result == 0) {
            if(isUsbInsert && !isConnectReported && (pollRetry < 10)) {
                if(is_usb_drive_connected()) {
                    usb_pbox_notify_state_changed(USB_CONNECTED, MUSIC_PATH);
                    isConnectReported = true;
                }
            }
            //printf("select timeout or no data\n");
            continue;
        }

        //printf("%s result:%d\n", __func__, result);
        for (int i = 0, ret =-1; i < USB_FD_NUM; i++) {
            if((ret = FD_ISSET(usb_fds[i], &read_set)) == 0)
                continue;
            switch (i) {
                case USB_UDP_SOCKET: {
                    int ret = recvfrom(usb_fds[USB_UDP_SOCKET], buff, sizeof(buff), 0, NULL, NULL);
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
                } break;

                case USB_DEV_DETECT: {
                    struct udev_device *dev = udev_monitor_receive_device(mon);
                    const char *action = udev_device_get_action(dev);
                    const char *devnode = udev_device_get_devnode(dev);
                    printf("%s dev found: %s, action： %s\n", __func__, devnode, action);

                    if(!action) {
                        break;
                    }
                    if (strcmp(action, "remove") == 0) {
                        printf("Device removed：%s\n", devnode);
                        isConnectReported = false;
                        isUsbInsert = false;
                        pollRetry = 0;
                        usb_pbox_notify_state_changed(USB_DISCONNECTED, NULL);
                    } else if (strcmp(action, "add") == 0) {
                        isUsbInsert = true;
                        isConnectReported = false;
                        pollRetry = 0;
                        //usb_pbox_notify_state_changed(USB_CONNECTED, MUSIC_PATH);
                        printf("Device added：%s\n", devnode);
                    }
                    udev_device_unref(dev);
                } break;
            }
        }
    }

    udev_monitor_unref(mon);
    udev_unref(udev);
    close(usb_fds[USB_UDP_SOCKET]);
    close(usb_fds[USB_DEV_DETECT]);
    return NULL;
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

