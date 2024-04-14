#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "pbox_common.h"
#include "pbox_hotplug.h"
#include "pbox_keyscan_app.h"
#include "pbox_lvgl.h"
#include "pbox_rockit.h"
#include "rk_btsink.h"
#include "pbox_btsink_app.h"
#include "pbox_light_effect.h"

#define PRINT_FLAG_ERR "[RK_SKT_ERROR]"
#define PRINT_FLAG_SUCESS "[RK_SKT_SUCESS]"

