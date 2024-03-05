#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#include <sys/select.h>
#include <sys/time.h>
#include "rc_partybox.h"
#include "rc_rkstudio_vendor.h"
#include "pbox_common.h"
#include "pbox_rockit.h"
#include "pbox_socket.h"
#include "pbox_socketpair.h"

//static void karaoke_callback(RK_VOID *pPrivateData, KARAOKE_EVT_E event, rc_s32 ext1, RK_VOID *ptr);
static void pb_rockit_notify(enum rc_pb_event event, rc_s32 cmd, void *opaque);

pthread_t rockit_task_id;
//void *player_ctx = NULL;
rc_pb_ctx partyboxCtx;
void *mpi_hdl = NULL;

rc_s32 (*rc_pb_create)(rc_pb_ctx *ctx, struct rc_pb_attr *attr);
rc_s32 (*rc_pb_destroy)(rc_pb_ctx ctx);
rc_s32 (*rc_pb_set_volume)(rc_pb_ctx ctx, rc_float volume_db);
rc_s32 (*rc_pb_get_volume)(rc_pb_ctx ctx, rc_float *volume_db);

rc_s32 (*rc_pb_player_start)(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_player_attr *attr);
rc_s32 (*rc_pb_player_stop)(rc_pb_ctx ctx, enum rc_pb_play_src src);
rc_s32 (*rc_pb_player_pause)(rc_pb_ctx ctx, enum rc_pb_play_src src);
rc_s32 (*rc_pb_player_resume)(rc_pb_ctx ctx, enum rc_pb_play_src src);
rc_s32 (*rc_pb_player_dequeue_frame)(rc_pb_ctx ctx, enum rc_pb_play_src src,
                                     struct rc_pb_frame_info *frame_info, rc_s32 ms);
rc_s32 (*rc_pb_player_queue_frame)(rc_pb_ctx ctx, enum rc_pb_play_src src,
                                   struct rc_pb_frame_info *frame_info, rc_s32 ms);

rc_s32 (*rc_pb_player_get_position)(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_s64 *usec);
rc_s32 (*rc_pb_player_get_duration)(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_s64 *usec);
rc_s32 (*rc_pb_player_set_loop)(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_bool loop);
rc_s32 (*rc_pb_player_seek)(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_s64 usec);
rc_s32 (*rc_pb_player_set_volume)(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_float volume_db);
rc_s32 (*rc_pb_player_get_volume)(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_float *volume);
rc_s32 (*rc_pb_player_set_param)(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_param *param);
rc_s32 (*rc_pb_player_get_param)(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_param *param);
rc_s32 (*rc_pb_player_get_energy)(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_energy *energy);
rc_s32 (*rc_pb_player_release_energy)(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_energy *energy);

rc_s32 (*rc_pb_recorder_start)(rc_pb_ctx ctx);
rc_s32 (*rc_pb_recorder_stop)(rc_pb_ctx ctx);
rc_s32 (*rc_pb_recorder_mute)(rc_pb_ctx ctx, rc_s32 idx, rc_bool mute);
rc_s32 (*rc_pb_recorder_set_volume)(rc_pb_ctx ctx, rc_s32 idx, rc_float volume_db);
rc_s32 (*rc_pb_recorder_get_volume)(rc_pb_ctx ctx, rc_s32 idx, rc_float *volume_db);
rc_s32 (*rc_pb_recorder_set_param)(rc_pb_ctx ctx, rc_s32 idx, struct rc_pb_param *param);
rc_s32 (*rc_pb_recorder_get_param)(rc_pb_ctx ctx, rc_s32 idx, struct rc_pb_param *param);

bool started_player[RC_PB_PLAY_SRC_BUTT];
int rk_demo_music_create() {
    //create karaoke recorder && player
    struct rc_pb_attr attr;
    static struct rc_pb_recorder_attr recorder_attr;

    mpi_hdl = dlopen("librockit.so", RTLD_LAZY);
    if (NULL == mpi_hdl) {
        printf("failed to open librockit.so, err:%s\n", dlerror());
        return -1;
    }

    if (mpi_hdl != NULL) {
         rc_pb_create = (rc_s32 (*)(rc_pb_ctx *ctx, struct rc_pb_attr *attr))dlsym(mpi_hdl, "rc_pb_create");
        if (NULL == rc_pb_create) {
                printf("failed to open func, err=%s\n", dlerror());
                return -1;
        }
        rc_pb_destroy = (rc_s32 (*)(rc_pb_ctx ctx))dlsym(mpi_hdl, "rc_pb_destroy");
        if (NULL == rc_pb_destroy) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_set_volume = (rc_s32 (*)(rc_pb_ctx ctx, rc_float volume_db))dlsym(mpi_hdl, "rc_pb_set_volume");
        if (NULL == rc_pb_set_volume) {
            printf("failed to dlsym rc_pb_set_volume, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_get_volume = (rc_s32 (*)(rc_pb_ctx ctx, rc_float volume_db))dlsym(mpi_hdl, "rc_pb_get_volume");
        if (NULL == rc_pb_get_volume) {
                printf("failed to dlsym rc_pb_get_volume, err=%s\n", dlerror());
                return -1;
        }

        rc_pb_player_start = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_player_attr *attr))dlsym(mpi_hdl, "rc_pb_player_start");
        if (NULL == rc_pb_player_start) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_player_stop = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src))dlsym(mpi_hdl, "rc_pb_player_stop");
        if (NULL == rc_pb_player_stop) {
        printf("failed to open  func, err=%s\n", dlerror());
        return -1;
        }

        rc_pb_player_pause = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src))dlsym(mpi_hdl, "rc_pb_player_pause");
        if (NULL == rc_pb_player_pause) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_player_resume = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src))dlsym(mpi_hdl, "rc_pb_player_resume");
        if (NULL == rc_pb_player_resume) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_player_get_position = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_s64 *usec))dlsym(mpi_hdl, 
                                                "rc_pb_player_get_position");
        if (NULL == rc_pb_player_get_position) {
        printf("failed to open  func, err=%s\n", dlerror());
        return -1;
            }

        rc_pb_player_get_duration = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_s64 *usec))dlsym(mpi_hdl, 
                                                "rc_pb_player_get_duration");
        if (NULL == rc_pb_player_get_duration) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_player_set_loop = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_bool loop))dlsym(mpi_hdl, "rc_pb_player_set_loop");
        if (NULL == rc_pb_player_set_loop) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_player_seek = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_s64 usec))dlsym(mpi_hdl, "rc_pb_player_seek");
        if (NULL == rc_pb_player_seek) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_player_set_volume = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_float volume_db))dlsym(mpi_hdl, 
                                                "rc_pb_player_set_volume");
        if (NULL == rc_pb_player_set_volume) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_player_get_volume = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src, rc_float *volume_db))dlsym(mpi_hdl, 
                                                "rc_pb_player_get_volume");
        if (NULL == rc_pb_player_get_volume) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_player_set_param = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_param *param))dlsym(mpi_hdl,
                                                "rc_pb_player_set_param");
        if (NULL == rc_pb_player_set_param) {
            printf("failed to open func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_player_get_param = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_param *param))dlsym(mpi_hdl, 
                                            "rc_pb_player_get_param");
        if (NULL == rc_pb_player_get_param) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_player_get_energy = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_energy *energy))dlsym(mpi_hdl, 
                                            "rc_pb_player_get_energy");
        if (NULL == rc_pb_player_get_energy) {
            printf("failed to open func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_player_release_energy = (rc_s32 (*)(rc_pb_ctx ctx, enum rc_pb_play_src src, struct rc_pb_energy *energy))dlsym(mpi_hdl,
                                                            "rc_pb_player_release_energy");
        if (NULL == rc_pb_player_release_energy) {
            printf("failed to open func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_recorder_start = (rc_s32 (*)(rc_pb_ctx ctx))dlsym(mpi_hdl, "rc_pb_recorder_start");
        if (NULL == rc_pb_recorder_start) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_recorder_stop = (rc_s32 (*)(rc_pb_ctx ctx))dlsym(mpi_hdl, "rc_pb_recorder_stop");
        if (NULL == rc_pb_recorder_stop) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_recorder_mute =  (rc_s32 (*)(rc_pb_ctx ctx, rc_s32 idx, rc_bool mute))dlsym(mpi_hdl,
                                                    "rc_pb_recorder_mute");
        if (NULL == rc_pb_recorder_mute) {
            printf("failed to open func, err=%s", dlerror());
            return -1;
        }

        rc_pb_recorder_set_volume =  (rc_s32 (*)(rc_pb_ctx ctx, rc_s32 idx, rc_float volume_db))dlsym(mpi_hdl,
                                                "rc_pb_recorder_set_volume");
        if (NULL == rc_pb_recorder_set_volume) {
            printf("failed to open func, err=%s",dlerror());
            return -1;
        }

        rc_pb_recorder_get_volume =  (rc_s32 (*)(rc_pb_ctx ctx, rc_s32 idx, rc_float *volume_db))dlsym(mpi_hdl,
                                            "rc_pb_recorder_get_volume");
        if (NULL == rc_pb_recorder_get_volume) {
            printf("failed to open func, err=%s", dlerror());
            return -1;
        }

        rc_pb_recorder_set_param = (rc_s32 (*)(rc_pb_ctx ctx, rc_s32 idx, struct rc_pb_param *param))dlsym(mpi_hdl, 
                                            "rc_pb_recorder_set_param");
        if (NULL == rc_pb_recorder_set_param) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }

        rc_pb_recorder_get_param = (rc_s32 (*)(rc_pb_ctx ctx, rc_s32 idx, struct rc_pb_param *param))dlsym(mpi_hdl, 
                                            "rc_pb_recorder_get_param");
        if (NULL == rc_pb_recorder_get_param) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
        }
    }

    attr.card_name              = "hw:0,0";
    attr.sample_rate            = 48000;
#if ENABLE_USE_SOCBT
    attr.channels               = 6;
#else
    attr.channels               = 2;
#endif
    attr.bit_width              = 16;
    attr.notify                 = pb_rockit_notify;
    attr.opaque                 = NULL;
    attr.record_attr            = &recorder_attr;
    attr.volume_db                 = -50;//main volume

    recorder_attr.card_name = "hw:0,0";
    recorder_attr.sample_rate = 48000;
    recorder_attr.channels    = 4;
    recorder_attr.bit_width   = 16;
#if ENABLE_USE_SOCBT
    recorder_attr.ref_layout = 0x0c;
    recorder_attr.rec_layout = 0x03;
    recorder_attr.ref_mode = RC_PB_HOWLING_REF_MODE_SOFT;
#else
    recorder_attr.ref_layout = 0x03;
    recorder_attr.rec_layout = 0x04;
    recorder_attr.ref_mode = RC_PB_HOWLING_REF_MODE_HARD;
#endif
    recorder_attr.chn_layout  = 0x0f;
    if (rc_pb_create(&partyboxCtx, &attr) != 0) {
        printf("rc_pb_create failed, err!!!\n");
        return -1;
    }

    if (rc_pb_recorder_start(partyboxCtx) != 0) {
        printf("rc_pb_recorder_start failed, err!!!\n");
        return -1;
    }
    printf("rockit media player created successfully, partyboxCtx=%p\n", partyboxCtx);
    memset(started_player, 0 , sizeof(started_player));
}

static void rockit_pbbox_notify_awaken(uint32_t wakeCmd)
{
    #if ENABLE_RK_ROCKIT
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;
    msg.msgId = PBOX_ROCKIT_AWAKEN_EVT;
    msg.wake_up.wakeCmd = wakeCmd;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
    #endif
}

static void rockit_pbbox_notify_playback_status(enum rc_pb_event event)
{
    #if ENABLE_RK_ROCKIT
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;

    if(event == RC_PB_EVENT_PLAYBACK_COMPLETE || event == RC_PB_EVENT_PLAYBACK_ERROR)
        msg.msgId = PBOX_ROCKIT_PLAY_COMPLETED_EVT;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
    #endif
}

//before call this func, duration shoud covert to ms(msecond), not us.
static void rockit_pbbox_notify_duration(uint32_t duration)
{
    #if ENABLE_RK_ROCKIT
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;
    msg.msgId = PBOX_ROCKIT_MUSIC_DURATION_EVT;
    msg.duration = duration;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
    #endif
}

//before call this func, duration shoud covert to ms(msecond), not us.
static void rockit_pbbox_notify_current_postion(uint32_t current)
{
    #if ENABLE_RK_ROCKIT
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;
    msg.msgId = PBOX_ROCKIT_MUSIC_POSITION_EVT;
    msg.mPosition = current;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
    #endif
}

static void rockit_pbbox_notify_volume(float volume)
{
    #if ENABLE_RK_ROCKIT
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;
    msg.msgId = PBOX_ROCKIT_MUSIC_MAIN_VOLUME_EVT;
    msg.volume = volume;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
    #endif
}

static void rockit_pbbox_notify_music_volume(float volume)
{
    #if ENABLE_RK_ROCKIT
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;
    msg.msgId = PBOX_ROCKIT_MUSIC_CHANNEL_VOLUME_EVT;
    msg.volume = volume;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
    #endif
}

static void rockit_pbbox_notify_energy(energy_info_t energy)
{
    #if ENABLE_RK_ROCKIT
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;
    msg.msgId = PBOX_ROCKIT_ENERGY_EVT;
    msg.energy_data = energy;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
    #endif
}

static enum rc_pb_play_src covert2rockitSource(input_source_t source) {
    enum rc_pb_play_src destSource = RC_PB_PLAY_SRC_BUTT;
    switch (source) {
        case SRC_USB: {
#if ENABLE_EXT_MCU_USB
            destSource = RC_PB_PLAY_SRC_BT;
#else
            destSource = RC_PB_PLAY_SRC_LOCAL;
#endif
        } break;
        case SRC_BT: {
            destSource = RC_PB_PLAY_SRC_BT; //recieve it from audiocard..
        } break;
#if ENABLE_UAC
        case SRC_UAC: {
            destSource = RC_PB_PLAY_SRC_UAC;
        } break;
#endif
#if ENABLE_AUX
        case SRC_AUX: {
            destSource = RC_PB_PLAY_SRC_BT;
        } break;
#endif
#if ENABLE_RAW_PCM
        case SRC_PCM: {
            destSource = RC_PB_PLAYER_SOURCE_PCM; //send raw pcm to rockit
        } break;
#endif
        default:
            break;
    }
    return destSource;
}

static void pbox_rockit_music_stop(input_source_t source)
{
    enum rc_pb_play_src dest = covert2rockitSource(source);
    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(partyboxCtx);
    assert(rc_pb_player_stop);

    printf("%s source:%d, started_player[%d]= %d, \n", __func__, source, dest, started_player[dest]);

    if(true == started_player[dest]) {
        started_player[dest] = false;
        rc_pb_player_stop(partyboxCtx, dest);
    }
}

static void pbox_rockit_music_local_start(const char *track_uri, const char *headers)
{
    struct rc_pb_player_attr playerAttr;
    enum rc_pb_play_src dest = covert2rockitSource(SRC_USB);

    assert(partyboxCtx);
    assert(rc_pb_player_start);
    memset(&playerAttr, 0, sizeof(playerAttr));
    playerAttr.url = track_uri;
    playerAttr.headers = headers;
    playerAttr.energy_band_cnt = 10;

    printf("%s :%s, ctx=%p\n", __func__, track_uri, partyboxCtx);
    pbox_rockit_music_stop(SRC_USB);
    rc_pb_player_start(partyboxCtx, RC_PB_PLAY_SRC_LOCAL, &playerAttr);

    started_player[dest] = true;
}
static bool vendor_started = false;
static void pbox_rockit_music_start_audiocard(input_source_t source, pbox_audioFormat_t audioFormat)
{
    struct rc_pb_player_attr playerAttr;
    int sampleFreq = audioFormat.sampingFreq;
    int channel = audioFormat.channel;
    char *cardName = audioFormat.cardName;
    enum rc_pb_play_src dest = covert2rockitSource(source);
    memset(&playerAttr, 0, sizeof(playerAttr));

    assert(dest != RC_PB_PLAY_SRC_BUTT);
    switch (sampleFreq) {
        case 0: {
            playerAttr.sample_rate = 48000;
        } break;
        default: {
            playerAttr.sample_rate = sampleFreq;
        } break;
    }

    switch (channel) {
        case 0: {
            playerAttr.channels = 2;
        } break;
        default: {
            playerAttr.channels = channel;
        } break;
    }

    playerAttr.bit_width = 16;
    playerAttr.card_name = cardName;
    assert(partyboxCtx);
    assert(rc_pb_player_start);

    printf("%s freq:%d, channel: %d, card:%s source:%d\n", __func__, sampleFreq, channel, cardName, source);
    #if 0//ENABLE_USE_SOCBT
    if(false == vendor_started) {
        vendor_started = true;
        rc_pb_player_start(partyboxCtx, dest, &playerAttr);
    }
    #else
    pbox_rockit_music_stop(source);
    rc_pb_player_start(partyboxCtx, dest, &playerAttr);
    #endif
    started_player[dest] = true;
    //set_vocal_separate_thread_cpu();
}

static void pbox_rockit_music_pause(input_source_t source)
{
    enum rc_pb_play_src dest = covert2rockitSource(source);
    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(partyboxCtx);
    assert(rc_pb_player_pause);

    printf("%s\n", __func__);
    rc_pb_player_pause(partyboxCtx, dest);
}

static void pbox_rockit_music_resume(input_source_t source, uint32_t volume)
{
    enum rc_pb_play_src dest = covert2rockitSource(source);
    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(partyboxCtx);
    assert(rc_pb_player_resume);

    printf("%s volume: %lld\n", __func__, volume);

    rc_pb_player_resume(partyboxCtx, dest);
}

static int64_t pbox_rockit_music_get_duration(input_source_t source) {
    rc_s64 duration = 0;
    enum rc_pb_play_src dest = covert2rockitSource(source);
    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(partyboxCtx);
    assert(rc_pb_player_get_duration);

    rc_pb_player_get_duration(partyboxCtx, dest, &duration);
    printf("%s duration: %lld\n", __func__, duration);

    return duration;
}

static int64_t pbox_rockit_music_get_position(input_source_t source) {
    rc_s64 position = 0;
    enum rc_pb_play_src dest = covert2rockitSource(source);
    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(partyboxCtx);
    assert(rc_pb_player_get_position);

    rc_pb_player_get_position(partyboxCtx, dest, &position);
    //printf("%s poststion: %lld\n", __func__, position);

    return position;
}

static void pbox_rockit_music_reverb_mode(uint8_t index, pbox_revertb_t mode) {
    struct rc_pb_param param;
    param.type = RC_PB_PARAM_TYPE_REVERB;

    assert(partyboxCtx);
    assert(rc_pb_recorder_set_param);
    rc_pb_recorder_get_param(partyboxCtx, index, &param);

    switch (mode) {
        case PBOX_REVERT_USER: {
            param.reverb.mode = RC_PB_REVERB_MODE_USER;
        } break;
        case PBOX_REVERT_STUDIO: {
            param.reverb.mode = RC_PB_REVERB_MODE_STUDIO;
        } break;
        case PBOX_REVERT_KTV: {
            param.reverb.mode = RC_PB_REVERB_MODE_KTV;
        } break;
        case PBOX_REVERT_CONCERT: {
            param.reverb.mode = RC_PB_REVERB_MODE_CONCERT;
        } break;
        case PBOX_REVERT_ECHO: {
            param.reverb.mode = RC_PB_REVERB_MODE_ECHO;
        } break;
        default: break;
    }

    #if ENABLE_USE_SOCBT
        param.reverb.mode = RC_PB_REVERB_MODE_KTV;
    #endif

    if (mode == RC_PB_REVERB_MODE_USER) 
        param.reverb.bypass = true;
    else 
        param.reverb.bypass = false;
    rc_pb_recorder_set_param(partyboxCtx, index, &param);
}

static void pbox_rockit_music_echo_reduction(uint8_t index, bool echo3a) {
    int ret;
    struct rc_pb_param param;
    bool on = echo3a;

    assert(rc_pb_recorder_set_param);
    assert(partyboxCtx);

    param.type = RC_PB_PARAM_TYPE_3A;
    param.howling.bypass = !on;
    ret = rc_pb_recorder_set_param(partyboxCtx, index, &param);
    printf("%s rc_pb_recorder_set_param 3a:%s res:%d\n" ,__func__, on?"on":"off", ret);
}

static void pbox_rockit_music_voice_seperate(input_source_t source, pbox_vocal_t vocal) {
    bool enable = vocal.enable;
    uint32_t hLevel = vocal.humanLevel;
    uint32_t aLevel = vocal.accomLevel;
    uint32_t rLevel = vocal.reservLevel;
    struct rc_pb_param param;
    enum rc_pb_play_src dest = covert2rockitSource(source);

    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(partyboxCtx);
    assert(rc_pb_player_get_param);
    assert(rc_pb_player_set_param);

    param.type = RC_PB_PARAM_TYPE_VOLCAL_SEPARATE;
    hLevel = hLevel>100?15 :hLevel;
    aLevel = aLevel>100?100:aLevel;
    rLevel = rLevel>100?100:rLevel;
    printf("%s hLevel:%d, aLevel:%d rLevel:%d , on:%d\n",__func__, hLevel, aLevel, rLevel, enable);

    int ret = rc_pb_player_get_param(partyboxCtx, dest, &param);

    if (enable)
        param.vocal.bypass = false;
    else
        param.vocal.bypass = true;
    param.vocal.human_level = hLevel;
    param.vocal.other_level = aLevel;
    param.vocal.reserve_level[0] = rLevel;
    ret = rc_pb_player_set_param(partyboxCtx, dest, &param);
    printf("%s rc_pb_player_set_param res:%d\n" ,__func__, ret);
}

float pbox_rockit_music_master_volume_get(input_source_t source) {
    enum rc_pb_play_src dest = covert2rockitSource(source);

    assert(dest != RC_PB_PLAY_SRC_BUTT);
    float volume = 0;
    assert(partyboxCtx);
    assert(rc_pb_get_volume);
    rc_pb_get_volume(partyboxCtx, &volume);

    return volume;
}

static float pbox_rockit_music_master_volume_adjust(input_source_t source, float Level) {
    enum rc_pb_play_src dest = covert2rockitSource(source);

    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(partyboxCtx);
    assert(rc_pb_set_volume);
    rc_pb_set_volume(partyboxCtx, Level);

    printf("%s source%d vol:%f\n" ,__func__, source, Level);
    return pbox_rockit_music_master_volume_get(source);
}

float pbox_rockit_music_channel_volume_get(input_source_t source) {
    enum rc_pb_play_src dest = covert2rockitSource(source);

    assert(dest != RC_PB_PLAY_SRC_BUTT);
    float volume = 0;
    assert(partyboxCtx);
    assert(rc_pb_player_get_volume);
    rc_pb_player_get_volume(partyboxCtx, dest, &volume);

    return volume;
}

static float pbox_rockit_music_channel_volume_adjust(input_source_t source, float Level) {
    enum rc_pb_play_src dest = covert2rockitSource(source);

    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(partyboxCtx);
    assert(rc_pb_player_set_volume);
    rc_pb_player_set_volume(partyboxCtx, dest, Level);

    printf("%s source%d vol:%f\n" ,__func__, source, Level);
    return pbox_rockit_music_channel_volume_get(source);
}

static void pbox_rockit_music_seek_set(input_source_t source, uint64_t usec) {
    enum rc_pb_play_src dest = covert2rockitSource(source);
    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(partyboxCtx);
    assert(rc_pb_player_seek);

    rc_pb_player_seek(partyboxCtx, dest, usec);
}

static void selectionSort(int length, int arr[]) {
    for (int i = 0; i < length/2; ++i) {
        int minIndex = i;
        for (int j = i + 1; j < length; ++j) {
            if (arr[j] < arr[minIndex]) {
                minIndex = j;
            }
        }

        // Swap arr[i] and arr[minIndex]
        int temp = arr[i];
        arr[i] = arr[minIndex];
        arr[minIndex] = temp;
    }
}
const char MAXTABLE[101] = {
/*00*/ 01, 02, 04, 06,  8, 10, 12, 14, 16, 18,
/*10*/ 20, 22, 24, 26, 28, 30, 32, 34, 36, 38,
/*20*/ 40, 42, 44, 46, 48, 50, 52, 54, 56, 58,
/*30*/ 60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
/*40*/ 70, 70, 71, 71, 72, 72, 73, 73, 74, 74,
/*50*/ 75, 75, 76, 76, 77, 77, 78, 78, 79, 79,
/*60*/ 80, 80, 81, 81, 82, 82, 83, 83, 84, 84,
/*70*/ 85, 85, 86, 86, 87, 87, 88, 88, 89, 89,
/*80*/ 90, 90, 91, 91, 92, 92, 93, 93, 94, 94,
/*90*/ 95, 95, 96, 96, 97, 97, 98, 98, 99, 100,
/*100*/100
};

static void mapDataToNewRange(int energyData[], int length, int nowMin, int nowMax) {
    // find the max value and the min value..
    int minVal = energyData[0];
    int maxVal = energyData[0];
    int minSum = 0;

    for (int i = 1; i < (length); ++i) {
        if (energyData[i] > maxVal) {
            maxVal = energyData[i];
        }
    }

    // Create a copy array for finding the minimum four values
    int copyData[length];
    memcpy(copyData, energyData, length * sizeof(int));
    selectionSort(length, copyData);

    for (int i = 0; i < length/2; ++i) {
        minSum += copyData[i];
    }

    int avgMin = minSum*2/length;
    minVal = avgMin;

    if(minVal == maxVal)
        return;

    // caculate the new min and max
    int newMin = minVal/3;
    int newMax = MAXTABLE[maxVal];

    // map to the new range of [newmin, newmax]
    for (int i = 0; i < length; ++i) {
        if (energyData[i] < avgMin) {
            energyData[i] = energyData[i]/2;
        }
        else {
            energyData[i] = (int)(((double)(energyData[i] - minVal) / (maxVal - minVal)) * (newMax - newMin) + newMin);
        }

        //if (energyData[i] > 100) energyData[i] = 100;
        //else if (energyData[i] < 1 )  energyData[i] = 1;
    }
}

static bool pbox_rockit_music_energyLevel_get(input_source_t source, energy_info_t* pEnergy) {
    struct rc_pb_energy energy;
    int energyData[10];
    static int energyDataPrev[10];
    static int energykeep[10];
    bool energy_debug = 0;
    enum rc_pb_play_src dest = covert2rockitSource(source);

    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(pEnergy);
    assert(partyboxCtx);
    assert(rc_pb_player_get_energy);
    assert(rc_pb_player_release_energy);

    int ret = rc_pb_player_get_energy(partyboxCtx, dest, &energy);
    if (!ret) {
        for (rc_s32 i = 0; i < sizeof(energyData)/sizeof(int); i++) {
            //translate energy range from [-90, 0] to [0, 100]
            energyData[i] = energy.energy_vec[10 + i] + 90;
            energyData[i] = energyData[i] + energyData[i]/10 + 1; //map to [1, 100]
            if(energy_debug) {
                printf("freq[%05d]HZ energy[%05d]DB energyData[%05d]\n",
                                energy.energy_vec[i], energy.energy_vec[10 + i], energyData[i]);
            }
        }

        mapDataToNewRange(energyData, sizeof(energyData)/sizeof(int), 0 , 100);

        for(rc_s32 i = 0; i < sizeof(energyData)/sizeof(int); i++) {
            if(abs(energyDataPrev[i] - energyData[i]) < 3){
                energykeep[i]++;
            } else {
                energykeep[i]=0;
            }

            if(energykeep[i] > 2) {
                energykeep[i]=0;
                energyData[i] = energyData[i]<5 ? energyData[i] : (energyData[i] - rand()%5);
            }
        }
        memcpy(energyDataPrev, energyData, sizeof(energyData)/sizeof(int) * sizeof(int));

        pEnergy->size = 10;
        for(int i = 0; i < pEnergy->size; i++) {
            pEnergy->energykeep[i].freq = energy.energy_vec[i];
            pEnergy->energykeep[i].energy = energyData[i];
        }

        rc_pb_player_release_energy(partyboxCtx, dest, &energy);
        return true;
    }
    return 0;
}



static void pbox_rockit_music_set_stereo_mode(input_source_t source, stereo_mode_t stereo) {
    enum rc_pb_play_src dest = covert2rockitSource(source);
    struct rc_pb_param param;

    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(partyboxCtx);
    printf("%s dest:%d, stereo:%d\n", __func__, dest, stereo);

    static rc_float sudioStereo;
    param.type = RC_PB_PARAM_TYPE_RKSTUDIO;
    param.rkstudio.data = (rc_float *)(&sudioStereo);
    switch (stereo) {
        case MODE_WIDEN: {
            param.rkstudio.addr = MUXES_SWITCH_STEREO_0_IDX_ADDR;
            param.rkstudio.cnt = 1;
            param.rkstudio.data[0] = 0;
            rc_pb_player_set_param(partyboxCtx, RC_PB_PLAY_SRC_BT, &param);

            param.rkstudio.addr = MUXES_SWITCH_STEREO_1_IDX_ADDR;
            param.rkstudio.cnt = 1;
            param.rkstudio.data[0] = 0;
            rc_pb_player_set_param(partyboxCtx, RC_PB_PLAY_SRC_BT, &param);

        } break;

        case MODE_STEREO: {
            param.rkstudio.addr = MUXES_SWITCH_STEREO_0_IDX_ADDR;
            param.rkstudio.cnt = 1;
            param.rkstudio.data[0] = 1;
            rc_pb_player_set_param(partyboxCtx, RC_PB_PLAY_SRC_BT, &param);

            param.rkstudio.addr = MUXES_SWITCH_STEREO_1_IDX_ADDR;
            param.rkstudio.cnt = 1;
            param.rkstudio.data[0] = 0;
            rc_pb_player_set_param(partyboxCtx, RC_PB_PLAY_SRC_BT, &param);
        } break;

        case MODE_MONO: {
            param.rkstudio.addr = MUXES_SWITCH_STEREO_1_IDX_ADDR;
            param.rkstudio.cnt = 1;
            param.rkstudio.data[0] = 1;
            rc_pb_player_set_param(partyboxCtx, RC_PB_PLAY_SRC_BT, &param);
        } break;
        default: break;
    }
}

static void pbox_rockit_music_set_inout_door(input_source_t source, inout_door_t outdoor) {
    enum rc_pb_play_src dest = covert2rockitSource(source);

    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(partyboxCtx);

    printf("%s:%d\n", __func__, outdoor);

}

static void pbox_rockit_music_set_placement(input_source_t source, placement_t place) {
    enum rc_pb_play_src dest = covert2rockitSource(source);
    struct rc_pb_param param;

    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(partyboxCtx);
    printf("%s dest:%d, place:%d\n", __func__, dest, place);

    static rc_float sudioPlace;
    param.type = RC_PB_PARAM_TYPE_RKSTUDIO;
    param.rkstudio.data = (rc_float *)(&sudioPlace);
    switch (place) {
        case PLACE_AUTO:
        case PLACE_HORI: {
            param.rkstudio.addr = MUXES_SWITCH_MONO_0_IDX_ADDR;
            param.rkstudio.cnt = 1;
            param.rkstudio.data[0] = 0;
            rc_pb_player_set_param(partyboxCtx, RC_PB_PLAY_SRC_BT, &param);

            param.rkstudio.addr = MUXES_SWITCH_MONO_1_IDX_ADDR;
            param.rkstudio.cnt = 1;
            param.rkstudio.data[0] = 0;
            rc_pb_player_set_param(partyboxCtx, RC_PB_PLAY_SRC_BT, &param);
        } break;

        case PLACE_VERT: {
            param.rkstudio.addr = MUXES_SWITCH_MONO_0_IDX_ADDR;
            param.rkstudio.cnt = 1;
            param.rkstudio.data[0] = 1;
            rc_pb_player_set_param(partyboxCtx, RC_PB_PLAY_SRC_BT, &param);

            param.rkstudio.addr = MUXES_SWITCH_MONO_1_IDX_ADDR;
            param.rkstudio.cnt = 1;
            param.rkstudio.data[0] = 1;
            rc_pb_player_set_param(partyboxCtx, RC_PB_PLAY_SRC_BT, &param);
        } break;
        default: break;
    }
}

static void pbox_rockit_music_mic_volume_adjust(uint8_t index, float micLevel) {
    assert(partyboxCtx);
    assert(rc_pb_recorder_set_volume);
    rc_pb_recorder_set_volume(partyboxCtx, index, micLevel);
}

static void pbox_rockit_music_mic_mute(uint8_t index, bool mute) {
    assert(partyboxCtx);
    assert(rc_pb_recorder_mute);
    rc_pb_recorder_mute(partyboxCtx, index, mute);
    printf("%s: %s\n", __func__, mute?"on":"off");
}

static void pbox_rockit_set_mic_treble(uint8_t index, float treble) {
    struct rc_pb_param param;
    param.type = RC_PB_PARAM_TYPE_EQDRC;
    param.eqdrc.bypass = false;
    param.eqdrc.eq = NULL;

    assert(partyboxCtx);
    assert(rc_pb_recorder_set_param);
    printf("%s index:%d, treble:%f\n", __func__, index, treble);

    if (true) {
        static struct rc_pb_param_eq eq;
        eq.ch = index;
        eq.filter = 0;
        eq.enable = 1;
        eq.type = 1;
        eq.fc = 1000;
        eq.q = 1.7;
        eq.boost = treble; // tunning
        param.eqdrc.eq = &eq;
    }
    rc_pb_recorder_set_param(partyboxCtx, index, &param);
}

static void pbox_rockit_set_mic_bass(uint8_t index, float bass) {
    struct rc_pb_param param;
    param.type = RC_PB_PARAM_TYPE_EQDRC;
    param.eqdrc.bypass = false;
    param.eqdrc.eq = NULL;

    assert(partyboxCtx);
    assert(rc_pb_recorder_set_param);
    printf("%s index:%d, bass:%f\n", __func__, index, bass);

    if (true) {
        static struct rc_pb_param_eq eq;
        eq.ch = index;
        eq.filter = 0;
        eq.enable = 1;
        eq.type = 1;
        eq.fc = 500;
        eq.q = 1;
        eq.boost = bass; // tunning
        param.eqdrc.eq = &eq;
    }
    rc_pb_recorder_set_param(partyboxCtx, index, &param);
}

static void pbox_rockit_set_mic_reverb(uint8_t index, float reverb) {
    struct rc_pb_param param;
    param.type = RC_PB_PARAM_TYPE_REVERB;

    assert(partyboxCtx);
    assert(rc_pb_player_set_param);
    assert(rc_pb_recorder_get_param);

    memset(&param, 0, sizeof(struct rc_pb_param));
    printf("%s index:%d, reverb:%f\n", __func__, index, reverb);
    rc_pb_recorder_get_param(partyboxCtx, index, &param);
    printf("%s got type:%d, bypass:%d, dry:%d, wet:%d\n", \
                __func__, param.type, param.reverb.bypass, param.reverb.dry_level, param.reverb.wet_level);

    if (param.reverb.mode != RC_PB_REVERB_MODE_STUDIO && \
        param.reverb.mode != RC_PB_REVERB_MODE_KTV && \
        param.reverb.mode != RC_PB_REVERB_MODE_CONCERT) {
        param.reverb.mode = RC_PB_REVERB_MODE_KTV;
    }

    #if ENABLE_USE_SOCBT
        param.reverb.mode = RC_PB_REVERB_MODE_KTV;
    #endif

    param.type = RC_PB_PARAM_TYPE_REVERB;
    param.reverb.bypass = false;
    param.reverb.dry_level = 80;
    param.reverb.wet_level = reverb;
    rc_pb_recorder_set_param(partyboxCtx, index, &param);
}

static void pbox_rockit_music_mic_set_parameter(pbox_rockit_msg_t *msg) {
    uint8_t index = msg->micdata.index;
    mic_set_kind_t  kind = msg->micdata.kind;
    mic_mux_t micMux = msg->micdata.micState.micMux;
    float micVolume = msg->micdata.micState.micVolume;
    float micTreble = msg->micdata.micState.micTreble;
    float micBass = msg->micdata.micState.micBass;
    float micReverb = msg->micdata.micState.micReverb;
    bool echo3a = msg->micdata.micState.echo3a;
    bool micmute = msg->micdata.micState.micmute;
    pbox_revertb_t  reverbMode = msg->micdata.micState.reverbMode;;

    printf("%s: index:%d, kind:%d, micMux:%d, volume: %f, treble:%f, bass:%f, reverb:%f\n",
        __func__, index, kind, micMux, micVolume, micTreble, micBass, micReverb);

    if (MIC_SET_DEST_ALL == kind) {
        pbox_rockit_music_mic_mute(index, micmute);
        //pbox_rockit_music_mic_mute(index, micMux == MIC_OFF? true: false);
        pbox_rockit_music_echo_reduction(index, echo3a);
        pbox_rockit_music_mic_volume_adjust(index, micVolume);
        pbox_rockit_set_mic_treble(index, micTreble);
        pbox_rockit_set_mic_bass(index, micBass);
        pbox_rockit_set_mic_reverb(index, micReverb);
        pbox_rockit_music_reverb_mode(index, reverbMode);//keep it after pbox_rockit_set_mic_reverb.
        return;
    }

    switch(kind) {
        case MIC_SET_DEST_ECHO_3A: {
            pbox_rockit_music_echo_reduction(index, echo3a);
        } break;
        case MIC_SET_DEST_MUTE: {
            pbox_rockit_music_mic_mute(index, micmute);
        } break;
        case MIC_SET_DEST_MUX: {
            pbox_rockit_music_mic_mute(index, micMux == MIC_OFF? true: false);
        } break;
        case MIC_SET_DEST_REVERB_MODE: {
            pbox_rockit_music_reverb_mode(index, reverbMode);
        } break;
        case MIC_SET_DEST_VOLUME: {
            pbox_rockit_music_mic_volume_adjust(index, micVolume);
        } break;
        case MIC_SET_DEST_TREBLE: {
            pbox_rockit_set_mic_treble(index, micTreble);
        } break;
        case MIC_SET_DEST_BASS: {
            pbox_rockit_set_mic_bass(index, micBass);
        } break;
        case MIC_SET_DEST_REVERB: {
            pbox_rockit_set_mic_reverb(index, micReverb);
        } break;
        default: break;
    }
}

static void pbox_rockit_music_destroy(void) {
    for(int i = 0; i < SRC_NUM; i++)
        pbox_rockit_music_stop(i);
    assert(rc_pb_recorder_stop);
    assert(rc_pb_destroy);
    rc_pb_recorder_stop(partyboxCtx);
    rc_pb_destroy(partyboxCtx);

    printf("destroy karaoke player\n");
}

static void pbox_rockit_uac_set_state(pbox_rockit_msg_t *msg) {
    uac_role_t role = msg->uac.uac_role;
    bool start = msg->uac.state;
}

static void pbox_rockit_uac_set_freq(pbox_rockit_msg_t *msg) {
    uac_role_t role = msg->uac.uac_role;
    uint32_t freq = msg->uac.sampleFreq;
}

static void pbox_rockit_uac_set_volume(pbox_rockit_msg_t *msg) {
    uac_role_t role = msg->uac.uac_role;
    uint32_t volume = msg->uac.volume;
    if(role == UAC_ROLE_SPEAKER) {
#if ENABLE_UAC
        pbox_rockit_music_master_volume_adjust(SRC_UAC, volume);
#endif
    }
    else if(role == UAC_ROLE_RECORDER) {
        pbox_rockit_music_mic_volume_adjust(0, volume);
    }
}

static void pbox_rockit_uac_set_mute(pbox_rockit_msg_t *msg) {
    uac_role_t role = msg->uac.uac_role;
    bool mute = msg->uac.mute;
    if(role == UAC_ROLE_SPEAKER) {}
    else if(role == UAC_ROLE_RECORDER) {}
}

#define MIC_SPK_SOUNDCARD_INDEX 0
static void pbox_rockit_uac_set_ppm(pbox_rockit_msg_t *msg) {
    char str[64] = {0};
    uac_role_t role = msg->uac.uac_role;
    int32_t ppm = msg->uac.ppm;
    struct rc_pb_param param;
    enum rc_pb_play_src dest = covert2rockitSource(msg->source);
    assert(dest != RC_PB_PLAY_SRC_BUTT);
    assert(rc_pb_player_set_param);

    snprintf(str, sizeof(str), "%d", ppm);
    param.type = RC_PB_PARAM_TYPE_AMIX;
    param.amix.card = MIC_SPK_SOUNDCARD_INDEX;
    param.amix.control = "PCM Clk Compensation In PPM";
    param.amix.values = str;

    printf("%s ppm:%d\n", __func__, ppm);
    rc_pb_player_set_param(partyboxCtx, dest, &param);
}

#define MIN_ROCKIT_TIMER_INTER 50
#if ENABLE_RK_ROCKIT
static void *pbox_rockit_server(void *arg)
{
    int rockit_fds[1] = {0};
    int maxfd, i;
    char buff[sizeof(pbox_rockit_msg_t)] = {0};
    pbox_rockit_msg_t *msg;
    pthread_setname_np(pthread_self(), "party_rockit");

    rk_demo_music_create();

    int sock_fd = get_server_socketpair_fd(PBOX_SOCKPAIR_ROCKIT);

    if(sock_fd < 0)
        return (void *)-1;
    
    rockit_fds[0] = sock_fd;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    for (i= 0, maxfd = rockit_fds[0]; i < sizeof(rockit_fds)/sizeof(int); i++) {
        FD_SET(rockit_fds[i], &read_fds);
        if (maxfd < rockit_fds[i])
            maxfd = rockit_fds[i];
    }

    while(true) {
        fd_set read_set = read_fds;

        int result = select(maxfd+1, &read_set, NULL, NULL, NULL);
        if ((result == 0) || (result < 0 && (errno != EINTR))) {
            printf("select timeout");
            continue;
        }

        if(result < 0) {
            break;
        }

        int ret = recv(sock_fd, buff, sizeof(buff), 0);

        if (ret <= 0)
            continue;

        pbox_rockit_msg_t *msg = (pbox_rockit_msg_t *)buff;
        if(msg->msgId != PBOX_ROCKIT_GET_PLAYERENERGYLEVEL && msg->msgId != PBOX_ROCKIT_GET_PLAYERCURRENTPOSITION)
        printf("%s recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

        if(msg->type == PBOX_EVT)
            continue;

        switch (msg->msgId) {
            case PBOX_ROCKIT_DESTROY: {
                pbox_rockit_music_destroy();
            } break;

            case PBOX_ROCKIT_START_LOCAL_PLAYER: {
                char *track_path = (char *)msg->dataSource.track_uri;
                if(strlen(track_path) == 0)
                    break;
                pbox_rockit_music_local_start(track_path, NULL);
            } break;

            case PBOX_ROCKIT_START_AUDIOCARD_PLAYER: {
                pbox_rockit_music_start_audiocard(msg->source, msg->audioFormat);
            } break;

            case PBOX_ROCKIT_PAUSE_PLAYER: {
                pbox_rockit_music_pause(msg->source);
            } break;

            case PBOX_ROCKIT_RESUME_PLAYER: {
                int volume = msg->volume;
                pbox_rockit_music_resume(msg->source, volume);
            } break;

            case PBOX_ROCKIT_STOP_PLAYER: {
                #if 1//ENABLE_USE_SOCBT == 0
                pbox_rockit_music_stop(msg->source);
                #endif
            } break;

            case PBOX_ROCKIT_GET_PLAYERCURRENTPOSITION: {
                uint32_t position = (uint32_t)(pbox_rockit_music_get_position(msg->source)/1000);
                rockit_pbbox_notify_current_postion(position);
            } break;

            case PBOX_ROCKIT_GET_PLAYERDURATION: {
                int64_t duration = pbox_rockit_music_get_duration(msg->source);
                rockit_pbbox_notify_duration(duration/1000);
            } break;

            case PBOX_ROCKIT_SET_PLAYERLOOPING: {
                //pending
            } break;

            case PBOX_ROCKIT_SET_PLAYERSEEKTO: {
                uint64_t seek = msg->mPosition*1000;
                pbox_rockit_music_seek_set(msg->source, seek);
            } break;

            case PBOX_ROCKIT_SET_MAINVOLUME: {
                float volume = msg->volume;
                printf("%s PBOX_ROCKIT_SET_MAINVOLUME volume:%f\n" ,__func__, volume);
                float vol_old = pbox_rockit_music_master_volume_adjust(msg->source, volume);
                if (volume != vol_old) {
                    rockit_pbbox_notify_volume(volume);
                }
            } break;

            case PBOX_ROCKIT_GET_MAINVOLUME: {
                uint32_t volume = pbox_rockit_music_master_volume_get(msg->source);
                rockit_pbbox_notify_volume(volume);
            } break;

            case PBOX_ROCKIT_GET_MUSICVOLUME: {
                uint32_t volume = pbox_rockit_music_channel_volume_get(msg->source);
                rockit_pbbox_notify_music_volume(volume);
            } break;

            case PBOX_ROCKIT_SET_MUSICVOLUME: {
                float volume = msg->volume;
                float vol_old = pbox_rockit_music_channel_volume_adjust(msg->source, volume);
                if (volume != vol_old) {
                    rockit_pbbox_notify_music_volume(volume);
                }
            } break;

            case PBOX_ROCKIT_SET_PLAYER_SEPERATE: {
                pbox_vocal_t vocal = msg->vocalSeperate;
                pbox_rockit_music_voice_seperate(msg->source, vocal);
            } break;

            case PBOX_ROCKIT_GET_PLAYER_SEPERATE: {
                //pending
            } break;

            case PBOX_ROCKIT_GET_PLAYERENERGYLEVEL: {
                energy_info_t energy;
                if(pbox_rockit_music_energyLevel_get(msg->source, &energy)) {
                    rockit_pbbox_notify_energy(energy);
                }
            } break;

            case PBOX_ROCKIT_SET_MIC_STATE: {
                pbox_rockit_music_mic_set_parameter(msg);
            } break;

            case PBOX_ROCKIT_SET_STEREO_MODE: {
                pbox_rockit_music_set_stereo_mode(msg->source, msg->stereo);
            } break;

            case PBOX_ROCKIT_SET_OUTDOOR_MODE: {
                pbox_rockit_music_set_inout_door(msg->source, msg->outdoor);
            } break;

            case PBOX_ROCKIT_SET_PLACEMENT_MODE: {
                pbox_rockit_music_set_placement(msg->source, msg->place);
            } break;

            case PBOX_ROCKIT_SET_UAC_STATE: {
                pbox_rockit_uac_set_state(msg);
            } break;
            case PBOX_ROCKIT_SET_UAC_SAMPLE_RATE: {
                pbox_rockit_uac_set_freq(msg);
            } break;
            case PBOX_ROCKIT_SET_UAC_VOLUME: {
                pbox_rockit_uac_set_volume(msg);
            } break;
            case PBOX_ROCKIT_SET_UAC_MUTE: {
                pbox_rockit_uac_set_mute(msg);
            } break;
            case PBOX_ROCKIT_SET_UAC_PPM: {
                pbox_rockit_uac_set_ppm(msg);
            } break;

            default: {
            } break;
        }

        if(msg->msgId != PBOX_ROCKIT_GET_PLAYERENERGYLEVEL && msg->msgId != PBOX_ROCKIT_GET_PLAYERCURRENTPOSITION)
            printf("%s end: type: %d, id: %d\n", __func__, msg->type, msg->msgId);
    }

    pbox_rockit_music_destroy();
}

int pbox_create_rockitTask(void)
{
    int ret;

    ret = pthread_create(&rockit_task_id, NULL, pbox_rockit_server, NULL);
    if (ret < 0)
    {
        printf("btsink server start failed\n");
    }

    return ret;
}
#endif

void pb_rockit_notify(enum rc_pb_event event, rc_s32 cmd, void *opaque) {
    printf("event: %d, cmd: %d\n", event, cmd);
    switch (event) {
        case RC_PB_EVENT_PLAYBACK_ERROR:
        case RC_PB_EVENT_PLAYBACK_COMPLETE: {
            rockit_pbbox_notify_playback_status(event);
        } break;
        case RC_PB_EVENT_AWAKEN: {
            rockit_pbbox_notify_awaken(cmd);
        }
        break;
        default:
        printf("Unknown event: %d", event);
    }
}
