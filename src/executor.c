#include "executor.h"
#include "signals.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

static int setup_input_redirection(const Command* cmd){
    if(cmd->input_file == NULL){
        return 0;
    }

    //open input file and get its file descriptor
    int fd = open(cmd->input_file, O_RDONLY);
    if(fd == -1){
        perror(cmd->input_file);
        return -1;
    }

    //everything that was read from terminal before now read from file
    // stdin -> terminal 
    // fd -> name_of_file.txt
    //after dup2
    // stdin -> name_of_file.txt
    // fd -> name_of_file.txt

    //and after closing fd only stdin -> name_of_file.txt left
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

    //mode for a newly created output file
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

/*
 * Run one external command in a child process.
 * Parent waits for foreground commands or records background jobs.
 */
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
        //reset signals config inside current (child) process
        signals_restore_defaults_for_child();

        if(setup_redirection(cmd) != 0){
            _exit(1);
        }
        execvp(cmd->argv[0], cmd->argv);
        perror(cmd->argv[0]);
        //command not found / exec failed
        _exit(127);
    }

    logger_logf(&state->logger, "external started: pid=%d command=%s", pid, command_text);

    //parent process
    if(cmd->background){
        const Job* job = job_table_add(&state->jobs, pid, command_text);

        if (job == NULL) {
            logger_logf(&state->logger, "background job add failed: pid=%d command=%s", pid, command_text);
            fprintf(stderr, "failed to add background job\n");
            return -1;
        }

        logger_logf(&state->logger, "background job started: pid=%d command=%s", pid, command_text);
        job_print(job);
        return 0;
    }

    int status; 
    pid_t waited;

    //waitpid could be interrupted with EINTR so we repeat it
    do {
        waited = waitpid(pid, &status, 0);
    } while(waited == -1 && errno == EINTR);

    if(waited == -1){
        perror("waitpid");
        return -1;
    }

    if(WIFEXITED(status)){
        state->last_status = WEXITSTATUS(status);
    }else if (WIFSIGNALED(status)) {
        state->last_status = 128 + WTERMSIG(status);
    }

    logger_logf(&state->logger, "external finished: pid=%d status=%d command=%s", pid, state->last_status, command_text);

    return state->last_status;
}

/*
 * Execute a two-command pipeline: left stdout is connected to right stdin.
 * Redirections are still applied inside the proper child after dup2().
 */
int execute_pipeline(Pipeline *pipeline, ShellState *state){
    if (pipeline == NULL || state == NULL) {
        return -1;
    }

    if (pipeline->left.argv[0] == NULL || pipeline->right.argv[0] == NULL) {
        return -1;
    }

    if (pipeline->left.output_file != NULL) {
        fprintf(stderr, "parse error: output redirection on left side of pipe is not supported\n");
        return -1;
    }

    if (pipeline->right.input_file != NULL) {
        fprintf(stderr, "parse error: input redirection on right side of pipe is not supported\n");
        return -1;
    }

    logger_logf(&state->logger, "pipeline started: %s | %s", pipeline->left.argv[0], pipeline->right.argv[0]);
    int pipefd[2]; 

    // pipefd[0] is the read end of the pipe.
    // pipefd[1] is the write end of the pipe.
    if(pipe(pipefd) == -1){
        perror("pipe");
        return -1; 
    }


    pid_t left_pid = fork();

    if (left_pid < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    //inside left child process
    //reset of configuration for the signals
    //we are redirecting standard output from here to pipefd[1]
    if(left_pid == 0){
        signals_restore_defaults_for_child();

        if(dup2(pipefd[1], STDOUT_FILENO) == -1){
            perror("dup2");
            _exit(1);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        if(setup_redirection(&pipeline->left) != 0){
            _exit(1);
        }
        execvp(pipeline->left.argv[0], pipeline->left.argv);
        perror(pipeline->left.argv[0]);
        //command not found / exec failed
        _exit(127);
    }

    pid_t right_pid = fork();

    if (right_pid < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(left_pid, NULL, 0);
        return -1;
    }

    //inside right child process
    //reset of configuration for the signals
    //we are redirecting standard input to take info from pipefd[0]
    if(right_pid == 0){
        signals_restore_defaults_for_child();

        if(dup2(pipefd[0], STDIN_FILENO) == -1){
            perror("dup2");
            _exit(1);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        if (setup_redirection(&pipeline->right) != 0) {
            _exit(1);
        }

        execvp(pipeline->right.argv[0], pipeline->right.argv);
        perror(pipeline->right.argv[0]);
        _exit(127);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    int left_status;
    int right_status;

    pid_t waited_left;
    pid_t waited_right;

    do {
        waited_left = waitpid(left_pid, &left_status, 0);
    } while (waited_left == -1 && errno == EINTR);

    if (waited_left == -1) {
        perror("waitpid");
        return -1;
    }

    do {
        waited_right = waitpid(right_pid, &right_status, 0);
    } while (waited_right == -1 && errno == EINTR);

    if (waited_right == -1) {
        perror("waitpid");
        return -1;
    }

    if (WIFEXITED(right_status)) {
        state->last_status = WEXITSTATUS(right_status);
    } else if (WIFSIGNALED(right_status)) {
        state->last_status = 128 + WTERMSIG(right_status);
    }

    logger_logf(&state->logger, "pipeline finished: status=%d", state->last_status);
    return state->last_status;
}
