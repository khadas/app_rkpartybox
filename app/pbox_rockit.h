#ifndef _PBOX_ROCKIT_H_
#define _PBOX_ROCKIT_H_
#include "pbox_common.h"
#include "rk_type.h"
#include "rk_comm_karaoke.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    //command
    PBOX_ROCKIT_CREATE = 1,
    PBOX_ROCKIT_DESTROY,
    PBOX_ROCKIT_SETDATASOURCE,
    PBOX_ROCKIT_STARTBTPLAYER,
    PBOX_ROCKIT_STOPBTPLAYER,
    PBOX_ROCKIT_STARTPLAYER,
    PBOX_ROCKIT_PAUSEPLAYER,
    PBOX_ROCKIT_RESUMEPLAYER,
    PBOX_ROCKIT_STOPPLAYER,
    PBOX_ROCKIT_GETPLAYERCURRENTPOSITION,//10
    PBOX_ROCKIT_GETPLAYERDURATION,
    PBOX_ROCKIT_SETPLAYERLOOPING,
    PBOX_ROCKIT_SETPLAYERSEEKTO,
    PBOX_ROCKIT_SETPLAYERVOLUME,
    PBOX_ROCKIT_GETPLAYERVOLUME,
    PBOX_ROCKIT_SETPLAYER_SEPERATE,
    PBOX_ROCKIT_GETPLAYER_SEPERATE,
    PBOX_ROCKIT_GETPLAYERENERGYLEVEL,
    PBOX_ROCKIT_RELEASEPLAYERENERGYLEVEL,
    PBOX_ROCKIT_STARTRECORDER,//20
    PBOX_ROCKIT_STOPRECORDER,
    PBOX_ROCKIT_SETRECORDERVOLUME,
    PBOX_ROCKIT_GETRECORDERVOLUME,
    PBOX_ROCKIT_SET_RECORDER_REVERT,
    PBOX_ROCKIT_SET_RECORDER_3A,

    //event
    PBOX_ROCKIT_ENERGY_EVT = 0x100,
    PBOX_ROCKIT_MUSIC_POSITION_EVT,
    PBOX_ROCKIT_MUSIC_DURATION_EVT,
    PBOX_ROCKIT_MUSIC_VOLUME_EVT,
    PBOX_ROCKIT_PLAY_COMPLETED_EVT,
    PBOX_ROCKIT_PLAY_ERROR_EVT,
    PBOX_ROCKIT_AWAKEN_EVT,
} pbox_rockit_opcode_t;

typedef struct {
    int freq;
    int energy;
}energy_t;

typedef struct {
    pbox_msg_t type;
    pbox_rockit_opcode_t msgId;
    union {
        //PBOX_ROCKIT_SETDATASOURCE
        struct {
            char track_uri[MAX_APP_NAME_LENGTH +1];
            char headers[MAX_APP_NAME_LENGTH +1];
        } dataSource;
        //PBOX_ROCKIT_SETPLAYERLOOPING
        RK_BOOL loop;
        //PBOX_ROCKIT_SETPLAYERSEEKTO
        //PBOX_ROCKIT_MUSIC_DURATION_EVT
        RK_S64 usecPosition;
        //PBOX_ROCKIT_SETPLAYERVOLUME
        //PBOX_ROCKIT_GETPLAYERVOLUME
        RK_U32 volume;
        pbox_revertb_t      reverbMode;
        pbox_vocal_t        vocal;
        RK_BOOL             echo3A_On;
        pbox_audioFormat_t  audioFormat;

        struct _wake_up {
            KARAOKE_WAKE_UP_CMD_E wakeCmd;
            union {
                RK_U32 volume;
            };
        } wake_up;
        RK_S64 duration;
        struct energy_info {
            int size;
            energy_t energykeep[10];
        } energy_data;
    };
} pbox_rockit_msg_t;


int pbox_create_rockitTask(void);

#ifdef __cplusplus
}
#endif
#endif//_PBOX_ROCKIT_H_