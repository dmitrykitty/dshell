#pragma once

#include <stdio.h>

char* trim(char* line);
int parse_positive_long(const char *text, long *result);
int write_all(int fd, const char *buffer, size_t length);
void build_timestamp(char *buffer, size_t size);