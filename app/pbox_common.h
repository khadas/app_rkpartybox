#ifndef _PTBOX_COMMON_H_
#define _PTBOX_COMMON_H_
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#define SOCKET_PATH_BTSINK_SERVER "/tmp/rockchip_btsink_server"
#define SOCKET_PATH_BTSINK_CLIENT "/tmp/rockchip_btsink_client"

#define SOCKET_PATH_ROCKIT_SERVER "/tmp/rockchip_rockit_server"
#define SOCKET_PATH_ROCKIT_CLINET "/tmp/rockchip_rockit_client"

#define SOCKET_PATH_LED_EFFECT_SERVER "/tmp/rockchip_led_effect_server"
//#define SOCKET_PATH_LED_EFFECT_CLINET "/tmp/rockchip_led_effect_client" //no need

#define SOCKET_PATH_LVGL_SERVER "/tmp/rockchip_lvgl_server"
#define SOCKET_PATH_LVGL_CLINET "/tmp/rockchip_lvgl_client"

//#define SOCKET_PATH_USB_SERVER "/tmp/rockchip_usb_server"
#define SOCKET_PATH_USB_CLIENT "/tmp/rockchip_usb_client"

#define SOCKET_PATH_KEY_SCAN_CLINET "/tmp/rockchip_keyscan_client"
#define MAX(A, B) (A > B ? A : B)
#define MIN(A, B) (A < B ? A : B)
#define MAX_APP_NAME_LENGTH 255

typedef enum {
    PBOX_MAIN_LVGL,
    PBOX_MAIN_BT,
    PBOX_MAIN_KEYSCAN,
    PBOX_MAIN_ROCKIT,
    PBOX_MAIN_USBDISK,
    PBOX_MAIN_FD_TIMER,
    PBOX_MAIN_NUM
} pb_module_main_t;

typedef enum {
    PBOX_CHILD_LVGL = 0,
    PBOX_CHILD_BT,
    PBOX_CHILD_ROCKIT,
    PBOX_CHILD_LED,
    PBOX_CHILD_NUM
} pb_module_child_t;

typedef enum {
    PBOX_CMD = 1,
    PBOX_EVT = 2,
} pbox_msg_t;

typedef struct {
    int sampingFreq;
    int channel;
} pbox_audioFormat_t;

typedef struct {
    bool enable;
    unsigned int  u32HumanLevel;    /* RW; Range: [0, 100];*/
    unsigned int  u32ReservLevel;   /* RW; Range: [0, 100];*/
    unsigned int  u32OtherLevel;    /* RW; Range: [0, 100];*/
} pbox_vocal_t;

typedef enum {
    PBOX_REVERT_OFF = 0,
    PBOX_REVERT_USER = 0,
    PBOX_REVERT_STUDIO,
    PBOX_REVERT_KTV,
    PBOX_REVERT_CONCERT,
    PBOX_REVERT_BUTT,
} pbox_revertb_t;

typedef struct {
    int freq;
    int energy;
}energy_t;

typedef struct energy_info {
    int size;
    energy_t energykeep[10];
} energy_info_t;

typedef enum
{
    IDLE = 0,
    PLAYING, 
    _PAUSE,
    _STOP,
    PLAY_NUM
} play_status_t;

void start_fd_timer(int timer_fd, int start, int interval, int loop);
int create_fd_timer (void);
void pause_fd_timer(int timer_fd);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
