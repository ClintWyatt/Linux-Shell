#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

void parsePipeArgs(const int size, int numPipes, char *cmd);
void pipeMethod(const int size, char *pipeCommands[][size], int numPipes);

