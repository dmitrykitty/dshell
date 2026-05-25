#define MAX_HISTORY 256
#define MAX_LINE_LENGTH 256

#pragma once 

typedef struct {
    char lines[MAX_HISTORY][MAX_LINE_LENGTH];
    int count;
    int head;
    long next_number; 
} History;

void history_init(History *history);
void history_add(History *history, const char *line);
void history_print(const History *history);