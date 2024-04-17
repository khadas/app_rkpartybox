#ifndef _UTILS_OS_TASK_H_
#define _UTILS_OS_TASK_H_

#include <pthread.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *params; //keep it on the head..
    volatile bool runing;
    pid_t pid_tid;
    pthread_t task_tid;
    pthread_mutex_t lock;
    char name[16];
}os_task_t;

typedef void *(* task_routine_t)(void *);

int os_task_create(os_task_t *task, const char* name, task_routine_t routine_func, void* stack);
void os_task_destroy(os_task_t *task);

#ifdef __cplusplus
}
#endif
#endif /* _UTILS_OS_TASK_H_ */
