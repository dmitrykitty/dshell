#include "utils.h"

#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>


char* trim(char* line){
    if(line == NULL) return NULL; 
    
    //skip leading spaces
    while(isspace((unsigned char)*line)){ 
        line++; 
    }

    //if only spaces
    if(*line == '\0') {
        return line;
    }

    char* end = line + strlen(line) - 1;

    //skip trailing spaces
    while(end > line && isspace((unsigned char)*end)){
        *end = '\0';
        end--;
    }

    return line; 
}

int parse_positive_long(const char *text, long *result) {
    if (text == NULL || text[0] == '\0' || result == NULL) {
        return 0;
    }

    char *end = NULL;
    errno = 0;

    long value = strtol(text, &end, 10);

    if (errno != 0 || end == text || *end != '\0' || value <= 0) {
        return 0;
    }

    *result = value;
    return 1;
}

int write_all(int fd, const char *buffer, size_t length) {
    while (length > 0) {
        //partial write is possible, so while loop is needed
        //ssize_t saves amount of bytes or -1 as error
        ssize_t written = write(fd, buffer, length);

        if (written == -1) {
            //interrupted by a caught signal
            if (errno == EINTR) {
                continue;
            }

            return -1;
        }

        if (written == 0) {
            //non typical IO error
            errno = EIO;
            return -1;
        }

        buffer += written;
        length -= (size_t) written;
    }

    return 0;
}

void build_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm tm_now;

    if (now == (time_t) -1 || localtime_r(&now, &tm_now) == NULL) {
        snprintf(buffer, size, "unknown-time");
        return;
    }

    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_now);
}
