#include "pipeLine.h"


void parsePipeArgs(const int size, int numPipes, char *cmd)
{
	char *pipeCommands[numPipes+1][size]; //2d string array that will hold the commands of each command on the right and left of the pipe symbol
	char *command;  // used to tokenize the user input while checking for a pipe "|"
	int pipeIndex[numPipes]; //used as a flag for all pipes in the command argument
	int index =0, j =0, pos =0; //used to set the NULL as the last argument       
	char *args[size];
	
	command = strtok(cmd, " ");
	//here, we are putting all input without a space into a array of strings. If the current substring is a pipe("|"), then the string will set to NULL and added to the array of
	//strings, which is the args variable. strtok tokenizes the cmd buffer, which is what proceses the user input.
	while(command != NULL)
	{
		if(strcmp(command, "|") !=0) //the string is not a pipe symbol
		{
			args[index] = command;          //adding the command to the array of strings
			index++;                        //going to the next index
			command = strtok(NULL, " \n");  // move to the next string that is not a space. Recently changed
		}
		else    //string is a pipe
		{
			pipeIndex[pos] = index;                 //setting the current index "pos" to the 
			pos++;                                  //going to the next index
			command = strtok(NULL, " \n");          // move to the next string that is not a space. Recently added \n
			args[index] = NULL;
			index++;
		}

	}
	pos =0; //resetting the index for the pipeIndex array
	int i =0; //used for the index the command is added at in the 2d string array
	for(j =0; j < index +1; j++) //setting up the strings jor all commands in a 2d string array called pipeComamnds
	{
		if(j < pipeIndex[pos]) //if the command is not a pipe
		{
			pipeCommands[pos][i] = args[j];//assign left side of the pipe index to args index(contains the entire string by the user)
			i++;
		}
		else if( j == pipeIndex[pos])
		{
			pipeCommands[pos][i] = args[j];//setting the current index to NULL instead of the pipe symbol
			pos++; //going to the next string
			i = 0;
		}
		else
		{
			pipeCommands[pos][i] = args[j];//adding the args[f] to the pipeCommands[pos][i], which is on the right side of the pipe symbol
			i++;
		}
	}
	
	pipeMethod(size, pipeCommands, numPipes);

	exit(0);
}

void pipeMethod(const int size, char *pipeCommands[][size], int numPipes)
{
	//for multiple pipes, the parent process must fork a different child process to handle the stdin side of the pipe! Redo this part
	int i=0, e =0, pipeIndex =0, childProcess;
	pid_t pid;
	int fds[numPipes *2];
	for(e; e < numPipes; e++)
	{
		if((pipe(fds + (e *2))) == -1)
		{
			perror("pipe");
			exit(1);
		}
	}
	for(i; i < numPipes+1; i++)
	{

		if((pid = fork())== -1)
		{
			perror("fork");
			exit (1);
		}
		else if(pid > 0 && i == 0)//first pipe
		{
			//printf("first pipe \n");
			if((dup2(fds[1], fileno(stdout))) == -1)
			{
				perror("dup2");
				exit(1);
			}
			
			for(e = 0; e < numPipes*2; e+=2)
			{
				close(fds[e]);
				close(fds[e+1]);
			}
			
			execvp(pipeCommands[i][0], pipeCommands[i]);
			perror("execvp");
		}
		else if(pid > 0 && i < numPipes)//not on last pipe
		{
			//printf("before last pipe \n");
			if((dup2(fds[pipeIndex -2], fileno(stdin))) == -1)
			{
				perror("dup2");
				exit (1);
			}

			if((dup2(fds[pipeIndex +1], fileno(stdout))) == -1)
			{
				perror("dup2");
				exit(1);
			}


			for(e = 0; e < numPipes*2; e+=2)
			{
				close(fds[e]);
				close(fds[e+1]);
			}

			execvp(pipeCommands[i][0], pipeCommands[i]);
			perror("execvp");
		}
		else if(pid > 0 && i == numPipes)//last pipe
		{
			if((dup2(fds[pipeIndex-2], fileno(stdin))) == -1)
			{
				perror("dup2");
				exit(1);
			}

			for(e = 0; e < numPipes*2; e+=2)
			{
				close(fds[e]);
				close(fds[e+1]);
			}
			execvp(pipeCommands[i][0], pipeCommands[i]);
			perror("execvp");
		}
		else//parent process
		{
			wait(&childProcess);
			pipeIndex +=2;
		}
	}
	for(e = 0; e < numPipes*2; e+=2)
	{
		close(fds[e]);
		close(fds[e+1]);
	}

}
