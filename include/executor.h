#pragma once

#include "command.h"
#include "shell_state.h"
#include "pipeline.h"

int execute_external(Command *cmd, ShellState *state, const char *command_text);
int execute_pipeline(Pipeline *pipeline, ShellState *state);