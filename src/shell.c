#include "shell.h"
#include "parser.h"
#include "command.h"
#include "builtins.h"
#include "history.h"
#include "shell_state.h"
#include "utils.h"
#include "jobs.h"
#include "executor.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void start_shell_loop(){
    char *line = NULL;
    size_t size = 0;

    ShellState state;
    state.last_status = 0;
    history_init(&state.history);
    job_table_init(&state.jobs);

    while(1){
        job_table_refresh(&state.jobs);

        printf("> "); 
        //to show immediately
        fflush(stdout);

        if(getline(&line, &size, stdin) == -1){
            break;
        }

        char* trimmed = trim(line);
        char command_text[MAX_LINE_LENGTH];
        snprintf(command_text, MAX_LINE_LENGTH, "%s", trimmed); 

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

        //print_args(cmd.argv); 
        BuiltinResult builtin_res = execute_builtin(&cmd, &state); 

        if(builtin_res == BUILTIN_EXIT){
            break; 
        }

        if(builtin_res == BUILTIN_DONE){
            continue;
        }

        execute_external(&cmd, &state, command_text);
    }

    free(line);
}