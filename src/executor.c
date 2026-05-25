#include "executor.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_external(Command *cmd, ShellState *state, const char *command_text){
    if (cmd == NULL || cmd->argv[0] == NULL || state == NULL) {
        return -1;
    }

    pid_t pid = fork(); 

    if(pid < 0){
        perror("fork");
        return -1;
    }

    //child process
    if(pid == 0){
        execvp(cmd->argv[0], cmd->argv);

        perror(cmd->argv[0]);
        //command not found / exec failed
        _exit(127);
    }

    //main process
    if(cmd->background){
        const Job* job = job_table_add(&state->jobs, pid, command_text);
        job_print(job);
        return 0;
    }

    int status; 
    if(waitpid(pid, &status, 0) == -1){
        perror("waitpid");
        return -1;
    }

    if(WIFEXITED(status)){
        state->last_status = WEXITSTATUS(status);
    }else if (WIFSIGNALED(status)) {
        state->last_status = 128 + WTERMSIG(status);
    }

    return state->last_status;
}
