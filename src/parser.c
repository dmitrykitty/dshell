#include "command.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h> 

static int parse_redirection_target(char **next_token, const char *operator_name) {
    if (*next_token == NULL) {
        fprintf(stderr, "parse error: missing file after '%s'\n", operator_name);
        return 0;
    }

    return 1;
}

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
        /*
        for tokens like ["echo", "hello", ">", "out.txt", NULL] 
        argv = [echo, hello, NULL] and cmd.input/output = name
        */
        if (strcmp(token, "<") == 0){
            //theoretically name of the file to be redirected
            token = strtok(NULL, " \t");

            if(parse_redirection_target(&token, "<") == 0){
                return 0;
            }

            if(cmd->input_file != NULL){
                fprintf(stderr, "parse error: duplicate input redirection\n");
                return 0;
            }

            cmd->input_file = token;
            token = strtok(NULL, " \t");
            continue;
        }

        if (strcmp(token, ">") == 0 || strcmp(token, ">>") == 0) {
            int append = (strcmp(token, ">>") == 0); 

            //theoretically name of the file to be redirected
            token = strtok(NULL, " \t");

            if(parse_redirection_target(&token, (append ? ">>" : ">")) == 0){
                return 0;
            }

            if(cmd->input_file != NULL){
                fprintf(stderr, "parse error: duplicate output redirection\n");
                return 0;
            }

            cmd->append_output = append; 
            cmd->output_file = token; 
            token = strtok(NULL, "\t");
            continue;
        }

        cmd->argv[argc++] = token; 
        token = strtok(NULL, " \t");
    }

    cmd->argv[argc] = NULL; 
    cmd->argc = argc; 
    return argc > 0; 
}