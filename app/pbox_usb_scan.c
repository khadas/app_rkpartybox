#include "stdio.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <paths.h>
#include <dirent.h>

#include "pbox_usb.h"
#include "pbox_common.h"

static char *strlwr(char *s)
{
  char *p = s;
  for(;*s;s++) {
    *s = tolower(*s);
  }
  return p;
}

static char *strrstr(const char *str, const char *token)
{
    int len = strlen(token);
    const char *p = str + strlen(str);

    while (str <= --p)
        if (p[0] == token[0] && strncmp(p, token, len) == 0)
            return (char *)p;
    return NULL;
}

bool file_is_supported(char *filepath, music_format_t* found)
{
    bool result = false;

    if (strstr(filepath, ".") == NULL)
        return false;

    char *suffix = strlwr(strdup(strrstr(filepath, ".") + 1));
    struct _music_file_raw {
        music_format_t format;
        char *support_suffix;
    } static const formats[] = {
        { MUSIC_FILE_MP3, "mp3"},
        { MUSIC_FILE_WAV, "wav"},
        { MUSIC_FILE_FLAC, "flac"},
        { MUSIC_FILE_OGG, "ogg"},
    };

    for (int i = 0; i < sizeof(formats) / sizeof(*formats); i++) {
        if (strcasecmp(suffix, formats[i].support_suffix) == 0) {
            *found = formats[i].format;
            result = true;
            break;
        }
    }

    printf("%s is %ssupported, [%s]\n", filepath, result ? "" : "not ", suffix);
    free(suffix);
    return result;
}

static uint32_t g_server_track_num = 0;
int scan_dir(const char *path, int depth, void (*call_back)(music_format_t, char *)) {
    DIR *dir;
    struct dirent *entry;
    int file_count = 0;
    dir = opendir(path);
    if (!dir)
    {
        printf("Error opening directory /oem\n");
        return -1;
    }
    //printf("open dir %s ok!", path);
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG) {
            music_format_t format = MUSIC_FILE_NONE;
            if (file_is_supported(entry->d_name, &format)) {
                    //strcpy(title_list[file_count++], entry->d_name);
                    if(call_back) {
                        call_back(format, entry->d_name);
                    }
            if (file_count > TRACK_MAX_NUM)
                break;
            }
        }
    }
    g_server_track_num = file_count;
    closedir(dir);
    return 0;
}
