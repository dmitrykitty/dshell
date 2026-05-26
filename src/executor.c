#include "executor.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

static int setup_input_redirection(const Command* cmd){
    if(cmd->input_file == NULL){
        return 0;
    }

    //getting file descriptor
    int fd = open(cmd->input_file, O_RDONLY);
    if(fd == -1){
        perror(cmd->input_file);
        return -1;
    }

    //everything what was read from terminal before now read from file 
    // stdin -> terminal 
    // fd -> name_of_file.txt
    //after dup2
    // stdin -> name_of_file.txt
    // fd -> name_of_file.txt

    //and after cloasinng fd only stdin -> name_of_file.txt left
    if(dup2(fd, STDIN_FILENO) == -1){
        perror("dup2");
        close(fd);
        return -1; 
    }

    close(fd);
    return 0;
}

static int setup_output_redirection(const Command* cmd){
    if(cmd->output_file == NULL){
        return 0;
    }

    int flags = O_WRONLY | O_CREAT; 
    flags |= cmd->append_output ? O_APPEND : O_TRUNC;

    //chmod for created file
    int fd = open(cmd->output_file, flags, 0644); 
    if(fd == -1){
        perror(cmd->output_file);
        return -1;
    }

    if(dup2(fd, STDOUT_FILENO) == -1){
        perror("dup2");
        close(fd);
        return -1; 
    }

    close(fd);
    return 0;
}

static int setup_redirection(const Command* cmd){
    if(setup_input_redirection(cmd) != 0){
        return -1;
    }

    if(setup_output_redirection(cmd) != 0){
        return -1;
    }

    return 0;
}



int execute_external(Command* cmd, ShellState* state, const char* command_text){
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
        if(setup_redirection(cmd) != 0){
            _exit(1);
        }
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
