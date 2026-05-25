#include <stdio.h>
#include <string.h>
#include <math.h>

#include "history.h"

void history_init(History *history){
    if(history == NULL){
        return;
    }
    history->head = 0;
    history->count = 0;
    history->next_number = 1;
}

void history_add(History* history, const char* line){
    if (history == NULL || line == NULL || line[0] == '\0') {
        return;
    }

    int idx; 
    if(history->count < MAX_HISTORY){
        //idx = (history->head + history->count) % MAX_HISTORY;
        idx = history->count; 
        history->count++; 
    } else {
        idx = history->head; 
        history->head = (history->head + 1) % MAX_HISTORY; 
    }

    snprintf(history->lines[idx], MAX_LINE_LENGTH, "%s", line);
    history->next_number++;
}

void history_print(const History *history){
    if(history == NULL || history->count == 0){
        return;
    }

    long cur_number = history->next_number - history->count; 

    for(int i = 0; i < history->count; i++){
        int idx = (history->head + i) % MAX_HISTORY; 
        printf("%ld %s\n", cur_number + i, history->lines[idx]);
    }
}