#pragma once 

#include "command.h"
#include "pipeline.h"

int parse_command(char *line, Command *cmd);
int parse_pipeline(char *line, Pipeline *pipeline);