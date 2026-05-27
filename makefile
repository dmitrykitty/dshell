CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -I./include

DEBUG_FLAGS = -g3 -O0 -DDEBUG

BUILD_DIR = build
TARGET = $(BUILD_DIR)/dshell
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

OBJS = $(addprefix $(BUILD_DIR)/,$(SRCS:.c=.o))
DEPS = $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	rm -rf src/bin
	rm -f dshell main.o main.d src/*.o src/*.d *.log

test: $(TARGET)
	sh test/run_examples.sh

debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

.PHONY: clean debug test

-include $(DEPS)
