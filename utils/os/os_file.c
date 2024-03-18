#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#ifndef BUFFER_SIZE
#ifdef __unix__
#define BUFFER_SIZE 1024
#else
//suppose this is rtos.
#define BUFFER_SIZE 64
#endif
#endif

int os_copy_file(const char *src_path, const char *dest_path) {
    assert(src_path != NULL);
    assert(dest_path != NULL);
    FILE *src_file = fopen(src_path, "rb");
    if (src_file == NULL) {
        perror("Failed to open source file");
        return -1;
    }

    FILE *dest_file = fopen(dest_path, "wb+");
    if (dest_file == NULL) {
        perror("Failed to create destination file");
        fclose(src_file);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    size_t bytes_written;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        bytes_written = fwrite(buffer, 1, bytes_read, dest_file);
        if (bytes_written != bytes_read) {
            perror("Failed to write to destination file");
            fclose(src_file);
            fclose(dest_file);
            return -1;
        }
    }

    if (ferror(src_file) != 0) {
        perror("Error reading source file");
        fclose(src_file);
        fclose(dest_file);
        return -1;
    }

    if (fclose(src_file) != 0 || fclose(dest_file) != 0) {
        perror("Error closing files");
        return -1;
    }

    // Ensure destination file has read permission
#ifdef __unix__
    struct stat st;
    if (stat(dest_path, &st) == 0) {
        if (!(st.st_mode & S_IRUSR)) {
            if (chmod(dest_path, st.st_mode | S_IRUSR) != 0) {
                perror("Failed to add read permission to destination file");
                return -1;
            }
        }
    }
#endif

    return 0;
}

