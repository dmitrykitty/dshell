#pragma once

#include "command.h"

typedef struct {
    Command left;
    Command right;
    int has_pipe;
} Pipeline;