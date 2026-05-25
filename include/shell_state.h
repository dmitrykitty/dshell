#pragma once

#include "history.h"
#include "jobs.h"

typedef struct {
    int last_status;
    History history;
    JobTable jobs;
} ShellState;