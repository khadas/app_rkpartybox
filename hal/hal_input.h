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

#ifdef __cplusplus
}
#endif
#endif