CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -I./include

DEBUG_FLAGS = -g3 -O0 -DDEBUG

TARGET = dshell
SRCS = main.c \
       src/shell.c \
       src/parser.c \
       src/builtins.c \
       src/history.c \
       src/utils.c \
	   src/jobs.c \
	   src/executor.c \
	   src/signals.c \
	   src/logger.c

OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS) *.o *.d *.log

debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

.PHONY: clean debug

-include $(DEPS)
