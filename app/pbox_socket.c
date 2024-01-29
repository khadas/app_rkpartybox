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

#define PRINT_FLAG_ERR "[RK_SKT_ERROR]"
#define PRINT_FLAG_SUCESS "[RK_SKT_SUCESS]"

int create_udp_socket(char *socket_path)
{
    if ((socket_path == NULL) || (strlen(socket_path) == 0))
        return -1;
    printf("----- %s server -----\n", socket_path);

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