#pragma once

#include <sys/types.h>

#define MAX_JOBS 64
#define MAX_JOB_COMMAND 256

typedef enum {
    JOB_RUNNING,
    JOB_DONE,
    JOB_TERMINATED
} JobStatus;

typedef struct {
    int id;
    pid_t pid;
    JobStatus status;
    int exit_code;
    int term_signal;
    char command[MAX_JOB_COMMAND];
} Job;

typedef struct {
    Job jobs[MAX_JOBS];
    int count;
    int next_id;
} JobTable;

void job_table_init(JobTable *table);
Job *job_table_add(JobTable *table, pid_t pid, const char *command);
Job *job_table_find_by_id(JobTable *table, int id);
void job_table_refresh(JobTable *table);
void job_table_print(const JobTable *table);
void job_print(const Job* job);