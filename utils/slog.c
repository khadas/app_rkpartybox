#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

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

