#pragma once

#define MAX_ARGS 64

typedef struct {
    char* argv[MAX_ARGS];
    int argc;

    char* input_file;
    char* output_file; 

    int append_output; 
    int background;
} Command;

