#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "slog.h"

static int pbox_app_log_level = LOG_LEVEL_INFO;

void LogMsg(const char *format, ...)
{
    char        temp1[400];
    char        *temp = temp1;
    va_list     Next;
    int         len;

    // Go ahead and trace...
    va_start(Next, format);
    len = vsnprintf(temp, 380, format, Next);
    va_end(Next);


    if ((len < 0) || (len > 380))
    {
        len = strlen(temp);
    }

    if (len > 380)
    {
        temp[380] = 0;
        len = 380;
    }

    if (temp[len - 1] >= ' ')
    {
        temp[len] = '\n';
        temp[len + 1] = '\0';
    }

    printf("%s", temp);
    return;
}

uint32_t get_pbox_log_level(void) {
    return pbox_app_log_level;
}

static struct _debugInfo {
    uint32_t level;
    char *str;
} debugInfo[] = {
    {LOG_LEVEL_ERROR, "ERROR"},
    {LOG_LEVEL_WARN, "WARN"},
    {LOG_LEVEL_INFO, "INFO"},
    {LOG_LEVEL_DEBUG, "DEBUG"},
};

char *get_debug_level_str(uint32_t level) {
    for(int i = 0; i < LOG_LEVEL_NUM; i++) {
        if(debugInfo[i].level == level)
            return debugInfo[i].str;
    }

    return "unknown";
}

void set_pbox_log_level(uint32_t level) {
    ALOGW("%s level[%d]:%s\n", __func__, level, get_debug_level_str(level));
    pbox_app_log_level = level;
}

uint32_t covert2debugLevel(char *str) {
    uint32_t debug_level;

    if(strncasecmp(str, "error", strlen("error"))==0)
        debug_level = LOG_LEVEL_ERROR;
    else if(strncasecmp(str, "warn", strlen("warn"))==0)
        debug_level = LOG_LEVEL_WARN;
    else if(strncasecmp(str, "info", strlen("info"))==0)
        debug_level = LOG_LEVEL_INFO;
    else if(strncasecmp(str, "debug", strlen("debug"))==0)
        debug_level = LOG_LEVEL_DEBUG;
    else
        debug_level = LOG_LEVEL_WARN;

    //ALOGW("%s buffer:%s level:%d\n", __func__, str, debug_level);
    return debug_level;
}