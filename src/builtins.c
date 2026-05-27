#define MAX_DIR_LENGTH 256

#include "builtins.h"
#include "shell_state.h"
#include "history.h"
#include "jobs.h"
#include "utils.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>


//list of env variables 
extern char** environ;

static BuiltinResult cmd_help(){
    printf("Built-in commands:\n");
    printf("  cd [path]\n");
    printf("  pwd\n");
    printf("  exit\n");
    printf("  help\n");
    printf("  env\n");
    printf("  setenv NAME VALUE\n");
    printf("  unsetenv NAME\n");
    printf("  history\n");
    printf("  jobs\n");
    printf("  kill\n");

    return BUILTIN_DONE;
}

static BuiltinResult cmd_pwd(){
    char cwd[MAX_DIR_LENGTH]; 

    if(getcwd(cwd, MAX_DIR_LENGTH) == NULL){
        perror("getcwd");
        return BUILTIN_DONE;
    }

    printf("%s\n", cwd); 

    return BUILTIN_DONE; 
}

static BuiltinResult cmd_cd(Command* cmd){
    if(cmd->argc > 2){
        fprintf(stderr, "usage: cd <PATH>\n");
        return BUILTIN_DONE;
    }
    const char *path = cmd->argv[1];

    if (path == NULL) {
        path = getenv("HOME");
    }

    if(path == NULL){
        fprintf(stderr, "cd: HOME is not set\n");
        return BUILTIN_DONE;
    }

    if(chdir(path) != 0){
        perror("cd");
    }; 
    
    return BUILTIN_DONE; 
}

static BuiltinResult cmd_env(){
    for(char** env = environ; *env != NULL; env++){
        printf("%s\n", *env);
    }

    return BUILTIN_DONE; 
}

static BuiltinResult cmd_setenv(Command* cmd){
    if (cmd->argc != 3) {
        fprintf(stderr, "usage: setenv <NAME> <VALUE>\n");
        return BUILTIN_DONE;
    }

    if(setenv(cmd->argv[1], cmd->argv[2], 1) != 0){
        perror("setenv");
    }

    return BUILTIN_DONE; 
}

static BuiltinResult cmd_unsetenv(Command* cmd){
    if (cmd->argc != 2){
        fprintf(stderr, "usage: unsetenv <NAME>\n");
        return BUILTIN_DONE;
    } 

    if(unsetenv(cmd->argv[1]) != 0){
        perror("unsetenv");
    }

    return BUILTIN_DONE; 
}

static BuiltinResult cmd_printenv(Command* cmd){
    if (cmd->argc != 2){
        fprintf(stderr, "usage: printenv <NAME>\n");
        return BUILTIN_DONE;
    } 

    char* env = getenv(cmd->argv[1]);
    if(env == NULL){
        printf("printenv: no such variable '%s' found\n", cmd->argv[1]);
        return BUILTIN_DONE;
    }

    printf("%s\n", env);
    return BUILTIN_DONE;
}

static BuiltinResult cmd_history(ShellState* state){
    history_print(&state->history);
    return BUILTIN_DONE; 
}

static BuiltinResult cmd_jobs(ShellState* state){
    job_table_refresh(&state->jobs);
    job_table_print(&state->jobs);
    return BUILTIN_DONE; 
}

static BuiltinResult cmd_kill(Command *cmd, ShellState *state){
    if(cmd->argc != 2){
        fprintf(stderr, "usage: kill <pid> | %%<job_id>\n");
        return BUILTIN_DONE;
    }

    if (state == NULL) {
        fprintf(stderr, "kill: shell state is not available\n");
        return BUILTIN_DONE;
    }
    pid_t pid; 

    if(cmd->argv[1][0] == '%'){
        long job_id;
        if(parse_positive_long(cmd->argv[1] + 1, &job_id) == 0){
            fprintf(stderr, "kill: invalid job id: %s\n", cmd->argv[1]);
            return BUILTIN_DONE;
        }
        
        Job* job = job_table_find_by_id(&state->jobs, job_id);

        if(job == NULL){
            fprintf(stderr, "kill: no such job: %s\n", cmd->argv[1]);
            return BUILTIN_DONE;
        }

        if (job->status != JOB_RUNNING) {
            fprintf(stderr, "kill: job is not running: %s\n", cmd->argv[1]);
            return BUILTIN_DONE;
        }

        pid = job->pid; 
    } else {
        long job_pid;
        if(parse_positive_long(cmd->argv[1], &job_pid) == 0){
            fprintf(stderr, "kill: invalid job pid: %s\n", cmd->argv[1]);
            return BUILTIN_DONE;
        }

        pid = (pid_t) job_pid;
    }

    if (kill(pid, SIGTERM) != 0) {
        perror("kill");
        return BUILTIN_DONE;
    }

    return BUILTIN_DONE;
}


/*
 * Execute built-ins in the shell process.
 * This is required for commands like cd/setenv because child changes would not
 * affect the parent shell.
 */
BuiltinResult execute_builtin(Command* cmd, ShellState* state){
    if (cmd == NULL || cmd->argv[0] == NULL) {
        return BUILTIN_NOT_FOUND;
    }

    if (strcmp(cmd->argv[0], "exit") == 0) {
        return BUILTIN_EXIT;
    }

    if (strcmp(cmd->argv[0], "pwd") == 0) {
        return cmd_pwd();
    }

    if (strcmp(cmd->argv[0], "cd") == 0) {
        return cmd_cd(cmd);
    }

    if (strcmp(cmd->argv[0], "help") == 0) {
        return cmd_help();
    }

    if (strcmp(cmd->argv[0], "env") == 0) {
        return cmd_env();
    }

    if (strcmp(cmd->argv[0], "setenv") == 0) {
        return cmd_setenv(cmd);
    }

    if (strcmp(cmd->argv[0], "unsetenv") == 0) {
        return cmd_unsetenv(cmd);
    }

    if (strcmp(cmd->argv[0], "printenv") == 0) {
        return cmd_printenv(cmd);
    }

    if(strcmp(cmd->argv[0], "history") == 0) {
        return cmd_history(state);
    }

    if(strcmp(cmd->argv[0], "jobs") == 0) {
        return cmd_jobs(state);
    }

    if(strcmp(cmd->argv[0], "kill") == 0) {
        return cmd_kill(cmd, state);
    }

    return BUILTIN_NOT_FOUND;
}
