#include "jobs.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

void job_table_init(JobTable* table){
    if (table == NULL) {
        return;
    }

    table->count = 0;
    table->next_id = 1;
}

Job* job_table_add(JobTable* table, pid_t pid, const char* command){
    if (table == NULL || command == NULL) {
        return NULL;
    }

    if (table->count >= MAX_JOBS) {
        fprintf(stderr, "jobs: job table is full\n");
        return NULL;
    }

    Job* job = &table->jobs[table->count];
    job->id = table->next_id++; 
    job->pid = pid; 
    job->status = JOB_RUNNING; 
    job->exit_code = 0; 
    job->term_signal = 0; 
    snprintf(job->command, MAX_JOB_COMMAND, "%s", command);

    table->count++; 

    return job; 
}

Job* job_table_find_by_id(JobTable* table, int id){
    if (table == NULL) {
        return NULL;
    }

    for (int i = 0; i < table->count; i++) {
        if (table->jobs[i].id == id) {
            return &table->jobs[i];
        }
    }

    return NULL;
}

void job_table_refresh(JobTable *table){
    if(table == NULL){
        return;
    }

    for(int i = 0; i < table->count; i++){
        Job* job = &table->jobs[i];

        if(job->status != JOB_RUNNING){
            continue;
        }

        int status; 
        pid_t result = waitpid(job->pid, &status, WNOHANG);

        if(result == 0){
            continue;
        }

        if (result == -1) {
            //no child process exists
            if (errno == ECHILD) {
                job->status = JOB_DONE;
            } else {
                perror("waitpid");
            }

            continue;
        }

        if(WIFEXITED(status)){
            job->status = JOB_DONE; 
            job->exit_code = WEXITSTATUS(status);
        } else if(WIFSIGNALED(status)) {
            job->status = JOB_TERMINATED; 
            job->term_signal = WTERMSIG(status);
        }
    }
}

void job_table_print(const JobTable *table){
    if (table == NULL) {
        return;
    }

    for (int i = 0; i < table->count; i++) {
        const Job *job = &table->jobs[i];
        job_print(job);
    }
}

void job_print(const Job* job){
    if (job->status == JOB_RUNNING) {
        printf("[%d] %d running %s\n", job->id, job->pid, job->command);
    } else if (job->status == JOB_DONE) {
        printf("[%d] %d done(%d) %s\n", job->id, job->pid, job->exit_code, job->command);
    } else if (job->status == JOB_TERMINATED) {
        printf("[%d] %d terminated(signal %d) %s\n", job->id, job->pid, job->term_signal, job->command);
    }
}