#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parser.h"
#include "shell.h"
#include "command.h"

void print_args(char** argv){
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

    while(1){
        printf("> "); 
        //to show immediately
        fflush(stdout);

        if(getline(&line, &size, stdin) == -1){
            break;
        }

        Command cmd; 

        if(parse_command(line, &cmd) != 1){
            continue;
        }

        if(cmd.argv[0] == NULL){
            continue;
        }

        if(strcmp(cmd.argv[0], "exit") == 0){
            break;
        }

        print_args(cmd.argv); 
    }

    free(line);
}