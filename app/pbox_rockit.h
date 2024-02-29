#ifndef _PBOX_ROCKIT_H_
#define _PBOX_ROCKIT_H_
#include "pbox_common.h"
#include "rc_partybox.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    //command
    PBOX_ROCKIT_CREATE = 1,
    PBOX_ROCKIT_DESTROY,
//    PBOX_ROCKIT_SET_DATASOURCE,
    PBOX_ROCKIT_START_AUDIOCARD_PLAYER,
//    PBOX_ROCKIT_STOPBTPLAYER,
    PBOX_ROCKIT_START_LOCAL_PLAYER,
    PBOX_ROCKIT_PAUSE_PLAYER,
    PBOX_ROCKIT_RESUME_PLAYER,
    PBOX_ROCKIT_STOP_PLAYER,
    PBOX_ROCKIT_GET_PLAYERCURRENTPOSITION,//10
    PBOX_ROCKIT_GET_PLAYERDURATION,
    PBOX_ROCKIT_SET_PLAYERLOOPING,
    PBOX_ROCKIT_SET_PLAYERSEEKTO,
    PBOX_ROCKIT_SET_MUSICVOLUME,
    PBOX_ROCKIT_GET_MUSICVOLUME,
    PBOX_ROCKIT_SET_PLAYERVOLUME,
    PBOX_ROCKIT_GET_PLAYERVOLUME,
    PBOX_ROCKIT_SET_PLAYER_SEPERATE,
    PBOX_ROCKIT_GET_PLAYER_SEPERATE,
    PBOX_ROCKIT_GET_PLAYERENERGYLEVEL,
    PBOX_ROCKIT_RELEASE_PLAYERENERGYLEVEL,
    PBOX_ROCKIT_START_RECORDER,//20
    PBOX_ROCKIT_STOP_RECORDER,
    PBOX_ROCKIT_SET_RECORDERVOLUME,
    PBOX_ROCKIT_GET_RECORDERVOLUME,
    PBOX_ROCKIT_SET_RECORDERMUTE,
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
    PBOX_ROCKIT_MUSIC_MAIN_VOLUME_EVT,
    PBOX_ROCKIT_MUSIC_CHANNEL_VOLUME_EVT,
    PBOX_ROCKIT_PLAY_COMPLETED_EVT,
    PBOX_ROCKIT_PLAY_ERROR_EVT,
    PBOX_ROCKIT_AWAKEN_EVT,
} pbox_rockit_opcode_t;

typedef struct {
    pbox_msg_t      type;
    pbox_rockit_opcode_t msgId;
    input_source_t  source;
    union {
        struct {
            char track_uri[MAX_MUSIC_NAME_LENGTH +1];
            char headers[MAX_APP_NAME_LENGTH +1];
        } dataSource;
        bool            loop;
        uint32_t        mPosition;
        uint32_t        volume;
        pbox_revertb_t  reverbMode;
        pbox_vocal_t    vocalSeperate;
        bool            echo3A_On;
        mic_data_t      micState;
        bool            micmute;
        pbox_audioFormat_t  audioFormat;
        stereo_mode_t   stereo;
        inout_door_t    outdoor;
        placement_t     place;
        struct _wake_up {
            enum rc_pb_wake_up_cmd wakeCmd;
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