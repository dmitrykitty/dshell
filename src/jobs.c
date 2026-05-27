#include "jobs.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

/*
 * Initialize the in-memory table used for background jobs.
 * Completed jobs stay in the table so the user can still see them with jobs.
 */
void job_table_init(JobTable* table){
    if (table == NULL) {
        return;
    }

    table->count = 0;
    table->next_id = 1;
}

/*
 * Add a newly-started background process.
 * The command string is copied because the parser buffer is reused next loop.
 */
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

/*
 * Find a job by the shell job id used by commands like kill %1.
 */
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

/*
 * Poll running background jobs without blocking the shell.
 * waitpid(..., WNOHANG) also reaps finished children to avoid zombies.
 */
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
            //No child process exists for this pid, so treat it as no longer running.
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

/*
 * Print all known jobs, including completed ones.
 */
void job_table_print(const JobTable *table){
    if (table == NULL) {
        return;
    }

    for (int i = 0; i < table->count; i++) {
        const Job *job = &table->jobs[i];
        job_print(job);
    }
}

/*
 * Print one job in a compact shell-like format.
 */
void job_print(const Job* job){
    if (job->status == JOB_RUNNING) {
        printf("[%d] %d running %s\n", job->id, job->pid, job->command);
    } else if (job->status == JOB_DONE) {
        printf("[%d] %d done(%d) %s\n", job->id, job->pid, job->exit_code, job->command);
    } else if (job->status == JOB_TERMINATED) {
        printf("[%d] %d terminated(signal %d) %s\n", job->id, job->pid, job->term_signal, job->command);
    }
}
