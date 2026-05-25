CC = gcc
CFLAGS = -Wall -Wextra -std=c11
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -I./include

TARGET = dshell
SRCS = main.c \
       src/shell.c \
       src/parser.c \
       src/builtins.c \
       src/history.c \
       src/utils.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS) *.o *.d *.log

.PHONY: clean

-include $(DEPS)
