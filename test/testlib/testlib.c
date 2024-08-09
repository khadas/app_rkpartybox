#include "stdio.h"

void hellolib(void) {
    printf("\033[1;31m %s \033[0m\n", __func__);
}