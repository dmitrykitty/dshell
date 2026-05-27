#include "shell.h"
#include "parser.h"
#include "command.h"
#include "builtins.h"
#include "history.h"
#include "shell_state.h"
#include "utils.h"
#include "jobs.h"
#include "executor.h"
#include "signals.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/*
 * Main REPL for dshell.
 * Owns process-wide shell state: history, jobs, signal handlers and logger.
 */
void start_shell_loop(){
    char *line = NULL;
    size_t size = 0;

    ShellState state;
    state.last_status = 0;

    history_init(&state.history);
    job_table_init(&state.jobs);
    signals_init();

    if (logger_init(&state.logger, ".dshell.log") != 0) {
        fprintf(stderr, "warning: logger disabled\n");
    }
    logger_log(&state.logger, "shell started");

    char history_path[MAX_LINE_LENGTH];
    if (history_default_path(history_path, MAX_LINE_LENGTH) == 0) {
        history_load(&state.history, history_path);
        logger_logf(&state.logger, "history loaded: %s", history_path);
    } else {
        perror("history path");
        history_path[0] = '\0';
    }

    while(1){
        if (signals_has_sigchld()) {
            signals_clear_sigchld();
            job_table_refresh(&state.jobs);
        }

        printf("> "); 
        //to show immediately
        fflush(stdout);

        if(getline(&line, &size, stdin) == -1){
            //interrupted syscall occurred
            if(errno == EINTR){
                clearerr(stdin);
                continue;
            }
            break;
        }

        char* trimmed = trim(line);
        char command_text[MAX_LINE_LENGTH];
        snprintf(command_text, MAX_LINE_LENGTH, "%s", trimmed); 

        if (trimmed[0] == '\0') {
            continue;
        }

        logger_logf(&state.logger, "command: %s", command_text);
        history_add(&state.history, trimmed);
        history_save_line(history_path, trimmed);
        
        Pipeline pipeline;

        if(parse_pipeline(trimmed, &pipeline) != 1){
            logger_logf(&state.logger, "parse failed: %s", command_text);
            continue;
        }

        Command* cmd = &pipeline.left;

        if(pipeline.has_pipe){
            execute_pipeline(&pipeline, &state);
            continue;
        }

        if(cmd->argv[0] == NULL){
            continue;
        }

        //print_args(cmd.argv); 
        BuiltinResult builtin_res = execute_builtin(cmd, &state); 

        if(builtin_res == BUILTIN_EXIT){
            logger_log(&state.logger, "exit requested");
            break; 
        }

        if(builtin_res == BUILTIN_DONE){
            continue;
        }

        execute_external(cmd, &state, command_text);
    }
    
    logger_log(&state.logger, "shell stopped");
    logger_shutdown(&state.logger);

    free(line);
}
