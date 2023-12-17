#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "pbox_common.h"

#define PRINT_FLAG_ERR "[RK_SKT_ERROR]"
#define PRINT_FLAG_SUCESS "[RK_SKT_SUCESS]"

int create_udp_socket(char *socket_path)
{
    if ((socket_path == NULL) || (strlen(socket_path) == 0))
        return -1;
    printf("----- rkbtsink_server -----\n");

    unlink(socket_path);
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("%s: Create socket failed!\n", __func__);
        return -1;
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);
    int ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        printf("%s: Bind Local addr failed!\n", __func__);
        close(sockfd);
        return -1;
    }

    struct timeval t = {0, 100*1000};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&t, sizeof(t));

    return sockfd;
}

int unix_socket_notify_msg(pb_module_main_t module, void *info, int length)
{
    struct sockaddr_un serverAddr;
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("%s create sockfd failed!\n", __func__);
        return -1;
    }

    serverAddr.sun_family = AF_UNIX;
    switch (module) { 
        case PBOX_MAIN_LVGL: {
            strcpy(serverAddr.sun_path, SOCKET_PATH_LVGL_CLINET);
        } break;
        case PBOX_MAIN_BT: {
            strcpy(serverAddr.sun_path, SOCKET_PATH_BTSINK_CLIENT);
        } break;
        case PBOX_MAIN_ROCKIT: {
            strcpy(serverAddr.sun_path, SOCKET_PATH_ROCKIT_CLINET);
        } break;
        case PBOX_MAIN_KEYSCAN: {
            strcpy(serverAddr.sun_path, SOCKET_PATH_KEY_SCAN_CLINET);
        } break;
    }

    int ret = sendto(sockfd, info, length, MSG_DONTWAIT, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        printf("%s: Socket send failed!  source = %d, ret = %d, errno: %d\n", __func__, module, ret, errno);
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}

//cmd from maintask to children task.
int unix_socket_send_cmd(pb_module_child_t module, void *info, int length)
{
    struct sockaddr_un serverAddr;
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("FUNC:%s create sockfd failed!\n", __func__);
        return -1;
    }

    serverAddr.sun_family = AF_UNIX;
    switch (module) { 
        case PBOX_CHILD_LVGL: {
            strcpy(serverAddr.sun_path, SOCKET_PATH_LVGL_SERVER);
        } break;
        case PBOX_CHILD_BT: {
            strcpy(serverAddr.sun_path, SOCKET_PATH_BTSINK_SERVER);
        } break;
        case PBOX_CHILD_ROCKIT: {
            strcpy(serverAddr.sun_path, SOCKET_PATH_ROCKIT_SERVER);
        } break;
        case PBOX_CHILD_LED: {
            strcpy(serverAddr.sun_path, SOCKET_PATH_LED_EFFECT_SERVER);
        } break;
    }

    int ret = sendto(sockfd, info, length, MSG_DONTWAIT, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        printf("%s: Socket send failed! ret = %d\n", __func__, ret);
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return ret;
}