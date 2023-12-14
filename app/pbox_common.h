#ifndef _PTBOX_COMMON_H_
#define _PTBOX_COMMON_H_
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
#define SOCKET_PATH_KEY_SCAN_SERVER "/tmp/rockchip_keyscan_server"
#define SOCKET_PATH_KEY_SCAN_CLINET "/tmp/rockchip_keyscan_client"
#define MAX(A,B) (A > B ? A : B)

typedef enum {
    PBOX_LVGL = 0,
    PBOX_BT,
    PBOX_ROCKIT,
    PBOX_KEYSCAN,
    PBOX_NUM
} pb_source_t;

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
    unsigned int  u32GuitarLevel;   /* RW; Range: [0, 100];*/
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

void start_fd_timer(int timer_fd, int start, int interval, int loop);
int create_fd_timer (void);
void pause_fd_timer(int timer_fd);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif