#pragma once

#include "command.h"
#include "shell_state.h"

int execute_external(Command *cmd, ShellState *state, const char *command_text);