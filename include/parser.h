#pragma once 

#include "command.h"

char* trim(char* line);
int parse_command(char *line, Command *cmd);