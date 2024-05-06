#ifndef _UTILS_OS_FILE_COPY_H_
#define _UTILS_OS_FILE_COPY_H_

#include <stdbool.h>
#include <stdint.h>
#include <semaphore.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef void* os_memptr_t;

#ifdef __unix__
typedef sem_t os_sem_t;
#endif
bool os_free_osi(os_memptr_t memptr);
os_memptr_t os_malloc(uint16_t u16size);

os_sem_t *os_sem_new(unsigned int value);
int os_sem_post(os_sem_t *sem);
void os_sem_free(os_sem_t *sem);
int os_sem_wait(os_sem_t *sem);
int os_sem_trywait(os_sem_t *sem);

uint32_t os_get_boot_time_ms(void);
uint64_t os_get_boot_time_us(void);
uint64_t os_unix_time_ms(void);
uint64_t os_unix_time_us(void);
#define os_free(memptr)  do {   \
    os_free_osi(memptr);        \
    memptr = NULL;              \
} while(0)

#ifdef __cplusplus
}
#endif
#endif