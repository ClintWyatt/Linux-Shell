#include "simpleCommand.h"

void simpleCommand(char *command, unsigned short size)
{

        char *allArgs[size];//is essentailly a array of "strings"
        int index =0;//used to add strings to indexes of the allArgs array of strings
        char * cmd;//cmd will be added to the allArgs array until the end of the command buffer is processed
        cmd = strtok(command, " \n"); //tokenizing the command buffer for spaces
        while(cmd != NULL)
        {
                //printf("%s \n", command);
                allArgs[index] = cmd;
                index++;
                cmd = strtok(NULL, " \n");  //edited last

        }
        allArgs[index] = NULL;//setting the last index to nul

        execvp(allArgs[0], allArgs);
        printf("Command not found \n");
        exit(EXIT_FAILURE);

}
