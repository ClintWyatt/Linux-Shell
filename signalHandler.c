#include "signalHandler.h"

void parent()
{
	struct sigaction actSignal;
        /* After this method executes, all signals for the parent process that can be overwritten will be ignored */
        tcsetpgrp(fileno(stdin), getpgrp()); //makes the process with the process group id pgrp the foreground process group on the terminal.
        actSignal.sa_handler = SIG_IGN; //specifies the actSignalion to be associated with signum. sig_ign means to ignore the signal.
        assert(sigaction(SIGHUP, &actSignal, NULL) ==0);
        assert(sigaction(SIGINT, &actSignal, NULL) ==0); //prevents ctrl c from terminating the sehll
        assert(sigaction(SIGQUIT, &actSignal, NULL) ==0); //prevents quit form the keyboard from terminating the shell
        assert(sigaction(SIGILL, &actSignal, NULL) ==0); //prevents an illegal instruction from killing the shell
        assert(sigaction(SIGABRT, &actSignal, NULL) ==0); //prevents a abort signal from killing the shell
        assert(sigaction(SIGFPE, &actSignal, NULL) ==0); //prevents a floating point exception from killing the shell
        assert(sigaction(SIGSEGV, &actSignal, NULL) ==0); //prevents invalid memory reference from killing the shell
        assert(sigaction(SIGPIPE, &actSignal, NULL) ==0); //prevents a broken pipe from killing the shell
        assert(sigaction(SIGALRM, &actSignal, NULL) ==0);
        assert(sigaction(SIGTERM, &actSignal, NULL) ==0);
        assert(sigaction(SIGUSR1, &actSignal, NULL) ==0);
        assert(sigaction(SIGUSR2, &actSignal, NULL) ==0);
        assert(sigaction(SIGTSTP, &actSignal, NULL) ==0); //prevents ctrl z from terminating the shell.
        assert(sigaction(SIGTTIN, &actSignal, NULL) ==0);
        assert(sigaction(SIGTTOU, &actSignal, NULL) ==0);
        assert(sigaction(SIGBUS, &actSignal, NULL) ==0); //prevents bad memory access from killing the shell
        assert(sigaction(SIGPOLL, &actSignal, NULL) ==0);
        assert(sigaction(SIGPROF, &actSignal, NULL) ==0);
        assert(sigaction(SIGSYS, &actSignal, NULL) ==0);
        assert(sigaction(SIGTRAP, &actSignal, NULL) ==0);
        assert(sigaction(SIGVTALRM, &actSignal, NULL) ==0); //prevents virtual alarm clock from killing the shell
        assert(sigaction(SIGXCPU, &actSignal, NULL) ==0); //prevents CPU time limit exceeded from killing the shell
        assert(sigaction(SIGXFSZ, &actSignal, NULL) ==0); //prevents file size limit exceeded from killing the shell
        assert(sigaction(SIGIOT, &actSignal, NULL) ==0);
        assert(sigaction(SIGSTKFLT, &actSignal, NULL) ==0);
        assert(sigaction(SIGIO, &actSignal, NULL) ==0);
        assert(sigaction(SIGPWR, &actSignal, NULL) ==0);
        assert(sigaction(SIGWINCH, &actSignal, NULL) ==0);
        //assert(sigaction(SIGUNUSED, &actSignal, NULL) ==0);
}

void child(pid_t pid)
{
	struct sigaction actSignal;
        setpgrp(); //sets process group of own process to itself
        tcsetpgrp(fileno(stdin), getpgid(pid));//having the child process in the foreground
        actSignal.sa_handler = SIG_DFL; //default signal
        assert(sigaction(SIGHUP, &actSignal, NULL) ==0);
        assert(sigaction(SIGINT, &actSignal, NULL) ==0);
        assert(sigaction(SIGQUIT, &actSignal, NULL) ==0);
        assert(sigaction(SIGILL, &actSignal, NULL) ==0);
        assert(sigaction(SIGABRT, &actSignal, NULL) ==0);
        assert(sigaction(SIGFPE, &actSignal, NULL) ==0);
        assert(sigaction(SIGSEGV, &actSignal, NULL) ==0); //prevents invalid memory reference from killing the shell
        assert(sigaction(SIGPIPE, &actSignal, NULL) ==0); //prevents a broken pipe from killing the shell
        assert(sigaction(SIGALRM, &actSignal, NULL) ==0);
        assert(sigaction(SIGTERM, &actSignal, NULL) ==0);
        assert(sigaction(SIGUSR1, &actSignal, NULL) ==0);
        assert(sigaction(SIGUSR2, &actSignal, NULL) ==0);
        //assert(sigaction(SIGTSTP, &actSignal, NULL) ==0); //prevents ctrl z from terminating the shell.
        assert(sigaction(SIGTTIN, &actSignal, NULL) ==0);
        assert(sigaction(SIGTTOU, &actSignal, NULL) ==0);
        assert(sigaction(SIGBUS, &actSignal, NULL) ==0); //prevents bad memory access from killing the shell
        assert(sigaction(SIGPOLL, &actSignal, NULL) ==0);
        assert(sigaction(SIGPROF, &actSignal, NULL) ==0);
        assert(sigaction(SIGSYS, &actSignal, NULL) ==0);
        assert(sigaction(SIGTRAP, &actSignal, NULL) ==0);
        assert(sigaction(SIGVTALRM, &actSignal, NULL) ==0); //prevents virtual alarm clock from killing the shell
        assert(sigaction(SIGXCPU, &actSignal, NULL) ==0); //prevents CPU time limit exceeded from killing the shell
        assert(sigaction(SIGXFSZ, &actSignal, NULL) ==0); //prevents file size limit exceeded from killing the shell
        assert(sigaction(SIGIOT, &actSignal, NULL) ==0);
        assert(sigaction(SIGSTKFLT, &actSignal, NULL) ==0);
        assert(sigaction(SIGIO, &actSignal, NULL) ==0);
        assert(sigaction(SIGPWR, &actSignal, NULL) ==0);
        assert(sigaction(SIGWINCH, &actSignal, NULL) ==0);
        //assert(sigaction(SIGUNUSED, &actSignal, NULL) ==0);

}
