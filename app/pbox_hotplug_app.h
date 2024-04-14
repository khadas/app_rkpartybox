#ifndef _PTBOX_HOTPLUG_APP_H_
#define _PTBOX_HOTPLUG_APP_H_
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

bool isUsbDiskConnected(void);
void pbox_app_usb_startScan(void);
void pbox_app_usb_pollState(void);
void maintask_hotplug_fd_process(int fd);

void pbox_app_uac_restart(void);

#ifdef __cplusplus
}
#endif
#endif
