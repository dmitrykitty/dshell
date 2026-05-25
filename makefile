CC = gcc
#FLAGS ?= -O2
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -I./include 

TARGET = dshell
OBJS = main.o shell.o parser.o

#for linking all together 
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

main.o : main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

shell.o : ./src/shell.c
	$(CC) $(CFLAGS) -c ./src/shell.c -o shell.o

parser.o : ./src/parser.c
	$(CC) $(CFLAGS) -c ./src/parser.c -o parser.o

clean:
	rm -rf *.o *.log