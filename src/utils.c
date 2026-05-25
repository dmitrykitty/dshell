#include <string.h>
#include <ctype.h>

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