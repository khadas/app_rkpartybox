
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <semaphore.h>
#include <assert.h>
#include "os_minor_type.h"
#include <sys/timerfd.h>
#include <sys/time.h>

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