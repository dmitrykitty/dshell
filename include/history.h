#define MAX_HISTORY 256
#define MAX_LINE_LENGTH 1024
#define HISTORY_FILE_NAME ".dshell_history"

#pragma once 

#include <stdio.h>

typedef struct {
    char lines[MAX_HISTORY][MAX_LINE_LENGTH];
    int count;
    int head;
    long next_number; 
} History;

void history_init(History *history);
void history_add(History *history, const char *line);
void history_print(const History *history);

int history_default_path(char *buffer, size_t size);
void history_load(History *history, const char *path);
int history_save_line(const char *path, const char *line);