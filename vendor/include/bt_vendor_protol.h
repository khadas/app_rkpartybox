#ifndef _VENDOR_INCLUDE_BT_VENDOR_PROTOL_
#define _VENDOR_INCLUDE_BT_VENDOR_PROTOL_
#ifdef __cplusplus
extern "C" {
#endif
int dsp2btsoc_echo_version(char *version);
int dsp2btsoc_notify_main_volume(float volume);
int dsp2btsoc_notify_music_volume(float volume);
int dsp2btsoc_notify_mic_mux(uint8_t index, uint8_t mux);
int dsp2btsoc_notify_inout_door(uint8_t inout);
int dsp2btsoc_notify_power_on(bool on);
int dsp2btsoc_notify_stereo_mode(uint8_t mode);
int dsp2btsoc_notify_human_voice_fadeout(bool fadeout);
int dsp2btsoc_notify_switch_source(uint8_t source, uint8_t status);

typedef void (*NotifyDSPVersionFunc)(uint32_t opcode, char *ver, int32_t len);
typedef void (*NotifyMasterVolumeFunc)(uint32_t opcode, float volume);
typedef void (*NotifyPlacementFunc)(uint32_t opcode, uint8_t placement);
typedef void (*NotifyMicMuxFunc)(uint32_t opcode, uint8_t mux);
typedef void (*NotifyInOutDoorFunc)(uint32_t opcode, uint8_t inout);
typedef void (*NotifyPowerStateFunc)(uint32_t opcode, bool on);
typedef void (*NotifyStereoModeFunc)(uint32_t opcode, uint8_t stereo);
typedef void (*NotifyHumanVoiceFadeoutFunc)(uint32_t opcode, bool fadeout);
typedef void (*NotifyVocalRatioLevelFunc)(uint32_t opcode, uint32_t level);
typedef void (*NotifySourceSwitchFunc)(uint32_t opcode, uint8_t source, uint8_t status);
typedef void (*NotifyMusicVolumeFunc)(uint32_t opcode, float volume);
typedef void (*NotifyMicMuteFunc)(uint32_t opcode, uint8_t index, bool mute);
typedef void (*NotifyMicIndexMuxFunc)(uint32_t opcode, uint8_t index, uint8_t mux);
typedef void (*NotifyMic3aEffectFunc)(uint32_t opcode, uint8_t index, bool on);
typedef void (*NotifyMicReverbModeFunc)(uint32_t opcode, uint8_t index, uint8_t reverbMode);
typedef void (*NotifyMicReverbFunc)(uint32_t opcode, uint8_t index, float micReverb);
typedef void (*NotifyMicBassFunc)(uint32_t opcode, uint8_t index, float micBass);
typedef void (*NotifyMicTrebleFunc)(uint32_t opcode, uint8_t index, float micTreble);
typedef void (*NotifyMicVolumeFunc)(uint32_t opcode, uint8_t index, float micVolume);

struct NotifyFuncs {
    NotifyDSPVersionFunc notify_dsp_version;
    NotifyMasterVolumeFunc notify_master_volume;
    NotifyPlacementFunc notify_placement;
    NotifyMicMuxFunc notify_mic1_mux;
    NotifyMicMuxFunc notify_mic2_mux;
    NotifyInOutDoorFunc notify_inout_door;
    NotifyPowerStateFunc notify_dsp_power_state;
    NotifyStereoModeFunc notify_dsp_stereo_mode;
    NotifyHumanVoiceFadeoutFunc notify_human_voice_fadeout;
    NotifyVocalRatioLevelFunc notify_vocal_human_level;
    NotifyVocalRatioLevelFunc notify_vocal_accomp_level;
    NotifyVocalRatioLevelFunc notify_vocal_reserv_level;
    NotifySourceSwitchFunc notify_dsp_switch_source;
    NotifyMusicVolumeFunc notify_music_volume;
    NotifyMicVolumeFunc notify_mic_volume;
    NotifyMicMuteFunc notify_mic_mute;
    NotifyMicIndexMuxFunc notify_mic_index_mux;
    NotifyMic3aEffectFunc notify_mic_3a_effect;
    NotifyMicReverbModeFunc notify_mic_reverb_mode;
    NotifyMicReverbFunc notify_mic_reverb;
    NotifyMicBassFunc notify_mic_bass;
    NotifyMicTrebleFunc notify_mic_treble;
};

typedef struct NotifyFuncs NotifyFuncs_t;
typedef struct uart_data_recv_class uart_data_recv_class_t;
typedef void (*vendor_data_recv_handler_t)(int, uart_data_recv_class_t*);
vendor_data_recv_handler_t vendor_get_data_recv_func(void);
int btsoc_register_vendor_notify_func(const NotifyFuncs_t* notify_funcs);
int btsoc_register_uart_write_fd(int fd);
uart_data_recv_class_t* uart_data_recv_init(void);
#ifdef __cplusplus
}
#endif
#endif