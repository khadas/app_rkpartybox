#ifndef _PTBOX_HARDWARE_INPUT_H_
#define _PTBOX_HARDWARE_INPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SRC_CHIP_USB,
    SRC_CHIP_BT,
    SRC_CHIP_UAC,
    SRC_EXT_BT,
    SRC_EXT_USB,
    SRC_EXT_AUX,
    SRC_NUM
} input_source_t;

#define MASK_SRC_CHIP_USB  (1 << SRC_CHIP_USB)
#define MASK_SRC_CHIP_BT   (1 << SRC_CHIP_BT)
#define MASK_SRC_CHIP_UAC  (1 << SRC_CHIP_UAC)
#define MASK_SRC_EXT_BT    (1 << SRC_EXT_BT)
#define MASK_SRC_EXT_USB   (1 << SRC_EXT_USB)
#define MASK_SRC_EXT_AUX   (1 << SRC_EXT_AUX)

typedef struct {
    int kernel_space;
    int user_space;
} key_pair_t;

typedef enum {
    HKEY_IDLE,
    HKEY_PLAY,//KEY APP PLAY
    HKEY_VOLUP,
    HKEY_VOLDOWN,
    HKEY_MODE,
    HKEY_MIC1MUTE,
    HKEY_MIC2MUTE,
    HKEY_MIC1BASS,
    HKEY_MIC2BASS,
    HKEY_MIC1TREB,
    HKEY_MIC2TREB,
    HKEY_MIC1REVB,
    HKEY_MIC2REVB,
    HKEY_MIC1_VOL,
    HKEY_MIC2_VOL,
    HKEY_NUM
} hal_key_t;

#ifdef __cplusplus
}
#endif
#endif