#ifndef _PTBOX_HARDWARE_BOARD_H_
#define _PTBOX_HARDWARE_BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

//#define PBOX_INPUT_SRCS HW_SUPPORT_SRCS

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

#define MASK_GUITAR 0
#define MASK_MIC    1

//sync this define to enum rc_pb_ref_mode
#define ECHO_REF_MODE_NONE       0
#define ECHO_REF_MODE_SOFT       1
#define ECHO_REF_MODE_HARD_COMBO 2
#define ECHO_REF_MODE_HARD_IND   3

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
    HKEY_GPIO_KEY1,
    HKEY_GPIO_KEY2,
    HKEY_ROTARY_POS,
    HKEY_ROTARY_NEG,
    HKEY_NUM
} hal_key_t;

float hw_main_gain(uint8_t index);
float hw_music_gain(uint8_t index);
float hw_mic_reverb(uint8_t index);
float hw_mic_treble(uint8_t index);
float hw_mic_bass(uint8_t index);
float hw_mic_gain(uint8_t index);
float hw_guitar_reverb(uint8_t index);
float hw_guitar_treble(uint8_t index);
float hw_guitar_bass(uint8_t index);

const float* hw_get_main_volume_table(uint16_t* size);
const float* hw_get_music_volume_table(uint16_t* size);
const float* hw_get_mic_reverb_table(uint16_t* size);
const float* hw_get_mic_treble_table(uint16_t* size);
const float* hw_get_mic_bass_table(uint16_t* size);
const float* hw_get_guitar_reverb_table(uint16_t* size);
const float* hw_get_guitar_treble_table(uint16_t* size);
const float* hw_get_guitar_bass_table(uint16_t* size);

int hal_dsp_max_main_vol(void);
int hal_dsp_max_music_vol(void);
int hal_dsp_max_mic_reverb(void);
int hal_dsp_max_mic_treble(void);
int hal_dsp_max_mic_bass(void);
int hal_dsp_max_mic_vol(void);

int hal_max_saradc_val(void);
int hal_min_saradc_val(void);

char *hal_get_audio_card(input_source_t source);
char* getInputSourceString(input_source_t source);
int hal_key_convert_kernel_to_upper(uint32_t value);

//speaker or codec
char *hal_get_spk_codec_card(void);
uint8_t hal_get_audio_spk_codec_channel(void);

//guitar
char *hal_get_kalaok_guitar_card(void);
//mic
char *hal_get_kalaok_mic_card(void);
uint8_t hal_get_kalaok_mic_ref_layout(void);
uint8_t hal_get_kalaok_mic_rec_layout(void);
uint8_t hal_get_kalaok_mic_chn_layout(void);
uint8_t hal_get_kalaok_mic_rec_channel(void);
uint8_t hal_get_kalaok_poor_count(void);
uint8_t hal_get_kalaok_ref_hard_mode(void);
uint32_t hal_get_supported_mic_matrix(void);
uint32_t hal_get_kalaok_rec_sample_rate(void);

//scene
char *hal_get_audio_scene_card(void);
uint8_t hal_get_scene_mic_ref_layout(void);
uint8_t hal_get_scene_mic_rec_layout(void);
uint8_t hal_get_scene_mic_rec_channel(void);
uint8_t hal_get_scene_ref_hard_mode(void);
uint32_t hal_get_scene_rec_sample_rate(void);

//input source
input_source_t hal_get_favor_source_order(int index);
uint32_t hal_get_supported_sources(void);

bool hal_get_sara_adc_usage(void);
#ifdef __cplusplus
}
#endif
#endif
