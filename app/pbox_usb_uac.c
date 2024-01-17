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
#include <mntent.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <linux/netlink.h>

#include "uac_uevent.h"
#include "uac_control.h"
#include "pbox_common.h"
#include "pbox_socket.h"
#include "pbox_usb_uac.h"
#include "pbox_usb.h"

void uacPboxRoleChange(uint32_t role, bool start) {
    uac_pbox_notify_role_change(role, start);
}

void uacPboxSetSampleRate(uint32_t role, uint32_t rate) {
    uac_pbox_notify_host_sample_rate(role, rate);
}

void uacPboxSetVolume(uint32_t role, uint32_t volume) {
    uac_pbox_notify_host_volume(role, volume);
}

void uacPboxSetMute(uint32_t role, bool mute) {
    uac_pbox_notify_host_mute(role, mute);
}

void uacPboxSetPpm(uint32_t role, int ppm) {
    uac_pbox_notify_host_ppm(role, ppm);
}

struct UACControl uac = {
    .uacRoleChange = uacPboxRoleChange,
    .uacSetSampleRate = uacPboxSetSampleRate,
    .uacSetVolume = uacPboxSetVolume,
    .uacSetMute = uacPboxSetMute,
    .uacSetPpm = uacPboxSetPpm,
};

void parse_event(const struct _uevent *event) {
    if (event->size <= 0)
        return;

#if 0
    printf("uac event start++++++++++++++++++++:\n");
    for (int i = 0 ; i < 10; i++) {
        if (event->strs[i] != NULL) {
            printf("strs[%d] = %s\n", i, event->strs[i]);
        }
    }
    printf("uac event -------------------------\n");
#endif

    {
        audio_event(event);
    }
}

void uac_init(void) {
    uac_control_create(&uac);
    exec_command_system("/etc/init.d/S50usbdevice restart");
}

int uac_monitor_get_fd(void) {
    struct sockaddr_nl sa;
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_groups = NETLINK_KOBJECT_UEVENT;
    sa.nl_pid = 0;

    int sockfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);

    if (sockfd == -1) {
        printf(("%s socket creating failed:%s\n", __func__, strerror(errno)));
        return sockfd;
    }

    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        printf("bind error:%s\n", strerror(errno));
        return sockfd;
    }

    return sockfd;
}
