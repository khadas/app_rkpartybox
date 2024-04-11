#ifndef _UTILS_AUDIO_PCM_H_
#define _UTILS_AUDIO_PCM_H_

#ifdef __cplusplus
extern "C" {
#endif
int pcm_open(snd_pcm_t **pcm, const char* device, int channels, int rate,
        unsigned int *buffer_time, unsigned int *period_time);

int pcm_close(snd_pcm_t **pcm);
#ifdef __cplusplus
}
#endif

#endif /* _UTILS_AUDIO_PCM_H_ */