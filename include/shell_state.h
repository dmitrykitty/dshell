#pragma once

#include "history.h"

typedef struct {
    int last_status;
    History history;
} ShellState;