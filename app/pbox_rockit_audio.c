#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "slog.h"
#include <alsa/asoundlib.h>
#include <sys/syscall.h>
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
#include <inttypes.h>

void *pbox_rockit_record_routine(void *params) {
    snd_pcm_t *pcm_handle = NULL;
    char *buffer;
    int bits2byte, resample = 0;
    snd_pcm_sframes_t buffer_size;
    snd_pcm_sframes_t period_size;
    snd_pcm_sframes_t in_frames;
    snd_pcm_sframes_t frames;
    snd_pcm_sframes_t sent;
    alsa_card_conf_t audioConfig;

    os_task_t *task = (os_task_t *)params;
    struct rockit_pbx_t *ctx;
    rc_pb_ctx *ptrboxCtx;
    struct rc_pb_frame_info frame_info;
    if(task == NULL) return NULL;

    task->pid_tid = syscall(SYS_gettid);//linux tid, not posix tid.
    rk_setRtPrority(task->pid_tid, SCHED_RR, 0);
    ctx = (struct rockit_pbx_t *)task->params;
    assert(ctx != NULL);
    assert(ctx->pboxCtx != NULL);
    ptrboxCtx = ctx->pboxCtx;

    ALOGW("%s cardName: %s, freq:%d, chanel:%d\n", 
            __func__, ctx->audioFormat.cardName, ctx->audioFormat.sampingFreq, ctx->audioFormat.channel);

    while(__atomic_load_n(&task->runing, __ATOMIC_RELAXED)) {
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

        rc_pb_recorder_dequeue_frame(*ptrboxCtx, &frame_info, -1);//dlopen not ready!!!
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
        uint64_t tmp = time_get_os_boot_us();
        frames = snd_pcm_writei(pcm_handle, (char *)buffer + written*4, in_frames - written);
        //ALOGE("pbox snd_pcm_writei, in:%08d, out:%08d, sent:%08d, %"PRIu64"\t\n", in_frames, frames, sent, time_get_os_boot_us() - tmp);
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
                    ALOGE("ALSA playback PCM underrun\n");
                    if(retry--) usleep(1000);
                    //snd_pcm_prepare(pcm_handle);
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