#pragma once

#define LOGGER_QUEUE_CAPACITY 128
#define LOGGER_MESSAGE_LENGTH 512

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    char messages[LOGGER_QUEUE_CAPACITY][LOGGER_MESSAGE_LENGTH];
    int head;
    int tail;
    int count;

    int fd;
    int initialized;
    int stop_requested;
    int dropped_messages;

    pthread_t thread;
    pthread_mutex_t mutex;
    sem_t items_available;
} Logger;

int logger_init(Logger *logger, const char *path);
void logger_log(Logger *logger, const char *message);
void logger_logf(Logger *logger, const char *format, ...);
void logger_shutdown(Logger *logger);
