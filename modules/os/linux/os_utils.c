#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <semaphore.h>
#include <assert.h>
#include <sys/timerfd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "os_minor_type.h"
#include "slog.h"

os_memptr_t os_malloc(uint16_t u16size) {
    return (os_memptr_t) malloc(u16size);
}

bool os_free_osi(os_memptr_t memptr) {
    if (NULL == memptr) {
        return true;
    }

    free(memptr);
    return true;
}

os_sem_t *os_sem_new(unsigned int value) {
  sem_t *sem = os_malloc(sizeof(sem_t));
  if (sem)
    sem_init(sem, 0, value);
  return sem;
}

int os_sem_post(os_sem_t *sem) {
  return sem_post(sem);
}

void os_sem_free(os_sem_t *sem) {
  if (!sem)
    return;

  sem_destroy(sem);
  os_free(sem);
}

int os_sem_wait(os_sem_t *sem) {
  assert(sem);

  return sem_wait(sem);
}

int os_sem_trywait(os_sem_t *sem) {
  assert(sem);

  return sem_trywait(sem);
}

uint32_t os_get_boot_time_ms(void) { 
    return os_get_boot_time_us() / 1000;
}

uint64_t os_get_boot_time_us(void) {
  struct timespec ts_now = {};
  clock_gettime(CLOCK_BOOTTIME, &ts_now);

  return ((uint64_t)ts_now.tv_sec * 1000000L) +
         ((uint64_t)ts_now.tv_nsec / 1000);
}

uint64_t os_unix_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

uint64_t os_unix_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}

/*
void timestamp(char *fmt, int index, int start)
{
    static long int st[128];
    struct timeval t;

    if (start)
    {
        printf("[%d %s]", index, fmt);
        gettimeofday(&t, NULL);
        st[index] = t.tv_sec * 1000000 + t.tv_usec;
    }
    else
    {
        gettimeofday(&t, NULL);
        printf("[%d # cost %ld us]\n", index, (t.tv_sec * 1000000 + t.tv_usec) - st[index]);
    }
}*/

//#include "rk_utils.h"
int os_gpio_init(uint32_t gpio)
{
    int fd;
    char cmd[64] = {0};
    char value[10] = {0};
    snprintf(cmd, sizeof(cmd), "%s", "/sys/class/gpio/export");
    fd = open(cmd, O_WRONLY);
    if (fd < 0) {
      ALOGE("Failed to open gpio%u for export!\n", gpio); 
      return -1;
    }

    ALOGD("%s gpio%u\n", __func__, gpio);
    snprintf(value, sizeof(value), "%u", gpio);
    if (write(fd, value, strlen(value)) < 0) {
        ALOGE("%s Failed to write export!\n", __func__); 
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int os_gpio_deinit(uint32_t gpio)
{
    int fd;
    char cmd[64] = {0};
    char value[10] = {0};
    snprintf(cmd, sizeof(cmd), "%s", "/sys/class/gpio/unexport");
    fd = open(cmd, O_WRONLY);
      if (fd < 0) {
      ALOGE("Failed to open gpio%u for unexport!\n", gpio); 
      return -1;
    }

    snprintf(value, sizeof(value), "%u", gpio);
    if (write(fd, value, strlen(value)) < 0) {
        ALOGE("%s Failed to write unexport!\n", __func__); 
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int os_set_gpio_pin_direction(uint32_t gpio, uint32_t direction) {
  int fd;
  char cmd[64] = {0};
  char value[4] = {0};

  snprintf(cmd, sizeof(cmd), "/sys/class/gpio/gpio%u/direction", gpio);
  fd = open(cmd, O_WRONLY);
  if (fd < 0) {
    ALOGE("Failed to open gpio%u for writing!\n", gpio); 
    return -1;
  }

  snprintf(value, sizeof(value), "%s", direction == 1 ? "out" : "in");
  if (write(fd, direction == 1 ? "out" : "in", strlen(value)) < 0){
    ALOGE("%s Failed to write gpio%u to value %d %s\n", __func__, gpio, value, strerror(errno));
    close(fd);
    return -1;
  }

  close(fd);
  return 0;
}

int os_set_gpio_value(uint32_t gpio, uint32_t value)
{
  int fd;
  char cmd[64] = {0};
  //sprintf(cmd, "echo %d > /sys/class/gpio/gpio%u/value", value, gpio);
  snprintf(cmd, sizeof(cmd), "/sys/class/gpio/gpio%u/value", gpio);
  fd = open(cmd, O_WRONLY);
  if (fd < 0) {
    ALOGE("Failed to open gpio%u for writing!\n", gpio); 
    return -1;
  }

  if (write(fd, value == 0 ? "0" : "1", 1) < 0){
    ALOGE("%s Failed to write gpio%u to value %d %s\n",__func__, gpio, value, strerror(errno));
    close(fd);
    return -1;
  }

  close(fd);
  return 0;
}

int os_get_gpio_value(uint32_t gpio)
{
  int fd;
  char cmd[64] = {0};
    char value[4] = {0};

  snprintf(cmd, sizeof(cmd), "/sys/class/gpio/gpio%u/value", gpio);
  fd = open(cmd, O_RDONLY);
  if (fd < 0) {
    ALOGE("Failed to open gpio%u for reading!\n", gpio); 
    return -1;
  }

  if (read(fd, value, sizeof(value)-1) < 0){
    ALOGE("Failed to read gpio%u to value %d\n", gpio, value);
    close(fd);
        return -1;
  }

  close(fd);
  return (value[0] == '0') ? 0 : 1;
}