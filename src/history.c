#include "history.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


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

int history_default_path(char *buffer, size_t size){
    if (buffer == NULL || size == 0) {
        return -1;
    }
    const char* home = getenv("HOME");
    int written; 

    if(home != NULL && home[0] != '\0'){
        written = snprintf(buffer, size, "%s/%s", home, HISTORY_FILE_NAME);
    } else {
        written = snprintf(buffer, size, "%s", HISTORY_FILE_NAME);
    }

    if(written < 0 || (size_t)written >= size){
        errno = ENAMETOOLONG; 
        return -1; 
    }
    return 0; 
}

void history_load(History *history, const char *path){
    if(history == NULL || path == NULL || path[0] == '\0'){
        return;
    }

    FILE* file = fopen(path, "r");
    if (file == NULL) {
        if (errno != ENOENT) {
            perror(path);
        }
        //if not exists yet - not mistake 
        return;
    }

    char line[MAX_LINE_LENGTH]; 

    //in the end of line fgets added \n if it possible 
    while(fgets(line, MAX_LINE_LENGTH, file) != NULL){
        char *newline = strchr(line, '\n');

        if (newline != NULL) {
            *newline = '\0';
        }

        if(line[0] != '\0'){
            history_add(history, line);
        }
    }

    //fgets returns NULL in 2 cases: 
    //  - EOF
    //  - error
    // so perror helps to recognize it 
    if(ferror(file)){
        perror(path);
    }

    fclose(file);
}

int history_save_line(const char *path, const char *line) {
    if (path == NULL || path[0] == '\0' || line == NULL || line[0] == '\0') {
        return 0;
    }

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror(path);
        return -1;
    }

    //writing entire command with new line 
    if (write_all(fd, line, strlen(line)) == -1 || write_all(fd, "\n", 1) == -1) {
        perror(path);
        close(fd);
        return -1;
    }

    if (close(fd) == -1) {
        perror(path);
        return -1;
    }

    return 0;
}