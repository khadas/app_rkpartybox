#ifndef _UTILS_OS_TASK_H_
#define _UTILS_OS_TASK_H_

#include <pthread.h>
#include "os_minor_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct os_task_t os_task_t;
typedef void *(* task_routine_t)(void *);
os_task_t* os_task_create(const char* name, task_routine_t routine_func, uint32_t stack_size, void* stack);
void os_task_destroy(os_task_t *task);
bool is_os_task_started(struct os_task_t* task);
pid_t os_task_get_pid(struct os_task_t* task);
char *os_task_get_name(struct os_task_t* task);
bool is_pid_equal(void *data, void *context);
os_task_t* os_task_find_by_pid(pid_t pid);
os_sem_t* os_task_get_quit_sem(pid_t pid);
int os_task_deint_all(void);

#ifdef __cplusplus
}
#endif
#endif /* _UTILS_OS_TASK_H_ */
