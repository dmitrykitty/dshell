#pragma once

#include "history.h"
#include "jobs.h"
#include "logger.h"

typedef struct {
    int last_status;
    History history;
    JobTable jobs;
    Logger logger;
} ShellState;