#ifndef _PBOX_USB_SCAN_H_
#define _PBOX_USB_SCAN_H_
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

int scan_dir(const char *path, int depth, void (*call_back)(music_format_t, char *));
#ifdef __cplusplus
}
#endif
#endif