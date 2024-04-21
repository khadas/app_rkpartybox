#ifndef _UTILS_AUDIO_PCM_H_
#define _UTILS_AUDIO_PCM_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int sampingFreq;
    int channel;
    char cardName[16];
    unsigned int* buffer_time;
    unsigned int* period_time;
} alsa_card_conf_t;

int pcm_open(snd_pcm_t **pcm, alsa_card_conf_t* audioConfig, int block);

int pcm_close(snd_pcm_t **pcm);
#ifdef __cplusplus
}
#endif

#endif /* _UTILS_AUDIO_PCM_H_ */