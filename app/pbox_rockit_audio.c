#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>
#include <sys/syscall.h>
#include "slog.h"
#include "rc_comm_partybox.h"
#include "alsa_pcm.h"
#include "os_task.h"
#include "rc_partybox.h"
#include "pbox_rockit_audio.h"
#include "rk_utils.h"
#include "os_minor_type.h"

//#define PCM_DEVICE "hw:2,0"

static unsigned int pcm_buffer_time = 160000;
static unsigned int pcm_period_time =  20000;
extern rc_s32 (*rc_pb_recorder_dequeue_frame)(rc_pb_ctx, struct rc_pb_frame_info *, rc_s32);
extern rc_s32 (*rc_pb_recorder_queue_frame)(rc_pb_ctx, struct rc_pb_frame_info *, rc_s32);
extern rc_s32 (*rc_pb_player_start)(rc_pb_ctx, enum rc_pb_play_src, struct rc_pb_player_attr *);
extern rc_s32 (*rc_pb_player_stop)(rc_pb_ctx, enum rc_pb_play_src);
extern rc_s32 (*rc_pb_player_dequeue_frame)(rc_pb_ctx, enum rc_pb_play_src, struct rc_pb_frame_info *, rc_s32);
extern rc_s32 (*rc_pb_player_queue_frame)(rc_pb_ctx, enum rc_pb_play_src, struct rc_pb_frame_info *, rc_s32);
extern os_sem_t* auxplay_looplay_sem;
extern bool is_prompt_loop_playing;
extern int scene_detect_playing;

static void dump_out_data(const void* buffer,size_t bytes, int size)
{
   static FILE* fd;
   static int offset = 0;
   if(fd == NULL) {
       fd=fopen("/data/debug.pcm","wb+");
           if(fd == NULL) {
           ALOGD("DEBUG open /data/debug.pcm error =%d ,errno = %d\n",fd,errno);
           offset = 0;
       }
   }
   fwrite(buffer,bytes,1,fd);
   offset += bytes;
   fflush(fd);
   if(offset >= (size)*1024*1024) {
       fseek(fd, 0, SEEK_SET);
       offset = 0;
       ALOGD("TEST playback pcmfile restart\n");
   }
}

void *pbox_rockit_record_routine(void *arg) {
    snd_pcm_t *pcm_handle = NULL;
    char *buffer;
    int bits2byte, resample = 0;
    snd_pcm_sframes_t buffer_size;
    snd_pcm_sframes_t period_size;
    snd_pcm_sframes_t in_frames;
    snd_pcm_sframes_t frames;
    snd_pcm_sframes_t sent;
    alsa_card_conf_t audioConfig;

    struct rockit_pbx_t *ctx = arg;
    rc_pb_ctx *ptrboxCtx;
    struct rc_pb_frame_info frame_info;

    rk_setRtPrority(syscall(SYS_gettid), SCHED_RR, 0);
    assert(ctx != NULL);
    assert(ctx->pboxCtx != NULL);
    ptrboxCtx = ctx->pboxCtx;

    ALOGW("%s cardName: %s, freq:%d, chanel:%d\n", 
            __func__, ctx->audioFormat.cardName, ctx->audioFormat.sampingFreq, ctx->audioFormat.channel);

    while(os_sem_trywait(ctx->rec_stop_sem) != 0) {
        if(pcm_handle == NULL) {
            snprintf(audioConfig.cardName, sizeof(audioConfig.cardName), "%s", ctx->audioFormat.cardName);
            audioConfig.sampingFreq = ctx->audioFormat.sampingFreq;
            audioConfig.channel = ctx->audioFormat.channel;
            audioConfig.buffer_time = &pcm_buffer_time;
            audioConfig.period_time = &pcm_period_time;

            if (pcm_open(&pcm_handle, &audioConfig, SND_PCM_NONBLOCK) < 0) {
                ALOGW("pcm_open %s\n", strerror(errno));
                continue;
            }
            snd_pcm_get_params(pcm_handle, &buffer_size, &period_size);
            ALOGW("period_size: %d, byte:%d\n", period_size, period_size*4);
        }

        rc_pb_recorder_dequeue_frame(*ptrboxCtx, &frame_info, -1);
        in_frames = frame_info.size/frame_info.channels/(frame_info.bit_width/8);
        if(frame_info.channels == 1) {
            resample = 1;
            buffer = os_malloc(frame_info.size*2);
            for(int i = 0; i < in_frames; i++) {
                ((int16_t *)buffer)[2*i] =   ((int16_t *)frame_info.data)[i];
                ((int16_t *)buffer)[2*i+1] = ((int16_t *)frame_info.data)[i];
            }
        } else {
            resample = 0;
            buffer = frame_info.data;
        }
        int32_t written = 0;
        int retry = 10;
        int64_t frameTime = (in_frames)*1000000/(frame_info.sample_rate);//us
retry_alsa_write:
        if (retry <=0) goto skip_alsa;
        //2 channel,16bit.
        uint64_t tmp = os_get_boot_time_us();
        frames = snd_pcm_writei(pcm_handle, (char *)buffer + written*4, in_frames - written);
        //ALOGE("pbox snd_pcm_writei, in:%08d, out:%08d, sent:%08d, %"PRIu64"\t\n", in_frames, frames, sent, os_get_boot_time_us() - tmp);
        if (frames < 0) {
            switch (-frames) {
                case EINTR: {
                    goto retry_alsa_write;
                } break;
                case EAGAIN: {
                    if(retry--) usleep(1000);
                    goto retry_alsa_write;
                } break;
                case EPIPE: {
                    ALOGE("alsa playback underrun:%d\n", retry);
                    if(retry--)// usleep(1000);
                        snd_pcm_prepare(pcm_handle);
                    goto retry_alsa_write;
                } break;
                default: {
                    ALOGE("ALSA playback PCM write error: %s\n", snd_strerror(frames));
                    goto close_alsa;
                } break;
            }
        }

        if(frame_info.seq%1000 == 0) {
            ALOGD("%s seq:%d in_frames:%d, frames: %d\n", __func__, frame_info.seq, in_frames, frames);
        }

        //dump_out_data1((char *)buffer + sent*4, frames*4, 60);
        retry = 10;
        written = written + frames;
        int64_t timeUs = (in_frames - written)*1000000/(frame_info.sample_rate);
        if(timeUs > 0) {
            /* - Sometimes the sleep schedule is too long, which results in the data not being sent in time and
            *  resulting in an underrun.The sleep time cannot exceed the buffing time,So bufferTime/2
            *  is used to avoid that.
            */
           timeUs = (timeUs > frameTime / 2 ? frameTime / 2 : timeUs);
           usleep(timeUs);
        }

        if (written < in_frames) {
            //ALOGE("11rewriting written:%d, \n", frames, in_frames);
            goto retry_alsa_write;
        }

skip_alsa:
        if(resample) { os_free(buffer);}
        rc_pb_recorder_queue_frame(*ptrboxCtx, &frame_info, -1);
        continue;

close_alsa:
        ALOGE("ALSA close_alsa: %d\n", frames);
        if(resample) { os_free(buffer);}
        rc_pb_recorder_queue_frame(*ptrboxCtx, &frame_info, -1);
        pcm_close(&pcm_handle);
    }

    pcm_close(&pcm_handle);
    return NULL;
}

//this is auxiliary rockit player.
// it used to play ring, notification etc...
#define READ_SIZE 1024
struct _audio_file {
  const char* fileName;
  uint32_t sampleFreq;
  uint32_t channels;
} prompt_File[PROMPT_NUM] = {
    {"/oem/Stereo.pcm",             16000, 2},
    {"/oem/Mono.pcm",               16000, 2},
    {"/oem/Widen.pcm",              16000, 2},
    {"/oem/vocal_on.pcm",           16000, 2},
    {"/oem/vocal_off.pcm",          16000, 2},
    {"/oem/guitar_on.pcm",    16000, 2},
    {"/oem/guitar_off.pcm",   16000, 2},
    {"/oem/Sense.pcm",              16000, 2},
    {"/oem/doa.pcm",                16000, 2},
    {"/oem/antifeedback_on.pcm",    16000, 2},
    {"/oem/antifeedback_off.pcm",   16000, 2},
    {"/oem/zero.pcm",          16000, 2},
    {"/oem/one.pcm",           16000, 2},
    {"/oem/two.pcm",           16000, 2},
    {"/oem/three.pcm",         16000, 2},
    {"/oem/four.pcm",          16000, 2},
    {"/oem/five.pcm",          16000, 2},
};

void audio_sound_prompt(rc_pb_ctx *ptrboxCtx, prompt_audio_t index, bool loop) {
    int32_t size = 0;
    FILE *file = NULL;
    struct rc_pb_player_attr attr;
    struct rc_pb_frame_info frame_info;

    index = index & 0xff;
    memset(&attr, 0, sizeof(attr));
    attr.bit_width = 16;
    attr.channels = 2;
    attr.sample_rate = 16000;
    attr.pool_size = READ_SIZE;
    attr.pool_cnt = 1;
    attr.detect.rms_tc = 200;
    attr.detect.hold_time = 0;
    attr.detect.decay_time = 200;
    attr.detect.detect_per_frm = 2;
    attr.detect.band_cnt = 10;

    static int old = -1;

    if((old == (int)index) && (index != PROMPT_INOUT_SENCE) && (index != PROMPT_DOA_SENCE)) {
        return;
    }
    old = index;
    if(index >= PROMPT_NUM) {
        return;
    }

    attr.channels = prompt_File[index].channels;
    attr.sample_rate = prompt_File[index].sampleFreq;
    file = fopen(prompt_File[index].fileName, "rb");
    if (file == NULL) {
        ALOGE("%s open prompt file:%s failed: %s.\n", __func__, prompt_File[index].fileName, strerror(errno));
        return;
    }

    ALOGD("%s file:%s play start!!!!!\n", __func__, prompt_File[index].fileName);
    rc_pb_player_start(*ptrboxCtx, RC_PB_PLAY_SRC_PCM, &attr);

    is_prompt_loop_playing = loop;
    while (true) {
        if(os_sem_trywait(auxplay_looplay_sem) == 0) {
            loop = 0;
            frame_info.size = 0;
            is_prompt_loop_playing = 0;
            break;
        }
        rc_pb_player_dequeue_frame(*ptrboxCtx, RC_PB_PLAY_SRC_PCM,
                                    &frame_info, -1);
        memset(frame_info.data, 0, READ_SIZE);
        size = fread(frame_info.data, 1, READ_SIZE, file);
        if (size <= 0) {
            ALOGW("eof\n");
            if(loop) {
                fseek(file, 0, SEEK_SET);
            } else {
                frame_info.size = 0;
                rc_pb_player_queue_frame(*ptrboxCtx, RC_PB_PLAY_SRC_PCM,
                                            &frame_info, -1);
                fclose(file);
                break;
            }
        }
        frame_info.sample_rate = 16000;//prompt_File[index].sampleFreq;
        frame_info.channels = prompt_File[index].channels;
        frame_info.bit_width = 16;
        frame_info.size = size;
        rc_pb_player_queue_frame(*ptrboxCtx, RC_PB_PLAY_SRC_PCM,
                                    &frame_info, -1);
    }

    rc_pb_player_stop(*ptrboxCtx, RC_PB_PLAY_SRC_PCM);
    ALOGD("%s file:%s play stop!!!!!\n", __func__, prompt_File[index].fileName);
}

void* pbox_rockit_aux_player_routine(void *arg) {
    //os_task_t *task = (os_task_t *)arg;
    struct rockit_pbx_t *ctx = arg;
    rc_pb_ctx *ptrboxCtx;
    int table[5];
    bool loop;
    int fd;

    ALOGD("hello %s\n", __func__);
    assert(ctx != NULL);
    assert(ctx->pboxCtx != NULL);
    //ctx = (struct rockit_pbx_t *)task->params;
    fd = ctx->signfd[0];
    ptrboxCtx = ctx->pboxCtx;
    ALOGD("%s Ctx:%p\n", __func__, ptrboxCtx);

    while (os_sem_trywait(ctx->auxplay_stop_sem) != 0) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);

        int result = select(fd + 1, &read_fds, NULL, NULL, NULL);
        if (result < 0) {
            if (errno != EINTR) {
                perror("select failed");
                break;
            } else {
                continue;
            }
        }

        int stashCount;
        if (ioctl(fd, FIONREAD, &stashCount) < 0) {
            continue;
        }
        stashCount = stashCount / sizeof(int);
        if (stashCount <= 0) {
            continue;
        }
        if (stashCount > sizeof(table) / sizeof(table[0])) {
            stashCount = sizeof(table) / sizeof(table[0]);
        }

        OSI_NO_INTR(stashCount = read(fd, (char*)&table[0], stashCount * sizeof(int)));
        if (stashCount <= 0) {
            break;
        }
        stashCount = stashCount / sizeof(int);
        assert(stashCount > 0);

        result = table[stashCount - 1];
        loop = (result < 0)? true:false;
        unsigned int mask = ~(unsigned int)0 >> 1;//clear the highest bit.
        result &= mask;

        ALOGD("%s total:%d, last = %u, loop:%d\n", __func__, stashCount, (prompt_audio_t)result, loop);
        audio_sound_prompt(ptrboxCtx, result, loop);
        scene_detect_playing = 0;
    }
}
