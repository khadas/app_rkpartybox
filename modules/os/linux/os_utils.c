
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <semaphore.h>
#include <assert.h>
#include "os_minor_type.h"

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