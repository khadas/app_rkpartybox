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
#include "rk_comm_karaoke.h"
#include "pbox_common.h"
#include "pbox_rockit.h"
#include "pbox_socket.h"


static void karaoke_callback(RK_VOID *pPrivateData, KARAOKE_EVT_E event, RK_S32 ext1, RK_VOID *ptr);

pthread_t rockit_task_id;
void *player_ctx = NULL;
void *mpi_hdl = NULL;
RK_S32 (*RK_MPI_KARAOKE_Create_func)(void **ctx, KARAOKE_ATTR_S *attr);
RK_S32 (*RK_MPI_KARAOKE_Destroy_func)(void *ctx);
RK_S32 (*RK_MPI_KARAOKE_SetDataSource_func)(void *ctx, const char *url, const char *headers);
RK_S32 (*RK_MPI_KARAOKE_StartPlayer_func)(void *ctx);
RK_S32 (*RK_MPI_KARAOKE_PausePlayer_func)(void *ctx);
RK_S32 (*RK_MPI_KARAOKE_ResumePlayer_func)(void *ctx);
RK_S32 (*RK_MPI_KARAOKE_StopPlayer_func)(void *ctx);
RK_S32 (*RK_MPI_KARAOKE_StartRecorder_func)(void *ctx);
RK_S32 (*RK_MPI_KARAOKE_StopRecorder_func)(void *ctx);
RK_S32 (*RK_MPI_KARAOKE_GetPlayerDuration_func)(void *ctx, RK_S64 *usec);
RK_S32 (*RK_MPI_KARAOKE_SetPlayerLooping_func)(void *ctx, RK_BOOL loop);
RK_S32 (*RK_MPI_KARAOKE_SetPlayerSeekTo_func)(void *ctx, RK_S64 usec);
RK_S32 (*RK_MPI_KARAOKE_SetPlayerVolume_func)(void *ctx, RK_U32 volume);
RK_S32 (*RK_MPI_KARAOKE_GetPlayerVolume_func)(void *ctx, RK_U32 *volume);
RK_S32 (*RK_MPI_KARAOKE_GetPlayerCurrentPosition_func)(void *ctx, RK_S64 *usec);
RK_S32 (*RK_MPI_KARAOKE_GetPlayerParam_func)(void *ctx, KARAOKE_PARAM_S *param);
RK_S32 (*RK_MPI_KARAOKE_SetPlayerParam_func)(void *ctx, KARAOKE_PARAM_S *param);
RK_S32 (*RK_MPI_KARAOKE_SetRecorderParam_func)(void *ctx, KARAOKE_PARAM_S *param);
RK_S32 (*RK_MPI_KARAOKE_GetRecorderParam_func)(void *ctx, KARAOKE_PARAM_S *param);
RK_S32 (*RK_MPI_KARAOKE_SetRecorderVolume_func)(void *ctx, RK_U32 volume);
RK_S32 (*RK_MPI_KARAOKE_GetRecorderVolume_func)(void *ctx, RK_U32 *volume);
RK_S32 (*RK_MPI_KARAOKE_GetPlayerEnergyLevel_func)(void *ctx, KARAOKE_ENERGY_LEVEL_S *energy);
RK_S32 (*RK_MPI_KARAOKE_ReleasePlayerEnergyLevel_func)(void *ctx, KARAOKE_ENERGY_LEVEL_S *energy);
RK_S32 (*RK_MPI_KARAOKE_StartBTPlayer_func)(void *ctx, KARAOKE_BT_ATTR_S *attr);
RK_S32 (*RK_MPI_KARAOKE_StopBTPlayer_func)(void *ctx);



int rk_demo_music_create() {
    //create karaoke recorder && player
    KARAOKE_ATTR_S attr;
    attr.pRecorderCardName = "hw:0,0";
    attr.pPlayerCardName = "hw:0,0";
    attr.u32PlayerEnergyBandCnt = 10;
    attr.u32AIChannels = 4;
    attr.u32AOChannels = 2;
    attr.u32ChnLayout = 0x0f;
    attr.u32RecLayout = 0x04;
    attr.u32RefLayout = 0x03;
    attr.callback = karaoke_callback;

    mpi_hdl = dlopen("librockit.so", RTLD_LAZY);
    if (NULL == mpi_hdl) {
        printf("failed to open librockit.so, err:%s\n", dlerror());
        return -1;
    }

    if (mpi_hdl != NULL) {
         RK_MPI_KARAOKE_Create_func = (RK_S32 (*)(void **ctx, KARAOKE_ATTR_S *attr))dlsym(mpi_hdl, "RK_MPI_KARAOKE_Create");
	 if (NULL == RK_MPI_KARAOKE_Create_func) {
            printf("failed to open func, err=%s\n", dlerror());
            return -1;
	 }
         RK_MPI_KARAOKE_Destroy_func = (RK_S32 (*)(void *ctx))dlsym(mpi_hdl, "RK_MPI_KARAOKE_Destroy");
         if (NULL == RK_MPI_KARAOKE_Destroy_func) {
             printf("failed to open  func, err=%s\n", dlerror());
             return -1;
         }
         RK_MPI_KARAOKE_SetDataSource_func = (RK_S32 (*)(void *ctx, const char *url, const char *header))
	                            dlsym(mpi_hdl, "RK_MPI_KARAOKE_SetDataSource");
         if (NULL == RK_MPI_KARAOKE_SetDataSource_func) {
             printf("failed to open  func, err=%s\n", dlerror());
             return -1;
         }
         RK_MPI_KARAOKE_StartPlayer_func = (RK_S32 (*)(void *ctx))dlsym(mpi_hdl, "RK_MPI_KARAOKE_StartPlayer");
         if (NULL == RK_MPI_KARAOKE_StartPlayer_func) {
             printf("failed to open  func, err=%s\n", dlerror());
             return -1;
         }
         RK_MPI_KARAOKE_PausePlayer_func = (RK_S32 (*)(void *ctx))dlsym(mpi_hdl, "RK_MPI_KARAOKE_PausePlayer");
         if (NULL == RK_MPI_KARAOKE_PausePlayer_func) {
             printf("failed to open  func, err=%s\n", dlerror());
             return -1;
         }  
         RK_MPI_KARAOKE_ResumePlayer_func = (RK_S32 (*)(void *ctx))dlsym(mpi_hdl, "RK_MPI_KARAOKE_ResumePlayer");
         if (NULL == RK_MPI_KARAOKE_ResumePlayer_func) {
             printf("failed to open  func, err=%s\n", dlerror());
             return -1;
         }
         RK_MPI_KARAOKE_StopPlayer_func = (RK_S32 (*)(void *ctx))dlsym(mpi_hdl, "RK_MPI_KARAOKE_StopPlayer");
         if (NULL == RK_MPI_KARAOKE_StopPlayer_func) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
         }  
	 RK_MPI_KARAOKE_StartRecorder_func = (RK_S32 (*)(void *ctx))dlsym(mpi_hdl, "RK_MPI_KARAOKE_StartRecorder");
         if (NULL == RK_MPI_KARAOKE_StartRecorder_func) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
         }
	 RK_MPI_KARAOKE_StopRecorder_func = (RK_S32 (*)(void *ctx))dlsym(mpi_hdl, "RK_MPI_KARAOKE_StopRecorder");
         if (NULL == RK_MPI_KARAOKE_StopRecorder_func) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
         }

	 RK_MPI_KARAOKE_GetPlayerCurrentPosition_func = (RK_S32 (*)(void *ctx, RK_S64 *usec))dlsym(mpi_hdl, 
			                                 "RK_MPI_KARAOKE_GetPlayerCurrentPosition");
	 if (NULL == RK_MPI_KARAOKE_GetPlayerCurrentPosition_func) {
	    printf("failed to open  func, err=%s\n", dlerror());
	    return -1;
         }

	 RK_MPI_KARAOKE_GetPlayerDuration_func = (RK_S32 (*)(void *ctx,  RK_S64 *usec))dlsym(mpi_hdl, 
			                                 "RK_MPI_KARAOKE_GetPlayerDuration");
	 if (NULL == RK_MPI_KARAOKE_GetPlayerDuration_func) {
            printf("failed to open  func, err=%s\n", dlerror());
	    return -1;
         }

	 RK_MPI_KARAOKE_SetPlayerLooping_func = (RK_S32 (*)(void *ctx))dlsym(mpi_hdl, "RK_MPI_KARAOKE_SetPlayerLooping");
         if (NULL == RK_MPI_KARAOKE_SetPlayerLooping_func) {
            printf("failed to open  func, err=%s\n", dlerror());
	    return -1;
         }

    RK_MPI_KARAOKE_SetPlayerSeekTo_func = (RK_S32 (*)(void *ctx,  RK_S64 usec))dlsym(mpi_hdl, "RK_MPI_KARAOKE_SetPlayerSeekTo");
         if (NULL == RK_MPI_KARAOKE_SetPlayerSeekTo_func) {
            printf("failed to open  func, err=%s\n", dlerror());
	    return -1;
         }

	 RK_MPI_KARAOKE_SetPlayerVolume_func = (RK_S32 (*)(void *ctx, RK_U32 volume))dlsym(mpi_hdl, 
			                                 "RK_MPI_KARAOKE_SetPlayerVolume");
         if (NULL == RK_MPI_KARAOKE_SetPlayerVolume_func) {
            printf("failed to open  func, err=%s\n", dlerror());
	    return -1;
         }
	 RK_MPI_KARAOKE_GetPlayerVolume_func = (RK_S32 (*)(void *ctx,  RK_U32 *volume))dlsym(mpi_hdl, 
			                                 "RK_MPI_KARAOKE_GetPlayerVolume");
         if (NULL == RK_MPI_KARAOKE_GetPlayerDuration_func) {
            printf("failed to open  func, err=%s\n", dlerror());
	    return -1;
         }
	 RK_MPI_KARAOKE_GetPlayerParam_func = (RK_S32 (*)(void *ctx, KARAOKE_PARAM_S *param))dlsym(mpi_hdl,
			                                 "RK_MPI_KARAOKE_GetPlayerParam");
	 if (NULL == RK_MPI_KARAOKE_GetPlayerParam_func) {
	    printf("failed to open func, err=%s\n", dlerror());
	    return -1;
	 }
	 RK_MPI_KARAOKE_SetPlayerParam_func = (RK_S32 (*)(void *ctx, KARAOKE_PARAM_S *param))dlsym(mpi_hdl, 
			                                 "RK_MPI_KARAOKE_SetPlayerParam");
         if (NULL == RK_MPI_KARAOKE_GetPlayerDuration_func) {
            printf("failed to open  func, err=%s\n", dlerror());
	    return -1;
         }
	 RK_MPI_KARAOKE_SetRecorderParam_func = (RK_S32 (*)(void *ctx, KARAOKE_PARAM_S *param))dlsym(mpi_hdl, 
			                                 "RK_MPI_KARAOKE_SetRecorderParam");
	 if (NULL == RK_MPI_KARAOKE_SetRecorderParam_func) {
	    printf("failed to open  func, err=%s\n", dlerror());
	    return -1;
         }
	 RK_MPI_KARAOKE_GetRecorderParam_func = (RK_S32 (*)(void *ctx, KARAOKE_PARAM_S *param))dlsym(mpi_hdl, 
			                                 "RK_MPI_KARAOKE_GetRecorderParam");
         if (NULL == RK_MPI_KARAOKE_GetRecorderParam_func) {
            printf("failed to open  func, err=%s\n", dlerror());
            return -1;
         }

	 RK_MPI_KARAOKE_SetRecorderVolume_func =  (RK_S32 (*)(void *ctx, KARAOKE_PARAM_S *param))dlsym(mpi_hdl,
			                                  "RK_MPI_KARAOKE_SetRecorderVolume");
	 if (NULL == RK_MPI_KARAOKE_SetRecorderVolume_func) {
	    printf("failed to open func, err=%s",dlerror());
	    return -1;
         }
         RK_MPI_KARAOKE_GetRecorderVolume_func =  (RK_S32 (*)(void *ctx, KARAOKE_PARAM_S *param))dlsym(mpi_hdl,
			                                  "RK_MPI_KARAOKE_GetRecorderVolume");
	 if (NULL == RK_MPI_KARAOKE_GetRecorderVolume_func) {
            printf("failed to open func, err=%s", dlerror());
	    return -1;
	 }

         RK_MPI_KARAOKE_GetPlayerEnergyLevel_func = (RK_S32 (*)(void *ctx, KARAOKE_ENERGY_LEVEL_S *energy))dlsym(mpi_hdl, 
			                                 "RK_MPI_KARAOKE_GetPlayerEnergyLevel");
	 if (NULL == RK_MPI_KARAOKE_GetPlayerEnergyLevel_func) {
	    printf("failed to open func, err=%s\n", dlerror());
	    return -1;
	 }
	 RK_MPI_KARAOKE_ReleasePlayerEnergyLevel_func = (RK_S32 (*)(void *ctx, KARAOKE_ENERGY_LEVEL_S *energy))dlsym(mpi_hdl,
                                                         "RK_MPI_KARAOKE_ReleasePlayerEnergyLevel");
         if (NULL == RK_MPI_KARAOKE_ReleasePlayerEnergyLevel_func) {
            printf("failed to open func, err=%s\n", dlerror());
	    return -1;
         }

	 RK_MPI_KARAOKE_StartBTPlayer_func = (RK_S32 (*)(void *ctx, KARAOKE_BT_ATTR_S *attr))dlsym(mpi_hdl,
                                                         "RK_MPI_KARAOKE_StartBTPlayer");
         if (NULL == RK_MPI_KARAOKE_StartBTPlayer_func) {
            printf("failed to open func, err=%s, line:%d\n", dlerror(), __LINE__);
	    return -1;
         }

	 RK_MPI_KARAOKE_StopBTPlayer_func = (RK_S32 (*)(void *ctx))dlsym(mpi_hdl,
                                                         "RK_MPI_KARAOKE_StopBTPlayer");
         if (NULL == RK_MPI_KARAOKE_StopBTPlayer_func) {
            printf("failed to open func, err=%s, line:%d\n", dlerror(), __LINE__);
	    return -1;
         }
    }

    if (RK_MPI_KARAOKE_Create_func(&player_ctx, &attr) != 0) {
        printf("RK_MPI_KARAOKE_Create_func failed, err!!!\n");
	return -1;
    }

    if (RK_MPI_KARAOKE_StartRecorder_func(player_ctx) != 0) {
	printf("RK_MPI_KARAOKE_StartRecorder_func failed, err!!!\n");
	return -1;
    }

    printf("rockit media player created successfully, player_ctx=%p\n", player_ctx);
}

static void rockit_pbbox_notify_awaken(uint32_t wakeCmd)
{
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;
    msg.msgId = PBOX_ROCKIT_AWAKEN_EVT;
    msg.wake_up.wakeCmd = wakeCmd;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
}

static void rockit_pbbox_notify_playback_status(uint32_t ext1)
{
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;

    if(ext1 == KARAOKE_EVT_PLAYBACK_COMPLETE || KARAOKE_EVT_PLAYBACK_ERROR == KARAOKE_EVT_PLAYBACK_ERROR)
        msg.msgId = PBOX_ROCKIT_PLAY_COMPLETED_EVT;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
}

//before call this func, duration shoud covert to ms(msecond), not us.
static void rockit_pbbox_notify_duration(uint32_t duration)
{
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;
    msg.msgId = PBOX_ROCKIT_MUSIC_DURATION_EVT;
    msg.duration = duration;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
}

//before call this func, duration shoud covert to ms(msecond), not us.
static void rockit_pbbox_notify_current_postion(uint32_t current)
{
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;
    msg.msgId = PBOX_ROCKIT_MUSIC_POSITION_EVT;
    msg.mPosition = current;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
}

static void rockit_pbbox_notify_volume(uint32_t volume)
{
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;
    msg.msgId = PBOX_ROCKIT_MUSIC_VOLUME_EVT;
    msg.volume = volume;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
}

static void rockit_pbbox_notify_energy(energy_info_t energy)
{
    pbox_rockit_msg_t msg = {0};
    msg.type = PBOX_EVT;
    msg.msgId = PBOX_ROCKIT_ENERGY_EVT;
    msg.energy_data = energy;

    unix_socket_notify_msg(PBOX_MAIN_ROCKIT, &msg, sizeof(pbox_rockit_msg_t));
}

static void pbox_rockit_music_setDataSource(const char *track_uri, const char *headers)
{
    assert(player_ctx);
    assert(RK_MPI_KARAOKE_SetDataSource_func);

    printf("%s :%s, ctx=%p\n", __func__, track_uri, player_ctx);
    RK_MPI_KARAOKE_SetDataSource_func(player_ctx, track_uri, headers);  
}


static void pbox_rockit_music_stop_bt(void)
{
    printf("%s:%p, %p\n", __func__, player_ctx, RK_MPI_KARAOKE_StopBTPlayer_func);
    assert(player_ctx);
    assert(RK_MPI_KARAOKE_StopBTPlayer_func);

    RK_MPI_KARAOKE_StopBTPlayer_func(player_ctx);
}

static void pbox_rockit_music_start(void)
{
    assert(player_ctx);
    assert(RK_MPI_KARAOKE_StartPlayer_func);

    printf("%s\n", __func__);
    RK_MPI_KARAOKE_StartPlayer_func(player_ctx);
    set_vocal_separate_thread_cpu();
}

static void pbox_rockit_music_stop(void) {
    assert(player_ctx);
    assert(RK_MPI_KARAOKE_StopPlayer_func);

    printf("%s\n", __func__);
    RK_MPI_KARAOKE_StopPlayer_func(player_ctx);
}

static void pbox_rockit_music_pause(void)
{
    assert(player_ctx);
    assert(RK_MPI_KARAOKE_PausePlayer_func);

    printf("%s\n", __func__);
    RK_MPI_KARAOKE_PausePlayer_func(player_ctx);

}

static void pbox_rockit_music_start_bt(int sampleFreq, int channel)
{
    KARAOKE_BT_ATTR_S attr;
    switch (sampleFreq) {
        case 0: {
            attr.u32Sample = 44100;
        } break;

        default: {
            attr.u32Sample = sampleFreq;
        } break;
    }

    switch (channel) {
        case 0: {
            attr.u32Channels = 2;
        } break;

        default: {
            attr.u32Channels = channel;
        } break;
    }

    attr.u32BitWidth = 16;

    assert(player_ctx);
    assert(RK_MPI_KARAOKE_StartBTPlayer_func);
    assert(RK_MPI_KARAOKE_StopBTPlayer_func);

    printf("%s sampe:%d, channel: %d\n", __func__, attr.u32Sample, attr.u32Channels);
    pbox_rockit_music_stop();
    RK_MPI_KARAOKE_StopBTPlayer_func(player_ctx);
    RK_MPI_KARAOKE_StartBTPlayer_func(player_ctx, &attr);
    set_vocal_separate_thread_cpu();
}

static void pbox_rockit_music_resume(uint32_t volume)
{
    assert(player_ctx);
    assert(RK_MPI_KARAOKE_ResumePlayer_func);
    assert(RK_MPI_KARAOKE_SetPlayerVolume_func);

    RK_MPI_KARAOKE_ResumePlayer_func(player_ctx);
    RK_MPI_KARAOKE_SetPlayerVolume_func(player_ctx, volume);

}

static int64_t pbox_rockit_music_get_duration(void) {
    RK_S64 duration = 0;
    assert(player_ctx);
    assert(RK_MPI_KARAOKE_GetPlayerDuration_func);

    RK_MPI_KARAOKE_GetPlayerDuration_func(player_ctx, &duration);
    printf("%s duration: %lld\n", __func__, duration);

    return duration;
}

static void pbox_rockit_music_reverb_mode(pbox_revertb_t mode) {
   KARAOKE_PARAM_S param;
   param.enType = KARAOKE_PARAM_REVERB;

   switch (mode) {
        case PBOX_REVERT_USER: {
            param.stReverbParam.enMode = KARAOKE_REVERB_MODE_USER;
        } break;
        case PBOX_REVERT_STUDIO: {
            param.stReverbParam.enMode = KARAOKE_REVERB_MODE_STUDIO;
        } break;
        case PBOX_REVERT_KTV: {
            param.stReverbParam.enMode = KARAOKE_REVERB_MODE_KTV;
        } break;
        case PBOX_REVERT_CONCERT: {
            param.stReverbParam.enMode = KARAOKE_REVERB_MODE_CONCERT;
        } break;
        default: break;
   }

   assert(player_ctx);
   assert(RK_MPI_KARAOKE_SetRecorderParam_func);

   if (mode == KARAOKE_REVERB_MODE_USER) 
       param.stReverbParam.bBypass = true;
   else 
       param.stReverbParam.bBypass = false;
   RK_MPI_KARAOKE_SetRecorderParam_func(player_ctx, &param);
}

static void pbox_rockit_music_echo_reduction(bool on) {
    KARAOKE_PARAM_S param;
    param.enType = KARAOKE_PARAM_3A;
    param.st3AParam.bBypass = !on;

    assert(player_ctx);
    assert(RK_MPI_KARAOKE_SetRecorderParam_func);

    int ret = RK_MPI_KARAOKE_SetRecorderParam_func(player_ctx, &param);
    printf("%s RK_MPI_KARAOKE_SetRecorderParam bypass:%d res:%d\n" ,__func__, param.st3AParam.bBypass, ret);
}

static void pbox_rockit_music_voice_seperate(pbox_vocal_t vocal) {
    KARAOKE_PARAM_S param;
    param.enType = KARAOKE_PARAM_VOLCAL_SEPARATE;

    assert(player_ctx);
    assert(RK_MPI_KARAOKE_GetPlayerParam_func);
    assert(RK_MPI_KARAOKE_SetPlayerParam_func);

    RK_BOOL enable = vocal.enable;
    int32_t hLevel = vocal.u32HumanLevel;
    int32_t mLevel = vocal.u32OtherLevel;
    int32_t rLevel = vocal.u32ReservLevel;

    if(hLevel > 100) hLevel = 15;
    else if(hLevel < 3) hLevel = 3;

    if(mLevel > 100 ) mLevel = 100;
    else if(mLevel < 0) mLevel = 0;

    if(rLevel > 100 ) rLevel = 100;
    else if(rLevel < 0) rLevel = 0;
    printf("%s hLevel:%d, mLevel:%d rLevel:%d , on:%d\n",__func__, hLevel, mLevel, rLevel, enable);

    int ret = RK_MPI_KARAOKE_GetPlayerParam_func(player_ctx, &param);

    if (enable)
        param.stVolcalSeparateParam.bBypass = false;
    else
        param.stVolcalSeparateParam.bBypass = true;
    param.stVolcalSeparateParam.u32HumanLevel = hLevel;
    param.stVolcalSeparateParam.u32OtherLevel = mLevel;
    param.stVolcalSeparateParam.u32ReservLevel = rLevel;
    ret = RK_MPI_KARAOKE_SetPlayerParam_func(player_ctx, &param);
    printf("%s RK_MPI_KARAOKE_SetPlayerParam_func res:%d\n" ,__func__, ret);
}

static uint32_t pbox_rockit_music_master_volume_get() {
    RK_U32 volume = 0; 
    assert(player_ctx);
    assert(RK_MPI_KARAOKE_GetPlayerVolume_func);
    RK_MPI_KARAOKE_GetPlayerVolume_func(player_ctx, &volume);

    return volume;
}

static uint32_t pbox_rockit_music_master_volume_adjust(int Level) {
    int volume;
    assert(player_ctx);
    assert(RK_MPI_KARAOKE_SetPlayerVolume_func);
    RK_MPI_KARAOKE_SetPlayerVolume_func(player_ctx, Level);

    return pbox_rockit_music_master_volume_get();
}

static void pbox_rockit_music_mic_volume_adjust(int micLevel) {
    assert(player_ctx);
    assert(RK_MPI_KARAOKE_SetRecorderVolume_func);
    RK_MPI_KARAOKE_SetRecorderVolume_func(player_ctx, micLevel);
}

static void pbox_rockit_music_seek_set(uint64_t usec) {
    assert(player_ctx);
    assert(RK_MPI_KARAOKE_SetPlayerSeekTo_func);
    RK_MPI_KARAOKE_SetPlayerSeekTo_func(player_ctx, usec);
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

static bool pbox_rockit_music_energyLevel_get(energy_info_t* pEnergy) {
    KARAOKE_ENERGY_LEVEL_S energy;
    int energyData[10];
    static int energyDataPrev[10];
    static int energykeep[10];
    static bool energy_debug = 0;
    assert(pEnergy);
    assert(player_ctx);
    assert(RK_MPI_KARAOKE_GetPlayerEnergyLevel_func);
    assert(RK_MPI_KARAOKE_ReleasePlayerEnergyLevel_func);

    int ret = RK_MPI_KARAOKE_GetPlayerEnergyLevel_func(player_ctx, &energy);
    if (!ret) {
        for (RK_S32 i = 0; i < sizeof(energyData)/sizeof(int); i++) {
            //translate energy range from [-90, 0] to [0, 100]
            energyData[i] = energy.ps16EnergyVec[10 + i] + 90;
            energyData[i] = energyData[i] + energyData[i]/10 + 1; //map to [1, 100]
            if(energy_debug) {
                printf("freq[%05d]HZ energy[%05d]DB energyData[%05d]\n",
                                energy.ps16EnergyVec[i], energy.ps16EnergyVec[10 + i], energyData[i]);
            }
        }

        mapDataToNewRange(energyData, sizeof(energyData)/sizeof(int), 0 , 100);

        for(RK_S32 i = 0; i < sizeof(energyData)/sizeof(int); i++) {
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
            pEnergy->energykeep[i].freq = energy.ps16EnergyVec[i];
            pEnergy->energykeep[i].energy = energyData[i];
        }

        RK_MPI_KARAOKE_ReleasePlayerEnergyLevel_func(player_ctx, &energy);

        return true;
    }
    return 0;
}

static void pbox_rockit_music_destroy(void) {
   if (RK_MPI_KARAOKE_StopPlayer_func != NULL) {
       RK_MPI_KARAOKE_StopPlayer_func(player_ctx);
   }
   if (RK_MPI_KARAOKE_StopRecorder_func != NULL) {
       RK_MPI_KARAOKE_StopRecorder_func(player_ctx);
   }
   if (RK_MPI_KARAOKE_Destroy_func != NULL) {  
       RK_MPI_KARAOKE_Destroy_func(player_ctx);
   }
   printf("destroy karaoke player\n");
}

#define MIN_ROCKIT_TIMER_INTER 50
static void *pbox_rockit_server(void *arg)
{
    int rockit_fds[1] = {0};
    int maxfd, i;
    char buff[sizeof(pbox_rockit_msg_t)] = {0};
    pbox_rockit_msg_t *msg;
    pthread_setname_np(pthread_self(), "party_rockit");

    rk_demo_music_create();

    int sock_fd = create_udp_socket(SOCKET_PATH_ROCKIT_SERVER);

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

        int ret = recvfrom(sock_fd, buff, sizeof(buff), 0, NULL, NULL);
        if (ret <= 0)
            continue;

        pbox_rockit_msg_t *msg = (pbox_rockit_msg_t *)buff;
        if(msg->msgId != 18)
            printf("%s recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

        if(msg->type == PBOX_EVT)
            continue;

        switch (msg->msgId) {
            case PBOX_ROCKIT_DESTROY: {
                pbox_rockit_music_destroy();
            } break;

            case PBOX_ROCKIT_SETDATASOURCE: {
                char *track_path = (char *)msg->dataSource.track_uri;
                if(strlen(track_path) == 0)
                    break;
                pbox_rockit_music_setDataSource(track_path, NULL);
            } break;

            case PBOX_ROCKIT_STARTBTPLAYER: {
                pbox_audioFormat_t audioFormat = msg->audioFormat;
                pbox_rockit_music_start_bt(audioFormat.sampingFreq, audioFormat.channel);
            } break;

            case PBOX_ROCKIT_STOPBTPLAYER: {
                pbox_rockit_music_stop();
            } break;

            case PBOX_ROCKIT_STARTPLAYER: {
                pbox_rockit_music_start();
            } break;

            case PBOX_ROCKIT_PAUSEPLAYER: {
                pbox_rockit_music_pause();
            } break;

            case PBOX_ROCKIT_RESUMEPLAYER: {
                int volume = msg->volume;
                pbox_rockit_music_resume(volume);
            } break;

            case PBOX_ROCKIT_STOPPLAYER: {
                pbox_rockit_music_stop();
            } break;

            case PBOX_ROCKIT_GETPLAYERCURRENTPOSITION: {
                //pending
            } break;

            case PBOX_ROCKIT_GETPLAYERDURATION: {
                RK_S64 duration = pbox_rockit_music_get_duration();
            } break;

            case PBOX_ROCKIT_SETPLAYERLOOPING: {
                //pending
            } break;

            case PBOX_ROCKIT_SETPLAYERSEEKTO: {
                uint64_t seek = msg->mPosition*1000;
                pbox_rockit_music_seek_set(seek);
            } break;

            case PBOX_ROCKIT_SETPLAYERVOLUME: {
                uint32_t volume = msg->volume;
                uint32_t vol_ret = pbox_rockit_music_master_volume_adjust(volume);
                if (volume != vol_ret) {
                    rockit_pbbox_notify_volume(volume);
                }

            } break;

            case PBOX_ROCKIT_GETPLAYERVOLUME: {
                uint32_t volume = pbox_rockit_music_master_volume_get();
                rockit_pbbox_notify_volume(volume);
            } break;

            case PBOX_ROCKIT_SETPLAYER_SEPERATE: {
                pbox_vocal_t vocal = msg->vocalSeperate;
                pbox_rockit_music_voice_seperate(vocal);
            } break;

            case PBOX_ROCKIT_GETPLAYER_SEPERATE: {
                //pending
            } break;

            case PBOX_ROCKIT_GETPLAYERENERGYLEVEL: {
                energy_info_t energy;
                if(pbox_rockit_music_energyLevel_get(&energy)) {
                    rockit_pbbox_notify_energy(energy);
                }
            } break;

            case PBOX_ROCKIT_SETRECORDERVOLUME: {
                pbox_rockit_music_mic_volume_adjust(msg->volume);
            } break;

            case PBOX_ROCKIT_SET_RECORDER_3A: {
                pbox_rockit_music_echo_reduction(msg->echo3A_On);
            } break;

            case PBOX_ROCKIT_SET_RECORDER_REVERT: {
                pbox_rockit_music_reverb_mode(msg->reverbMode);
            } break;

            default: {
            } break;
        }
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

void karaoke_callback(RK_VOID *pPrivateData, KARAOKE_EVT_E event, RK_S32 ext1, RK_VOID *ptr) {
    int ret = 0;

    printf("event: %d, ext1: %d\n", event, ext1);
    switch (event) {
        case KARAOKE_EVT_PLAYBACK_COMPLETE:
        case KARAOKE_EVT_PLAYBACK_ERROR: {
            rockit_pbbox_notify_playback_status(event);
        } break;
        case KARAOKE_EVT_AWAKEN: {
            rockit_pbbox_notify_awaken(ext1);
        }
        break;
        default:
        printf("Unknown event: %d", event);
    }
}