#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "slog.h"
#include "os_task.h"

int os_task_create(os_task_t *task, const char* name, task_routine_t routine_func, void* stack) {
    int ret;
    assert(task);

    pthread_mutex_init(&task->lock, NULL);
    task->params = stack;

    __atomic_store_n(&task->runing, true, __ATOMIC_RELAXED);
    ret = pthread_create(&task->task_tid, NULL, routine_func, task);
    if (ret) {
        ALOGE("Create thread failed %d\n", ret);
        return -1;
    }

    pthread_mutex_lock(&task->lock);
    snprintf(task->name, sizeof(task->name), "%s", name?name:"pbox_child");
    pthread_mutex_unlock(&task->lock);
    pthread_setname_np(task->task_tid, name);

    ALOGW("create task %s success..\n", name);
    return 0;
}

void os_task_destroy(os_task_t *task) {
    if(!task) return;

    if(__atomic_load_n(&task->runing, __ATOMIC_RELAXED)) {
        __atomic_store_n(&task->runing, false, __ATOMIC_RELAXED);
        pthread_join(task->task_tid, NULL);  // Wait for the thread to finish
        ALOGW("task %s joined..\n", task->name);
    }

    pthread_mutex_lock(&task->lock);
    memset(task->name, sizeof(task->name), 0);
    pthread_mutex_unlock(&task->lock);

    pthread_mutex_destroy(&task->lock);  // Destroy the mutex
}