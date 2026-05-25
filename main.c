#include <stdio.h>

#include "shell.h"

void print_welcome(); 

int main(int argc, char** argv){
    print_welcome();
    
    start_shell_loop(); 
    return 1; 
}

void print_welcome(){
    printf("----------------------\n");
    printf("Welcome to the DShell!\n");
    printf("Print 'help' to see all available commands\n");
    printf("----------------------\n");
}