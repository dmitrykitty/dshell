#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "shell.h"
#include "parser.h"
#include "command.h"
#include "builtins.h"
#include "history.h"
#include "shell_state.h"
#include "utils.h"

static void print_args(char** argv){
    int argc = 0;
    while(argv[argc] != NULL){
        printf("(arg%d: %s) ", argc, argv[argc]);
        argc++;
    }
    printf("\n");
}

void start_shell_loop(){
    char *line = NULL;
    size_t size = 0;

    ShellState state;
    state.last_status = 0;
    history_init(&state.history);

    while(1){
        printf("> "); 
        //to show immediately
        fflush(stdout);

        if(getline(&line, &size, stdin) == -1){
            break;
        }

        char* trimmed = trim(line);

        if (trimmed[0] == '\0') {
            continue;
        }

        history_add(&state.history, trimmed);

        Command cmd;

        if(parse_command(trimmed, &cmd) != 1){
            continue;
        }

        if(cmd.argv[0] == NULL){
            continue;
        }

        print_args(cmd.argv); 
        int builtin_res = execute_builtin(&cmd, &state); 

        if(builtin_res == BUILTIN_EXIT){
            break; 
        }

        if(builtin_res == BUILTIN_DONE){
            continue;
        }
    }

    free(line);
}