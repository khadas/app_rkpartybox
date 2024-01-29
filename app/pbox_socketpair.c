#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "pbox_common.h"
#include "pbox_usb.h"
#include "pbox_keyscan_app.h"
#include "pbox_lvgl.h"
#include "pbox_rockit.h"
#include "rk_btsink.h"
#include "pbox_btsink_app.h"
#include "pbox_light_effect.h"

int get_client_socketpair_fd(uint32_t source)
{
    return pbox_pipe_fds[source].fd[0];
}

int get_server_socketpair_fd(uint32_t source)
{
    return pbox_pipe_fds[source].fd[1];
}

int unix_socket_notify_msg(pb_module_main_t module, void *info, int length)
{
    int sockfd;
    switch (module) { 
        #if ENABLE_LCD_DISPLAY
        case PBOX_MAIN_LVGL: {
            sockfd = get_server_socketpair_fd(PBOX_SOCKPAIR_LVGL);
        } break;
        #endif
        case PBOX_MAIN_BT: {
            sockfd = get_server_socketpair_fd(PBOX_SOCKPAIR_BT);
        } break;
        case PBOX_MAIN_ROCKIT: {
            sockfd = get_server_socketpair_fd(PBOX_SOCKPAIR_ROCKIT);
        } break;
        case PBOX_MAIN_KEYSCAN: {
            sockfd = get_server_socketpair_fd(PBOX_SOCKPAIR_KEYSCAN);
        } break;
        case PBOX_MAIN_USBDISK: {
            sockfd = get_server_socketpair_fd(PBOX_SOCKPAIR_USBDISK);
        } break;

        default: {
        printf("%s module:%d", __func__, module);
        return -1;
        }
    }

    int ret = send(sockfd, info, length, 0);//sendto(sockfd, info, length, MSG_DONTWAIT, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        int id = -1;
        switch(module) {
            #if ENABLE_LCD_DISPLAY
            case PBOX_MAIN_LVGL:
                id = ((pbox_lcd_msg_t*)info)->msgId;
                break;
            #endif
            case PBOX_MAIN_BT:
                id = ((pbox_bt_msg_t*)info)->msgId;
                break;
            case PBOX_MAIN_ROCKIT:
                id = ((pbox_rockit_msg_t*)info)->msgId;
                break;
            case PBOX_MAIN_USBDISK:
                id = ((pbox_usb_msg_t*)info)->msgId;
                break;
            case PBOX_MAIN_KEYSCAN:
                id = ((pbox_keyevent_msg_t*)info)->key_code;
                break;
        }
        printf("%s: Socket send failed!  source = %d, id:%d, ret = %d, errno: %d\n", __func__, module, id, ret, errno);
        return -1;
    }

    return 0;
}

//cmd from maintask to children task.
int unix_socket_send_cmd(pb_module_child_t module, void *info, int length)
{
    int sockfd;
    switch (module) {
        #if ENABLE_LCD_DISPLAY
        case PBOX_CHILD_LVGL: {
            sockfd = get_client_socketpair_fd(PBOX_SOCKPAIR_LVGL);
        } break;
        #endif
        case PBOX_CHILD_BT: {
            sockfd = get_client_socketpair_fd(PBOX_SOCKPAIR_BT);
        } break;
        case PBOX_CHILD_ROCKIT: {
            sockfd = get_client_socketpair_fd(PBOX_SOCKPAIR_ROCKIT);
        } break;
        case PBOX_CHILD_LED: {
            sockfd = get_client_socketpair_fd(PBOX_SOCKPAIR_LED);
        } break;
        case PBOX_CHILD_USBDISK: {
            sockfd = get_client_socketpair_fd(PBOX_SOCKPAIR_USBDISK);
        } break;

        default: {
        printf("%s module:%d", __func__, module);
        return -1;
        }
    }

    int ret = send(sockfd, info, length, 0);//sendto(sockfd, info, length, MSG_DONTWAIT, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        int id = -1;
        switch(module) {
#if ENABLE_LCD_DISPLAY
            case PBOX_CHILD_LVGL:
                id = ((pbox_lcd_msg_t*)info)->msgId;
                break;
#endif
            case PBOX_CHILD_BT:
                id = ((pbox_bt_msg_t*)info)->msgId;
                break;
            case PBOX_CHILD_ROCKIT:
                id = ((pbox_rockit_msg_t*)info)->msgId;
                break;
            case PBOX_CHILD_LED:
                id = ((pbox_light_effect_msg_t*)info)->msgId;
                break;
            case PBOX_CHILD_USBDISK:
                id = ((pbox_usb_msg_t*)info)->msgId;
                break;
        }
        printf("%s: module:%d, id:%d, Socket send failed! ret = %d\n", __func__, module, id, ret);
        return -1;
    }

    return ret;
}
