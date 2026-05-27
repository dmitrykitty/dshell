#include "logger.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


static void *logger_thread_main(void *arg) {
    Logger *logger = (Logger *) arg;
    char message[LOGGER_MESSAGE_LENGTH];

    while (1) {
        //waiting for new message
        while (sem_wait(&logger->items_available) == -1) {
            if (errno == EINTR) {
                continue;
            }

            perror("sem_wait");
            return NULL;
        }

        pthread_mutex_lock(&logger->mutex);
        //checking if any messages available and loger still working 
        int should_stop = logger->stop_requested && logger->count == 0;
        pthread_mutex_unlock(&logger->mutex);

        if (should_stop) {
            break;
        }

        while (logger_pop(logger, message, LOGGER_MESSAGE_LENGTH)) {
            logger_write_message(logger, message);
        }
    }

    return NULL;
}

static void logger_write_message(Logger *logger, const char *message) {
    char timestamp[32];
    build_timestamp(timestamp, sizeof(timestamp));

    char line[LOGGER_MESSAGE_LENGTH + 64];
    int written = snprintf(line, sizeof(line), "[%s] %s\n", timestamp, message);

    if (written < 0) {
        return;
    }

    //if snprintf tried to write more chars and didn't add \n in the end
    size_t length = (size_t) written;
    if (length >= sizeof(line)) {
        length = sizeof(line) - 1;
        line[length - 1] = '\n';
    }

    if (write_all(logger->fd, line, length) == -1) {
        perror("logger write");
    }
}

static int logger_pop(Logger *logger, char *buffer, size_t size) {
    pthread_mutex_lock(&logger->mutex);

    if (logger->count == 0) {
        pthread_mutex_unlock(&logger->mutex);
        return 0;
    }

    snprintf(buffer, size, "%s", logger->messages[logger->head]);

    logger->head = (logger->head + 1) % LOGGER_QUEUE_CAPACITY;
    logger->count--;

    pthread_mutex_unlock(&logger->mutex);
    return 1;
}

int logger_init(Logger *logger, const char *path) {
    if (logger == NULL || path == NULL || path[0] == '\0') {
        return -1;
    }

    memset(logger, 0, sizeof(*logger));
    logger->fd = -1;

    logger->fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (logger->fd == -1) {
        perror(path);
        return -1;
    }

    //mutex initialization
    if (pthread_mutex_init(&logger->mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        close(logger->fd);
        logger->fd = -1;
        return -1;
    }

    //semaphore initialization
    if (sem_init(&logger->items_available, 0, 0) == -1) {
        perror("sem_init");
        pthread_mutex_destroy(&logger->mutex);
        close(logger->fd);
        logger->fd = -1;
        return -1;
    }

    //creating new thread: run function logger_thread_main in new thread passing logger as argument
    if (pthread_create(&logger->thread, NULL, logger_thread_main, logger) != 0) {
        perror("pthread_create");
        sem_destroy(&logger->items_available);
        pthread_mutex_destroy(&logger->mutex);
        close(logger->fd);
        logger->fd = -1;
        return -1;
    }

    logger->initialized = 1;
    logger_log(logger, "logger started");

    return 0;
}

void logger_log(Logger *logger, const char *message) {
    if (logger == NULL || !logger->initialized || message == NULL) {
        return;
    }

    //locking logger to add new message for working thread
    pthread_mutex_lock(&logger->mutex);

    if (logger->count == LOGGER_QUEUE_CAPACITY) {
        logger->dropped_messages++;
        pthread_mutex_unlock(&logger->mutex);
        return;
    }

    snprintf(logger->messages[logger->tail], LOGGER_MESSAGE_LENGTH, "%s", message);

    logger->tail = (logger->tail + 1) % LOGGER_QUEUE_CAPACITY;
    logger->count++;

    pthread_mutex_unlock(&logger->mutex);

    //notify working thread about new message
    sem_post(&logger->items_available);
}

void logger_logf(Logger *logger, const char *format, ...){
    if (logger == NULL || !logger->initialized || format == NULL) {
        return;
    }

    char message[LOGGER_MESSAGE_LENGTH];

    va_list args;
    //list of arguments stats after format variable
    va_start(args, format);
    int written = vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (written < 0) {
        return;
    }

    logger_log(logger, message);
}

void logger_shutdown(Logger *logger){
    if (logger == NULL || !logger->initialized) {
        return;
    }

    logger_log(logger, "logger stopping");

    pthread_mutex_lock(&logger->mutex);
    logger->stop_requested = 1;
    pthread_mutex_unlock(&logger->mutex);

    //notifying working thread
    sem_post(&logger->items_available);

    pthread_join(logger->thread, NULL);

    if (logger->dropped_messages > 0) {
        char message[LOGGER_MESSAGE_LENGTH];
        snprintf(message, sizeof(message), "logger dropped %d messages", logger->dropped_messages);
        logger_write_message(logger, message);
    }

    sem_destroy(&logger->items_available);
    pthread_mutex_destroy(&logger->mutex);

    close(logger->fd);
    logger->fd = -1;
    logger->initialized = 0;
}
