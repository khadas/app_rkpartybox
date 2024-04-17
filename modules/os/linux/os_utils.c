
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include "os_minor_type.h"

os_memptr_t os_malloc(uint16_t u16size) {
    return (os_memptr_t) malloc(u16size);
}

bool os_free(os_memptr_t memptr) {
    if (NULL == memptr) {
        return true;
    }

    free(memptr);
    memptr = NULL;
    return true;
}