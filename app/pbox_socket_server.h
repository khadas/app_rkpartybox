#ifndef _PBOX_SOCKET_SERVER_H_
#define _PBOX_SOCKET_SERVER_H_
#ifdef __cplusplus
extern "C" {
#endif

int create_udp_socket(char *socket_path);
int unix_socket_send_msg(int module, void *info, int length);

#ifdef __cplusplus
}
#endif
#endif //_PBOX_SOCKET_SERVER_H_