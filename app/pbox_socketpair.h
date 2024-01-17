#ifndef _PBOX_SOCKETPAIR_H_
#define _PBOX_SOCKETPAIR_H_

#include "pbox_common.h"
#if !ENABLE_UDP_CONNECTION_LESS
int get_client_socketpair_fd(uint32_t source);
int get_server_socketpair_fd(uint32_t source);

int unix_socket_send_cmd(pb_module_child_t module, void *info, int length);//from children task to main task.
int unix_socket_notify_msg(pb_module_main_t module, void *info, int length);//from children task to main task.
#endif

#endif