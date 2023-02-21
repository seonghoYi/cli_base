#define _GNU_SOURCE


#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "thread.h"
#include "mutex.h"

#include "cli.h"
#include "term.h"

static int g_isMainRunning = 1;


void sighandle_exit(int signo)
{
    printf("SIGNAL :: EXIT (%d, %d)\n", signo, getpid());
    g_isMainRunning = 0;
}

void sighandle_ignore(int signo)
{
    printf("SIGNAL :: IGNORE (%d, %d)\n", signo, getpid());
}

void sigaction_segv(int signo, siginfo_t* siginfo, void* context)
{
    printf("SIGNAL :: SEGV (%d, %d)\n", signo, getpid());
    exit(1);
}


void set_signal(void)
{
    struct sigaction act_print_exit;
    struct sigaction act_print_segv;
    struct sigaction act_print_ignore;

    memset(&act_print_exit, 0, sizeof(struct sigaction));
    memset(&act_print_segv, 0, sizeof(struct sigaction));
    memset(&act_print_ignore, 0, sizeof(struct sigaction));

    act_print_exit.sa_handler = sighandle_exit;
    act_print_ignore.sa_handler = sighandle_ignore;
    act_print_segv.sa_sigaction = sigaction_segv;

    sigemptyset(&act_print_exit.sa_mask);
    sigemptyset(&act_print_segv.sa_mask);
    sigemptyset(&act_print_ignore.sa_mask);

    act_print_exit.sa_flags = 0;
    act_print_ignore.sa_flags = 0;
    act_print_segv.sa_flags = SA_SIGINFO;

    sigaction(SIGINT, &act_print_exit, NULL);
    sigaction(SIGABRT, &act_print_exit, NULL);
    sigaction(SIGILL, &act_print_exit, NULL);
    sigaction(SIGFPE, &act_print_exit, NULL);
    sigaction(SIGSYS, &act_print_exit, NULL);
    sigaction(SIGXCPU, &act_print_exit, NULL);
    sigaction(SIGXFSZ, &act_print_exit, NULL);
    sigaction(SIGTERM, &act_print_exit, NULL);
    sigaction(SIGQUIT, &act_print_exit, NULL);
    sigaction(SIGPIPE, &act_print_exit, NULL);

    sigaction(SIGUSR1, &act_print_ignore, NULL);
    sigaction(SIGUSR2, &act_print_ignore, NULL);
    sigaction(SIGCHLD, &act_print_ignore, NULL);

    sigaction(SIGSEGV, &act_print_segv, NULL);
}


int main(int argc, char** argv)
{
    set_signal();

    
    cliInit();

    cliOpen();

    while(g_isMainRunning)
    {
        cliSpinOnce();
    }

    cliClose();
    cliDeinit();

    exit(0);
}