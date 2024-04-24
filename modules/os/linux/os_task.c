#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include "os_minor_type.h"
#include "os_utils.h"
#include "slog.h"
#include "os_task.h"

#define THREAD_NAME_MAX 16

struct os_task_t {
    void *context;
    task_routine_t user_func;
    pthread_t task_tid;
    pid_t pid;
    volatile bool started;
    //pthread_mutex_t lock;
    char name[THREAD_NAME_MAX];
};

struct start_arg {
    struct os_task_t *task;
    os_sem_t *start_sem;
};

static void* thread_wrapper(void *arg) {
    struct start_arg *start = arg;
    struct os_task_t *task = start->task;

    assert(task);
    assert(task->user_func);
    prctl(PR_SET_NAME, (unsigned long)task->name);
    task->started = true;
    os_sem_post(start->start_sem);

    task->pid = syscall(SYS_gettid);
    task->user_func(task->context);

    return NULL;
}

struct os_task_t* os_task_create(const char* name, task_routine_t routine_func, uint32_t stack_size, void* context) {
    int ret;
    struct os_task_t *task = os_malloc(sizeof(os_task_t));
    struct start_arg start_arg;

    start_arg.task = task;
    start_arg.start_sem = os_sem_new(0);

    //pthread_mutex_init(&task->lock, NULL);
    snprintf(task->name, THREAD_NAME_MAX, "%s", name? name:"pbox_child");
    task->context = context;
    task->user_func = routine_func;

    ret = pthread_create(&task->task_tid, NULL, thread_wrapper, &start_arg);
    if (ret) {
        ALOGE("Create thread failed %d\n", ret);
        goto fail;
    }

    os_sem_wait(start_arg.start_sem);
    os_sem_free(start_arg.start_sem);
    ALOGW("create task %s success..\n", name);
    return task;

fail:
    //pthread_mutex_destroy(&task->lock);
    os_sem_free(start_arg.start_sem);
    os_free(start_arg.task);
    return NULL;
}

void os_task_destroy(struct os_task_t *task) {
    if(!task) return;

    pthread_join(task->task_tid, NULL);  // Wait for the thread to finish
    ALOGW("task %s joined..\n", task->name);

    task->started = false;
    task->context = NULL;
    task->pid = -1;

    //pthread_mutex_destroy(&task->lock);  // Destroy the mutex
    os_free(task);
}

bool is_os_task_started(struct os_task_t* task) {
    return task->started;
}