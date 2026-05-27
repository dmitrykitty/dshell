#include "signals.h"

#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

static volatile sig_atomic_t sigchld_received = 0;

//Handle Ctrl+C in the shell process by printing a newline.
static void handle_sigint(int signo) {
    (void) signo;
    
    //Async-signal-safe output.
    //We do not use printf() inside signal handlers.
    const char newline = '\n';
    write(STDOUT_FILENO, &newline, 1);
}

//Set a flag when a child process exits or is terminated.
static void handle_sigchld(int signo) {
    (void) signo;
    sigchld_received = 1;
}

void signals_init(void){

    struct sigaction sa_int;
    //initialize all fields before setting the handler
    memset(&sa_int, 0, sizeof(sa_int));

    sa_int.sa_handler = handle_sigint;

    //sa_mask keeps signals that should be blocked during the handler
    //so we keep mask empty to not block any other signals
    sigemptyset(&sa_int.sa_mask);

    //Do not use SA_RESTART for SIGINT. This allows getline() to return
    //with EINTR after Ctrl+C, so the shell can redraw the prompt.
    sa_int.sa_flags = 0;

    //install configuration for SIGINT signal
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction SIGINT");
    }

    struct sigaction sa_chld;
    memset(&sa_chld, 0, sizeof(sa_chld));

    sa_chld.sa_handler = handle_sigchld;
    
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
        perror("sigaction SIGCHLD");
    }
}

//Reset signal configuration for child processes because they inherit it from the shell.
void signals_restore_defaults_for_child(void){
    signal(SIGINT, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
}

int signals_has_sigchld(void){
    return sigchld_received;
}

void signals_clear_sigchld(void){
    sigchld_received = 0;
}
