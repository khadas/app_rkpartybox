#ifndef _RK_UTILS_H_
#define _RK_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

//int test_pthread(pthread_t tid);
void exec_command(char cmdline[], char recv_buff[], int len);
int exec_command_system(const char *cmd);
int run_task(char *name, char *cmd);
int kill_task(char *name);
int get_ps_pid(const char Name[]);
int test_pthread(pthread_t tid); /*pthread_kill的返回值：成功（0） 线程不存在（ESRCH） 信号不合法（EINVAL）*/
int get_psgrep_pid(const char Name[]);
void set_vocal_separate_thread_cpu(void);
int get_ps_pid_new(const char Name[]);
int rk_setRtPrority(pid_t pid, int policy, int priority);
void rk_schedparam_show(pid_t pid);

#define msleep(x) usleep(x * 1000)

#ifdef __cplusplus
}
#endif
#endif //RKBT_UTILITY_H