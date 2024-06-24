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
#include "osi_list.h"

#define THREAD_NAME_MAX 16

static void os_task_free(void *ptr);

struct os_task_t {
    void *context;
    task_routine_t user_func;
    pthread_t task_tid;
    pid_t tid;
    volatile bool started;
    os_sem_t *quit_sem;
    char name[THREAD_NAME_MAX];
};

struct start_arg {
    struct os_task_t *task;
    os_sem_t *start_sem;
};

pthread_mutex_t tasklock;
static list_t *task_list = NULL;
static void* thread_wrapper(void *arg) {
    struct start_arg *start = arg;
    struct os_task_t *task = start->task;

    assert(task);
    assert(task->user_func);
    prctl(PR_SET_NAME, (unsigned long)task->name);
    task->started = true;
    os_sem_post(start->start_sem);

    task->tid = syscall(SYS_gettid);
    task->user_func(task->context);

    return NULL;
}

struct os_task_t* os_task_create(const char* name, task_routine_t routine_func, uint32_t stack_size, void* context) {
    int ret;
    struct os_task_t *task = os_malloc(sizeof(os_task_t));
    struct start_arg start_arg;

    if(task_list == NULL) {
        pthread_mutex_init(&tasklock, NULL);
        task_list = list_new(os_task_free);
    }

    snprintf(task->name, THREAD_NAME_MAX, "%s", name? name:"pbox_child");
    task->context = context;
    task->user_func = routine_func;
    task->quit_sem = os_sem_new(0);
    start_arg.task = task;
    start_arg.start_sem = os_sem_new(0);

    pthread_mutex_lock(&tasklock);
    list_append(task_list, task);
    ALOGW("%s %s, total tasks:%d\n", __func__, name,list_length(task_list));
    pthread_mutex_unlock(&tasklock);

    ret = pthread_create(&task->task_tid, NULL, thread_wrapper, &start_arg);
    if (ret) {
        ALOGE("Create thread failed %d\n", ret);
        goto fail;
    }

    os_sem_wait(start_arg.start_sem);
    os_sem_free(start_arg.start_sem);

    ALOGW("create task %s pid:%d success..\n", name, task->tid);
    return task;

fail:
    //pthread_mutex_destroy(&task->lock);
    os_sem_free(start_arg.start_sem);
    pthread_mutex_lock(&tasklock);
    bool res = list_remove(task_list, task);
    if(!res) {
        ALOGW("%s list remove failed..\n", __func__);
        assert(res);
    }
    pthread_mutex_unlock(&tasklock);
    return NULL;
}

// static void os_task_destroy_inner(struct os_task_t *task) {
//     assert(task);

//     os_sem_post(task->quit_sem);
//     pthread_join(task->task_tid, NULL);  // Wait for the thread to finish
//     ALOGW("%s task %s joined..\n", __func__, task->name);

//     task->started = false;
//     task->context = NULL;
//     task->tid = -1;
//     os_sem_free(task->quit_sem);
//     task->quit_sem = NULL;

//     //pthread_mutex_destroy(&task->lock);  // Destroy the mutex
//     os_free(task);
// }

void os_task_free(void *ptr) {
    if(!ptr) return;
    struct os_task_t *task = ptr;
    task->started = false;
    task->context = NULL;
    task->tid = -1;
    os_sem_free(task->quit_sem);
    task->quit_sem = NULL;
    os_free(task);
}

void os_task_destroy(struct os_task_t *task) {
    if(!task) return;

    ALOGW("%sing: %s\n", __func__, task->name);
    os_sem_post(task->quit_sem);
    pthread_join(task->task_tid, NULL);  // Wait for the thread to finish
    ALOGW("%s task %s joined..\n", __func__, task->name);

    pthread_mutex_lock(&tasklock);
    bool res = list_remove(task_list, task);
    if(!res) {
        ALOGW("%s list remove failed..\n", __func__);
        assert(res);
    }
    ALOGW("%s end, task_list len:%d\n", __func__, list_length(task_list));
    pthread_mutex_unlock(&tasklock);
}

bool is_os_task_started(struct os_task_t* task) {
    return task->started;
}

pid_t os_task_get_pid(struct os_task_t* task) {
    return task->tid;
}

char *os_task_get_name(struct os_task_t* task) {
    return task->name;
}

bool is_pid_equal(void *data, void *context) {
    pid_t* tid = context;
    os_task_t *task = data;
    if(task->tid == *tid)
        return false;

    return true;
}

os_task_t* os_task_find_by_pid(pid_t tid) {
    pthread_mutex_lock(&tasklock);
    list_node_t *node = list_foreach(task_list, is_pid_equal, &tid);
    if (node == NULL) {
        ALOGW("%s: can't find the task for tid:%d", __func__, tid);
        return NULL;
    }
    os_task_t *task = list_node(node);
    pthread_mutex_unlock(&tasklock);
    return task;
}

os_sem_t* os_task_get_quit_sem(pid_t tid) {
    os_task_t *task = os_task_find_by_pid(tid);
    if(task == NULL)
        return NULL;
    return task->quit_sem;
}

int os_task_deint_all(void) {
    assert(task_list);
    pthread_mutex_lock(&tasklock);
    ALOGW("%s,task_list len %d\n", __func__, list_length(task_list));

    for (const list_node_t *node = list_begin(task_list); node != list_end(task_list); node = list_next(node))
    {
        struct os_task_t *task = list_node(node);
        if(!task) {
            continue;
        }
        ALOGW("%s: task %d:%s not handled!!!!\n", __func__, task->tid, task->name);
        pthread_mutex_unlock(&tasklock);
        os_task_destroy(task);
        pthread_mutex_lock(&tasklock);
    }
    list_free(task_list);
    pthread_mutex_unlock(&tasklock);

    pthread_mutex_destroy(&tasklock);
    return 0;
}