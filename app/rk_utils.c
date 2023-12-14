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

#include "rk_utils.h"

#define DEBUG 1

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

	printf("[EXEC_DEBUG_SYS]: %s\n", cmd);

	status = system_fd_closexec(cmd);

	if (-1 == status) {
		printf("[system_exec_err] -1\n");
		return -1;
	} else {
		if (WIFEXITED(status)) {
			if (0 == WEXITSTATUS(status)) {
				return 0;
			} else {
				printf("[system_exec_err %s] -2\n", cmd);
				return -2;
			}
		} else {
			printf("[system_exec_err] -3\n");
			return -3;
		}
	}

	return 0;
}

void exec_command(char cmdline[], char recv_buff[], int len)
{
	printf("[EXEC_DEBUG]: %s\n", cmdline);

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

		printf("[EXEC_DEBUG] execute_r: %s \n", recv_buff);
		pclose(stream);
	} else
		printf("[popen] error: %s\n", cmdline);
}

int test_pthread(pthread_t tid) /*pthread_kill的返回值：成功（0） 线程不存在（ESRCH） 信号不合法（EINVAL）*/
{
	int pthread_kill_err;
	pthread_kill_err = pthread_kill(tid, 0);

	if(pthread_kill_err == ESRCH)
		printf("ID 0x%x NOT EXIST OR EXIT\n", (unsigned int)tid);
	else if(pthread_kill_err == EINVAL)
		printf("SIGNAL ILL\n");
	else
		printf("ID 0x%x ALIVE\n", (unsigned int)tid);

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
        printf("Unable to open /proc directory");
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
                fgets(buf, sizeof(buf), file);

                // Remove the newline character
                size_t len = strlen(buf);
                if (len > 0 && buf[len - 1] == '\n') {
                    buf[len - 1] = '\0';
                }

                // Check if the process name matches
                if (strncmp(buf, Name, strlen(Name)) == 0) {
                    closedir(dir);
                    fclose(file);
                    return atoi(ent->d_name);  // Process name matches, indicating the process is running
                }

                fclose(file);
            }
        }
    }

    // Close the /proc directory
    closedir(dir);

    // No matching process name found, indicating the process is not running
    printf("%s Can't find %s process running\n", __func__, Name);
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

    printf("%s tid=%d\n", Name, pid);
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
		printf("%s: kill %s failure [%d]\n", __func__, name, get_ps_pid(name));
		return -1;
	} else {
		printf("%s: kill %s successful\n", __func__, name);
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
		printf("%s: run %s failed\n", __func__, name);
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

