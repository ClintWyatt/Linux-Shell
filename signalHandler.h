#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

//struct sigaction actSignal;//used for the signaling

void child(pid_t pid);

void parent();
