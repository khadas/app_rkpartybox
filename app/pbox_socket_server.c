#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
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
        printf("%s: Bind Local addr failed!\n", PRINT_FLAG_ERR);
        close(sockfd);
        return -1;
    }

    struct timeval t = {4, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&t, sizeof(t));

    return sockfd;
}

int unix_socket_send_msg(int module, void *info, int length)
{
    struct sockaddr_un serverAddr;
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("FUNC:%s create sockfd failed!\n", __func__);
        return -1;
    }

    serverAddr.sun_family = AF_UNIX;
    switch (module) { 
        case PBOX_LVGL: {
            strcpy(serverAddr.sun_path, SOCKET_PATH_LVGL_CLINET);
        } break;
        case PBOX_BT: {
            strcpy(serverAddr.sun_path, SOCKET_PATH_BTSINK_CLIENT);
        } break;
        case PBOX_ROCKIT: {
            strcpy(serverAddr.sun_path, SOCKET_PATH_ROCKIT_CLINET);
        } break;
        case PBOX_KEYSCAN: {
            strcpy(serverAddr.sun_path, SOCKET_PATH_KEY_SCAN_CLINET);
        } break;
    }

    int ret = sendto(sockfd, info, length, MSG_DONTWAIT, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        printf("%s: Socket send failed! ret = %d\n", PRINT_FLAG_ERR, ret);
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}


