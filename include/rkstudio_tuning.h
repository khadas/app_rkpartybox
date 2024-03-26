#ifndef __RKSTUDIO_TUNING_INCLUDE_H__
#define __RKSTUDIO_TUNING_INCLUDE_H__

typedef enum {
    CORE_IPC_MSG_ID_AUDIO_PARAM_INIT = 0,  // rkstudio init
    CORE_IPC_MSG_ID_AUDIO_PARAM_SET,       // rkstudio set
    CORE_IPC_MSG_ID_AUDIO_PARAM_GET,       // rkstudio get
} core_ipc_msg_id;



typedef int (*core_ipc_callback)(uint32_t dst_id, core_ipc_msg_id msg_id, void *data, uint32_t len);

int init_tuning(core_ipc_callback cb);

int deinit_tuning();

#endif