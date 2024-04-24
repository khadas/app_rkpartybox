#ifndef _UTILS_OS_TASK_H_
#define _UTILS_OS_TASK_H_

#include <pthread.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct os_task_t os_task_t;
typedef void *(* task_routine_t)(void *);
os_task_t* os_task_create(const char* name, task_routine_t routine_func, uint32_t stack_size, void* stack);
void os_task_destroy(os_task_t *task);
bool is_os_task_started(struct os_task_t* task);

#ifdef __cplusplus
}
#endif
#endif /* _UTILS_OS_TASK_H_ */
