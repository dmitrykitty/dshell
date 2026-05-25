#include <string.h>
#include <ctype.h>

#include "command.h"

static char* trim(char* line){
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

int parse_command(char *line, Command *cmd) {
    char* trimmed = trim(line); 
    int argc = 0;

    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_output = 0;
    cmd->background = 0;

    //divide by " " into separate strings
    char* token = strtok(line, " \t");
    while(token != NULL && argc < MAX_ARGS - 1){
        cmd->argv[argc++] = token; 
        token = strtok(NULL, " \t");
    }

    cmd->argv[argc] = NULL; 
    return argc > 0; 
}