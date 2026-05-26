#include <stdio.h>

#include "shell.h"

void print_welcome(); 

int main(void){
    print_welcome();
    
    start_shell_loop(); 
    return 0; 
}

void print_welcome(){
    printf("----------------------\n");
    printf("Welcome to the DShell!\n");
    printf("Print 'help' to see all available commands\n");
    printf("----------------------\n");
}