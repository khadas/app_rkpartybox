#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <paths.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#include "rk_utils.h"
#include "slog.h"

#define DEBUG 1
#define PRIORITY_BASE(x)                       (sched_get_priority_min(x))

static int system_fd_closexec(const char* command)
{
    int wait_val = 0;
    pid_t pid = -1;

    if (!command)
        return 1;

    if ((pid = vfork()) < 0)
        return -1;

    if (pid == 0) {
        int i = 0;
        int stdin_fd = fileno(stdin);
        int stdout_fd = fileno(stdout);
        int stderr_fd = fileno(stderr);
        long sc_open_max = sysconf(_SC_OPEN_MAX);
        if (sc_open_max < 0) {
            sc_open_max = 20000; /* enough? */
        }
        /* close all descriptors in child sysconf(_SC_OPEN_MAX) */
        for (i = 0; i < sc_open_max; i++) {
            if (i == stdin_fd || i == stdout_fd || i == stderr_fd)
                continue;
            close(i);
        }

        execl(_PATH_BSHELL, "sh", "-c", command, (char*)0);
        _exit(127);
    }

    while (waitpid(pid, &wait_val, 0) < 0) {
        if (errno != EINTR) {
            wait_val = -1;
            break;
        }
    }

    return wait_val;
}

int exec_command_system(const char *cmd)
{
    pid_t status;

    ALOGW("[EXEC_DEBUG_SYS]: %s\n", cmd);

    status = system_fd_closexec(cmd);

    if (-1 == status) {
        ALOGE("[system_exec_err] -1\n");
        return -1;
    } else {
        if (WIFEXITED(status)) {
            if (0 == WEXITSTATUS(status)) {
                return 0;
            } else {
                ALOGE("[system_exec_err %s] -2\n", cmd);
                return -2;
            }
        } else {
            ALOGE("[system_exec_err] -3\n");
            return -3;
        }
    }

    return 0;
}

void exec_command(char cmdline[], char recv_buff[], int len)
{
    ALOGW("[EXEC_DEBUG]: %s\n", cmdline);

    FILE *stream = NULL;
    char *tmp_buff = recv_buff;

    memset(recv_buff, 0, len);

    if ((stream = popen(cmdline, "r")) != NULL) {
        while (fgets(tmp_buff, len, stream)) {
            //pr_info("tmp_buf[%d]: %s\n", strlen(tmp_buff), tmp_buff);
            tmp_buff += strlen(tmp_buff);
            len -= strlen(tmp_buff);
            if (len <= 1)
                break;
        }

        ALOGE("[EXEC_DEBUG] execute_r: %s \n", recv_buff);
        pclose(stream);
    } else
        ALOGE("[popen] error: %s\n", cmdline);
}

int test_pthread(pthread_t tid) /*pthread_kill的返回值：成功（0） 线程不存在（ESRCH） 信号不合法（EINVAL）*/
{
    int pthread_kill_err;
    pthread_kill_err = pthread_kill(tid, 0);

    if(pthread_kill_err == ESRCH)
        ALOGW("ID 0x%x NOT EXIST OR EXIT\n", (unsigned int)tid);
    else if(pthread_kill_err == EINVAL)
        ALOGW("SIGNAL ILL\n");
    else
        ALOGW("ID 0x%x ALIVE\n", (unsigned int)tid);

    return pthread_kill_err;
}

// Function definition
int get_ps_pid_new(const char Name[]) {
    DIR *dir;
    struct dirent *ent;
    char buf[512];

    // Open the /proc directory
    dir = opendir("/proc");
    if (dir == NULL) {
        ALOGE("Unable to open /proc directory");
        return 0;
    }

    // Traverse all subdirectories in the /proc directory
    while ((ent = readdir(dir)) != NULL) {
        // Check if the directory name is a number to filter out non-process directories
        if (isdigit(ent->d_name[0])) {
            // Build the complete path to the process's comm file
            snprintf(buf, sizeof(buf), "/proc/%s/comm", ent->d_name);

            // Open the process's comm file
            FILE *file = fopen(buf, "r");
            if (file != NULL) {
                // Read the process name
                if (fgets(buf, sizeof(buf), file) == NULL) {}

                // Remove the newline character
                size_t len = strlen(buf);
                if (len > 0 && buf[len - 1] == '\n') {
                    buf[len - 1] = '\0';
                }

                // Check if the process name matches
                if (strncmp(buf, Name, strlen(Name)) == 0) {
                    int pid = atoi(ent->d_name);
                    closedir(dir);
                    fclose(file);
                    return pid;  // Process name matches, indicating the process is running
                }

                fclose(file);
            }
        }
    }

    // Close the /proc directory
    closedir(dir);

    // No matching process name found, indicating the process is not running
    ALOGE("%s Can't find %s process running\n", __func__, Name);
    return 0;
}

int get_thread_pid(const char Name[])
{
    int len;
    char name[64] = {0};
    char cmdresult[256] = {0};
    char cmd[256] = {0};
    FILE *pFile = NULL;
    int  pid = 0;
    int retry_cnt = 3;

retry:
    memset(name, 0, 32);
    memset(cmdresult, 0, 256);
    memset(cmd, 0, 64);

    len = strlen(Name);
    strncpy(name,Name,len);
    name[31] ='\0';

    sprintf(cmd, "ps -T | grep %s | grep -v grep | sort -k1,1nr | awk 'NR==1 {print $1}'", name);

    pFile = popen(cmd, "r");
    if (pFile != NULL)  {
        while (fgets(cmdresult, sizeof(cmdresult), pFile)) {
            pid = atoi(cmdresult);
            break;
        }
        pclose(pFile);
    }

    if ((pid == 0) && (retry_cnt--))
        goto retry;

    ALOGW("%s tid=%d\n", Name, pid);
    return pid;
}

int get_psgrep_pid(const char Name[])
{
    int len;
    char name[64] = {0};
    char cmdresult[256] = {0};
    char cmd[256] = {0};
    FILE *pFile = NULL;
    int  pid = 0;
    int retry_cnt = 3;

retry:
    memset(name, 0, 32);
    memset(cmdresult, 0, 256);
    memset(cmd, 0, 64);

    len = strlen(Name);
    strncpy(name,Name,len);
    name[31] ='\0';

    sprintf(cmd, "ps -ef | grep \"%s\" | grep -v grep | sort -k1,1nr | awk 'NR==1 {print $1}'", name);

    pFile = popen(cmd, "r");
    if (pFile != NULL)  {
        while (fgets(cmdresult, sizeof(cmdresult), pFile)) {
            pid = atoi(cmdresult);
            break;
        }
        pclose(pFile);
    }

    if ((pid == 0) && (retry_cnt--))
        goto retry;

    return pid;
}

int get_ps_pid(const char Name[])
{
    int len;
    char name[32] = {0};
    char cmdresult[256] = {0};
    char cmd[64] = {0};
    FILE *pFile = NULL;
    int  pid = 0;
    int retry_cnt = 3;

retry:
    memset(name, 0, 32);
    memset(cmdresult, 0, 256);
    memset(cmd, 0, 64);

    len = strlen(Name);
    strncpy(name,Name,len);
    name[31] ='\0';

    sprintf(cmd, "pidof %s", name);

    pFile = popen(cmd, "r");
    if (pFile != NULL)  {
        while (fgets(cmdresult, sizeof(cmdresult), pFile)) {
            pid = atoi(cmdresult);
            break;
        }
        pclose(pFile);
    }

    if ((pid == 0) && (retry_cnt--))
        goto retry;

    ALOGW("%s [%s]=%d\n", __func__, Name, pid);
    return pid;
}

int kill_task(char *name)
{
    char cmd[128] = {0};
    //int exec_cnt = 3, retry_cnt = 10;

    if (!get_ps_pid(name))
        return 0;

    memset(cmd, 0, 128);
    sprintf(cmd, "killall %s", name);

    exec_command_system(cmd);

    if (get_ps_pid(name))
        msleep(600);

    if (get_ps_pid(name)) {
        memset(cmd, 0, 128);
        sprintf(cmd, "killall -9 %s", name);
        exec_command_system(cmd);
        msleep(300);
    }

    if (get_ps_pid(name)) {
        ALOGE("%s: kill %s failure [%d]\n", __func__, name, get_ps_pid(name));
        return -1;
    } else {
        ALOGE("%s: kill %s successful\n", __func__, name);
        return 0;
    }
}

int run_task(char *name, char *cmd)
{
    int exec_cnt = 3, retry_cnt = 6;

    while(exec_cnt) {
        if(!exec_command_system(cmd))
            break;
        exec_cnt--;
    }

    if(exec_cnt <= 0) {
        ALOGE("%s: run %s failed\n", __func__, name);
        return -1;
    }
    msleep(100);

retry:
    if (!get_ps_pid(name) && (retry_cnt--)) {
        msleep(100);
        goto retry;
    }

    if (!get_ps_pid(name))
        return -1;
    else
        return 0;
}

static void *vocal_separate_cpu(void *arg)
{
    int pid;
    static int old_vocal_neet=0 , old_vocal_separate= 0;
    //usleep(200*1000);
    for (int i = 0; i < 3; i ++) {
        if (((pid = get_thread_pid("vocal_neet")) > 0)&&(old_vocal_neet != pid)) {
            char cmdline[512];
            ALOGW("%s %d roud:%d\n", __func__, pid, i);
            sprintf(cmdline, "taskset -p 08 %d", pid);
            exec_command_system(cmdline);
            old_vocal_neet = pid;
            break;
        }
        usleep(100*1000);
    }

    for (int i = 0; i < 3; i ++) {
        if (((pid = get_thread_pid("vocal_separate-")) > 0)&&(old_vocal_separate != pid)) {
            char cmdline[512];
            ALOGW("%s %d roud:%d\n", __func__, pid, i);
            sprintf(cmdline, "taskset -p 08 %d", pid);
            exec_command_system(cmdline);
            old_vocal_separate= pid;
            break;
        }
        usleep(100*1000);
    }
}

void rk_schedparam_show(pid_t pid)
 {
     int policy, ret;
     struct sched_param param;

    if(pid == 0) {pid = syscall(SYS_getpid);}
    ALOGI("%s pid=%d\n", __func__, pid);

    ret = sched_getparam(pid, &param);
    policy = sched_getscheduler(pid);
    ALOGI("%s pid:%d(%d) ret:(%d) priority(%d) policy:%s\n",
        __func__, pid, getpriority(PRIO_PROCESS, pid), ret, param.sched_priority,
        (policy==SCHED_OTHER)?"OTHER":(policy==SCHED_FIFO)?"FIFO":"RR");
}

 int rk_setRtPrority(pid_t pid, int policy, int priority)
 {
     struct sched_param param;
     struct timespec tp;

     if(pid == 0) pid = syscall(SYS_getpid);

     param.sched_priority = PRIORITY_BASE(policy) + priority;

     int a = sched_setscheduler(pid, policy, &param);
     perror("sched_setscheduler");

     int b = sched_rr_get_interval(pid,&tp);
     ALOGI("pid:%d sched_priority:%d-[%d-%d], time:%u(s).%u(us), policy:%d,%d\n",\
         pid, priority, sched_get_priority_min(policy), sched_get_priority_max(policy),
         tp.tv_sec,tp.tv_nsec/1000,policy,a|b);

     rk_schedparam_show(pid);
     return a|b;
 }

#define ENV_BUF_SIZE_LINUX 1023
 int32_t os_env_get_u32(const char *name, uint32_t *value, uint32_t default_value) {
    char *ptr = getenv(name);

    if (NULL == ptr) {
        *value = default_value;
    } else {
        char *endptr;
        int base = (ptr[0] == '0' && ptr[1] == 'x') ? (16) : (10);
        errno = 0;
        *value = strtoul(ptr, &endptr, base);
        if (errno || (ptr == endptr)) {
            errno = 0;
            *value = default_value;
        }
    }
    return 0;
}

int32_t os_env_get_float(const char *name, float *value, float default_value) {
    char *ptr = getenv(name);

    if (NULL == ptr) {
        *value = default_value;
    } else {
        char *endptr;
        errno = 0;
        *value = strtod(ptr, &endptr);
        if (errno || (ptr == endptr)) {
            errno = 0;
            *value = default_value;
        }
    }

    return 0;
}

int32_t os_env_get_str(const char *name, const char **value, const char *default_value) {
    *value = getenv(name);

    if (NULL == *value) {
        *value = default_value;
    }
    return 0;
}

int32_t os_env_set_u32(const char *name, uint32_t value) {
    char buf[ENV_BUF_SIZE_LINUX];
    snprintf(buf, sizeof(buf), "%u", value);
    return setenv(name, buf, 1);
}

int32_t os_env_set_str(const char *name, char *value) {
    return setenv(name, value, 1);
}

/**
 * Get a boolean value from an environment variable.
 * Interprets "true" (case insensitive) as true, and any other value as false.
 */
int32_t os_env_get_bool(const char *name, bool *value, bool default_value) {
    char *ptr = getenv(name);

    if (NULL == ptr) {
        *value = default_value;
    } else {
        if (strcasecmp(ptr, "true") == 0) {
            *value = true;
        } else {
            *value = false;
        }
    }
    return 0;
}

/**
 * Set an environment variable to "true" or "false" based on the boolean value.
 */
int32_t os_env_set_bool(const char *name, bool value) {
    return setenv(name, value ? "true" : "false", 1);
}