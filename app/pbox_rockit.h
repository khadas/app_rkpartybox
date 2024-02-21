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
    PBOX_ROCKIT_SETRECORDERMUTE,
    PBOX_ROCKIT_SET_RECORDER_REVERT,
    PBOX_ROCKIT_SET_RECORDER_3A,
    PBOX_ROCKIT_SET_MIC_STATE,
    PBOX_ROCKIT_SET_STEREO_MODE,
    PBOX_ROCKIT_SET_OUTDOOR_MODE,
    PBOX_ROCKIT_SET_PLACEMENT_MODE,
    PBOX_ROCKIT_SET_UAC_STATE,
    PBOX_ROCKIT_SET_UAC_SAMPLE_RATE,
    PBOX_ROCKIT_SET_UAC_VOLUME,
    PBOX_ROCKIT_SET_UAC_MUTE,
    PBOX_ROCKIT_SET_UAC_PPM,

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
    pbox_msg_t type;
    pbox_rockit_opcode_t msgId;
    union {
        struct {
            char track_uri[MAX_MUSIC_NAME_LENGTH +1];
            char headers[MAX_APP_NAME_LENGTH +1];
        } dataSource;
        bool loop;
        uint32_t mPosition;
        uint32_t volume;
        pbox_revertb_t  reverbMode;
        pbox_vocal_t    vocalSeperate;
        bool            echo3A_On;
        mic_data_t      micState;
        bool            micmute;
        pbox_audioFormat_t  audioFormat;
        stereo_mode_t       stereo;
        inout_door_t    outdoor;
        placement_t     place;

        struct _wake_up {
            KARAOKE_WAKE_UP_CMD_E wakeCmd;
            union {
                uint32_t volume;
            };
        } wake_up;
        uint32_t duration;
        energy_info_t energy_data;
        uac_t uac;
    };
} pbox_rockit_msg_t;


int pbox_create_rockitTask(void);

#ifdef __cplusplus
}
#endif
#endif//_PBOX_ROCKIT_H_