#ifndef _PBOX_SOCKET_H_
#define _PBOX_SOCKET_H_
#include "pbox_common.h"

#ifdef __cplusplus
extern "C" {
#endif
int create_udp_socket(char *socket_path);
int unix_socket_notify_msg(pb_module_main_t module, void *info, int length); //from children task to main task.
int unix_socket_send_cmd(pb_module_child_t module, void *info, int length); //from children task to main task.
#ifdef __cplusplus
}
#endif
#endif //_PBOX_SOCKET_H_