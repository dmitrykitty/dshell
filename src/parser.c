#include "command.h"
#include "pipeline.h"

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

/*
 * Parse one simple command.
 * This parser is intentionally whitespace-based: no quotes, escapes or globbing.
 */
int parse_command(char *line, Command *cmd) {
    int argc = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_output = 0;
    cmd->background = 0;

    //split by spaces and tabs into separate tokens
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
            //next token must be the file used as stdin
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

            //next token must be the file used as stdout
            token = strtok(NULL, " \t");

            if(parse_redirection_target(&token, (append ? ">>" : ">")) == 0){
                return 0;
            }

            if(cmd->output_file != NULL){
                fprintf(stderr, "parse error: duplicate output redirection\n");
                return 0;
            }

            cmd->append_output = append; 
            cmd->output_file = token; 
            token = strtok(NULL, " \t");
            continue;
        }

        cmd->argv[argc++] = token; 
        token = strtok(NULL, " \t");
    }

    cmd->argv[argc] = NULL; 
    cmd->argc = argc; 
    return argc > 0; 
}

/*
 * Split a line into either one command or a two-command pipeline.
 * dshell currently supports one pipe, so "a | b | c" is rejected.
 */
int parse_pipeline(char* line, Pipeline* pipeline){
    char* pipe_pos = strchr(line, '|'); 

    if(pipe_pos == NULL){
        pipeline->has_pipe = 0;
        return parse_command(line, &pipeline->left);
    }

    pipeline->has_pipe = 1; 
    *pipe_pos = '\0';

    char* left_line = line; 
    char* right_line = pipe_pos + 1; 

    if (strchr(right_line, '|') != NULL) {
        fprintf(stderr, "parse error: only one pipe is supported\n");
        return 0;
    }

    if(parse_command(left_line, &pipeline->left) == 0){
        return 0;
    }

    if(parse_command(right_line, &pipeline->right) == 0){
        return 0;
    }

    if (pipeline->left.background || pipeline->right.background) {
        fprintf(stderr, "parse error: background pipelines are not supported yet\n");
        return 0;
    }

    return 1;
}
