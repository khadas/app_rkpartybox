#ifndef _PBOX_STORE_APP_H_
#define _PBOX_STORE_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

int pbox_app_music_store_volume(float volume);
int pbox_app_music_store_music_volume(float volume);
int pbox_app_music_store_accomp_music_level(uint32_t volume);
int pbox_app_music_store_human_music_level(uint32_t volume);
int pbox_app_music_store_reserv_music_level(uint32_t volume);
int pbox_app_music_store_vocal_split(bool split);

int pbox_app_music_store_stereo_mode(stereo_mode_t stereo);
int pbox_app_music_store_outdoor_mode(inout_door_t outdoor);
int pbox_app_music_store_placement(placement_t place);

int pbox_app_music_store_mic_mux(uint8_t index, mic_mux_t mux);
int pbox_app_music_store_mic_volume(uint8_t index, float volume);
int pbox_app_music_store_echo_3a(uint8_t index, bool enable);
int pbox_app_music_store_mic_mute(uint8_t index, bool mute);
int pbox_app_music_store_recoder_revert(uint8_t index, pbox_revertb_t reverbMode);
int pbox_app_music_store_mic_treble(uint8_t index, float treble);
int pbox_app_music_store_mic_bass(uint8_t index, float bass);
int pbox_app_music_store_mic_reverb(uint8_t index, float reverb);
int pbox_app_data_save(void);

int pbox_app_ui_load(void);
int pbox_app_data_deinit(void);
int pbox_app_ui_init(const char *ini_path);
#ifdef __cplusplus
}
#endif
#endif