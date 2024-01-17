#ifndef _PBOX_USB_UAC_H_
#define _PBOX_USB_UAC_H_
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif


void uac_init(void);
void parse_event(const struct _uevent *event);
int uac_monitor_get_fd(void);

#ifdef __cplusplus
}
#endif
#endif