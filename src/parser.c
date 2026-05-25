#include "command.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h> 


int parse_command(char *line, Command *cmd) {
    int argc = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_output = 0;
    cmd->background = 0;

    //divide by " " into separate strings
    char* token = strtok(line, " \t");
    while(token != NULL && argc < MAX_ARGS - 1){
        if (strcmp(token, "&") == 0) {
            cmd->background = 1;

            token = strtok(NULL, " \t");

            if (token != NULL) {
                fprintf(stderr, "parse error: '&' must be the last token\n");
                return 0;
            }

            break;
        }
        cmd->argv[argc++] = token; 
        token = strtok(NULL, " \t");
    }

    cmd->argv[argc] = NULL; 
    cmd->argc = argc; 
    return argc > 0; 
}