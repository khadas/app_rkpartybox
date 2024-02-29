#ifndef _PTBOX_COMMON_H_
#define _PTBOX_COMMON_H_
#include <stdbool.h>
#include <stdint.h>
#include "slog.h"
#include "pbox_model.h"

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

#define SOCKET_PATH_USB_SERVER "/tmp/rockchip_usb_server"
#define SOCKET_PATH_USB_CLIENT "/tmp/rockchip_usb_client"

#define SOCKET_PATH_KEY_SCAN_CLINET "/tmp/rockchip_keyscan_client"

#define MUSIC_PATH "/mnt/udisk/"
#define MAX(A, B) (A > B ? A : B)
#define MIN(A, B) (A < B ? A : B)

#define MAX_SHORT_NAME_LENGTH 63
#define TRACK_MAX_NUM 100

typedef enum {
    PBOX_SOCKPAIR_BT,
    PBOX_SOCKPAIR_KEYSCAN,
#if ENABLE_RK_ROCKIT
    PBOX_SOCKPAIR_ROCKIT,
#endif
    PBOX_SOCKPAIR_USBDISK,
#if ENABLE_LCD_DISPLAY
    PBOX_SOCKPAIR_LVGL,
#endif
#if ENABLE_RK_LED_EFFECT
    PBOX_SOCKPAIR_LED, //keep it before PBOX_MAIN_NUM
#endif
    PBOX_SOCKPAIR_NUM
} pb_sock_pair_t;

typedef enum {
    PBOX_MAIN_BT,
    PBOX_MAIN_KEYSCAN,
#if ENABLE_RK_ROCKIT
    PBOX_MAIN_ROCKIT,
#endif
    PBOX_MAIN_USBDISK,
#if ENABLE_LCD_DISPLAY
    PBOX_MAIN_LVGL,
#endif
    PBOX_MAIN_FD_TIMER,
    PBOX_MAIN_NUM
} pb_module_main_t;

typedef enum {
    PBOX_CHILD_BT,
#if ENABLE_RK_ROCKIT
    PBOX_CHILD_ROCKIT,
#endif
#if ENABLE_RK_LED_EFFECT
    PBOX_CHILD_LED,
#endif

    PBOX_CHILD_USBDISK,
#if ENABLE_LCD_DISPLAY
    PBOX_CHILD_LVGL,
#endif
    PBOX_CHILD_NUM
} pb_module_child_t;

typedef enum {
    PBOX_CMD = 1,
    PBOX_EVT = 2,
} pbox_msg_t;

typedef struct {
    int sampingFreq;
    int channel;
    char cardName[30];
} pbox_audioFormat_t;

typedef struct {
    bool enable;
    unsigned int  humanLevel;    /* RW; Range: [0, 100];*/
    unsigned int  accomLevel;    /* RW; Range: [0, 100];*/
    unsigned int  reservLevel;   /* RW; Range: [0, 100];*/
} pbox_vocal_t;

typedef enum {
    PBOX_REVERT_OFF = 0,
    PBOX_REVERT_USER = 0,
    PBOX_REVERT_STUDIO,
    PBOX_REVERT_KTV,
    PBOX_REVERT_CONCERT,
    PBOX_REVERT_ECHO,
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

typedef enum {
    MUSIC_FILE_NONE,
    MUSIC_FILE_MP3,
    MUSIC_FILE_WAV,
    MUSIC_FILE_FLAC,
    MUSIC_FILE_OGG
} music_format_t;

typedef enum {
    USB_DISCONNECTED,
    USB_CONNECTED,
    USB_SCANNING,
    USB_SCANNED,
} usb_state_t;

typedef struct {
    usb_state_t usbState;
    char usbDiskName[MAX_APP_NAME_LENGTH+1];//reserved
} usb_disk_info_t;

typedef struct {
    music_format_t format;//reserved
    char fileName[MAX_MUSIC_NAME_LENGTH+1];
} usb_music_file_t;

typedef enum {
    UAC_ROLE_SPEAKER,
    UAC_ROLE_RECORDER,
    UAC_ROLE_NUM
} uac_role_t;

typedef enum {
    SRC_USB,
    SRC_BT,
#if ENABLE_UAC
    SRC_UAC,
#endif
#if ENABLE_AUX
    SRC_AUX,
#endif
#if ENABLE_RAW_PCM
    SRC_PCM,
#endif
    SRC_NUM
} input_source_t;

typedef enum {
    MANUAL,
    AUTO,
    ANY
} switch_source_t;

typedef enum {
    OP_WRITE,
    OP_READ,
    OP_NUM
} ext_cmd_t;

typedef enum {
    MIC_OFF = 1,
    MIC_IN = 2,
    MIC_GT = 3,
}mic_mux_t;

typedef struct {
    uint8_t index;
    mic_mux_t micMux;
    uint32_t micVolume;
    uint32_t micTreble;
    uint32_t micBass;
    uint32_t micReverb;
} mic_data_t;

typedef enum {
    INDOOR = 0,
    OUTDOOR = 1,
}inout_door_t;

typedef enum {
    PLACE_AUTO,
    PLACE_HORI,
    PLACE_VERT,
}placement_t;

typedef enum {
    MODE_STEREO = 0,
    MODE_MONO = 1,
    MODE_WIDEN = 2,
}stereo_mode_t;

typedef struct _uac {
    uac_role_t uac_role;
    union {
        bool state;
        uint32_t sampleFreq;
        uint32_t volume;
        bool mute;
        int32_t ppm;
    };
} uac_t;

typedef struct _pbox_pipe {
    int fd[2];
} pbox_pipe_t;

#define PBOX_ARRAY_SET(array, value, size)	\
do											\
{											\
    for (int i=0; i< size; i++)				\
    {										\
        array[i] = value;					\
    }										\
} while (0);

extern pbox_pipe_t pbox_pipe_fds[PBOX_SOCKPAIR_NUM];

void start_fd_timer(int timer_fd, int start, int interval, bool loop);
int create_fd_timer (void);
void pause_fd_timer(int timer_fd);
uint64_t time_get_os_boot_us(void);
uint64_t time_get_os_boot_ms(void);
int findMax(int array[], int size);
void pbox_init_background(void);
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
