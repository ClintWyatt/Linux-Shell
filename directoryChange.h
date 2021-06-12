#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <stdio.h>
/*
 *@Name: directoryChange
 *@Params: char cmd
 *@returns: none
 *@Description: Method attempts to change the direcory for the user. The first 2 characters of the string must be "cd"
 */
void directoryChange(char *cmd);
