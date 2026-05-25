#pragma once 

#include "command.h"
#include "shell_state.h"

typedef enum {
    BUILTIN_NOT_FOUND = 0,
    BUILTIN_DONE = 1,
    BUILTIN_EXIT = 2
} BuiltinResult;

BuiltinResult execute_builtin(Command *cmd, ShellState *state);