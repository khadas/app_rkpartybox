#ifndef __RKSTUDIO_TUNING_INCLUDE_H__
#define __RKSTUDIO_TUNING_INCLUDE_H__

typedef int (*core_ipc_callback)(uint32_t dst_id, uint32_t msg_id, void *data, uint32_t len);

int init_tuning(core_ipc_callback cb);

int deinit_tuning();

#endif