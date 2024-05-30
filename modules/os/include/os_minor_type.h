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

#define UNUSED_ATTR __attribute__((unused))

pid_t os_gettid(void);
bool os_free(os_memptr_t memptr);
os_memptr_t os_malloc(uint16_t u16size);
os_memptr_t os_calloc(uint16_t u16size);

/*careful!! the null operation only useful
**for none formal parameter *************/
#define os_free_marco(memptr) do {   \
    os_free(memptr);            \
    memptr = NULL;              \
} while(0)

os_sem_t *os_sem_new(unsigned int value);
void os_sem_free(os_sem_t *sem);
int os_sem_post(os_sem_t *sem);

#define os_sem_free_macro(sem) do {   \
    os_sem_free(sem)            \
    sem = NULL;                 \
} while(0)

int os_sem_wait(os_sem_t *sem);
int os_sem_trywait(os_sem_t *sem);

uint32_t os_get_boot_time_ms(void);
uint64_t os_get_boot_time_us(void);
uint64_t os_unix_time_ms(void);
uint64_t os_unix_time_us(void);

int os_gpio_init(uint32_t gpio);
int os_gpio_deinit(uint32_t gpio);
int os_set_gpio_pin_direction(uint32_t gpio, uint32_t direction);
int os_set_gpio_value(uint32_t gpio, uint32_t value);
int os_get_gpio_value(uint32_t gpio);

int os_copy_file(const char *src_path, const char *dest_path);

#ifdef __cplusplus
}
#endif
#endif