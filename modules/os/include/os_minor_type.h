#ifndef _UTILS_OS_FILE_COPY_H_
#define _UTILS_OS_FILE_COPY_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef void* os_memptr_t;

bool os_free(os_memptr_t memptr);
os_memptr_t os_malloc(uint16_t u16size);

#ifdef __cplusplus
}
#endif
#endif