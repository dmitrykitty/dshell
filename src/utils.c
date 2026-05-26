#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include "utils.h"

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