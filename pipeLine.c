#include "pipeLine.h"


void parsePipeArgs(const int size, int numPipes, char *cmd)
{
	char *pipeCommands[numPipes+1][size]; //2d string array that will hold the commands of each command on the right and left of the pipe symbol
	char *command;  // used to tokenize the user input while checking for a pipe "|"
	int allPipes[numPipes][2]; //2d array of file descriptors
	int pipeIndex[numPipes]; //used as a flag for all pipes in the command argument
	command = strtok(cmd, " "); //tokenizing all spaces
	int index =0; //used to set the NULL as the last argument       
	int j =0; //used for the right side of the pipe if there are only 2 arguments 
	//pid_t p1, p2; 
	int pos =0; //used for the pipeIndex
	int t =0; //used for the loop that can handle a large number of pipes
	char *args[size];
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
	int f; //used in the next for loop
	int i =0; //used for the index the command is added at in the 2d string array
	for(f =0; f < index +1; f++) //setting up the strings for all commands in a 2d string array called pipeComamnds
	{
		if(f < pipeIndex[pos]) //if the command is not a pipe
		{
			pipeCommands[pos][i] = args[f];//assign left side of the pipe index to args index(contains the entire string by the user)
			i++;
		}
		else if( f == pipeIndex[pos])
		{
			pipeCommands[pos][i] = args[f];//setting the current index to NULL instead of the pipe symbol
			pos++; //going to the next string
			i = 0;
		}
		else
		{
			pipeCommands[pos][i] = args[f];//adding the args[f] to the pipeCommands[pos][i], which is on the right side of the pipe symbol
			i++;
		}
	}
	pos =0;//resetting the poisition, which will be used to access the pipeCommands index when using dup2

	int e =0; //used to error check all pipes that are being created
	int k =0;
	for( e; e < numPipes; e++) //creating all the pipes
	{
		if(pipe(allPipes[e]) ==-1)
		{
			perror("pipe");
			return; //exiting the method
		}

	}

	if(numPipes == 1)
	{
		onePipe(size, pipeCommands);
	}
	else//more than 1 pipe
	{
		//for multiple pipes, the parent process must fork a different child process to handle the stdin side of the pipe! Redo this part
		int numProcesses=0, childProcess;
		int e =0; //used to error check all pipes that are being created
		int fd[2];
		pid_t pid;
		pid = fork();
		
		if(pid == -1)
		{
			perror("fork");
			exit(0);
		}
		else if(pid > 0)
		{
			pid_t pids[numPipes +1];
			for(numProcesses; numProcesses < numPipes+1; numProcesses++)
			{
				pipe(fd);
				pids[numProcesses]= fork();
				if(pids[numProcesses] == -1)
				{
					perror("fork");
					exit (1);
				}
				else if(pids[numProcesses] > 0 && numProcesses == 0)//first pipe
				{
					dup2(fd[1], fileno(stdout));
					close(fd[0]);
					close(fd[1]);
					execvp(pipeCommands[numProcesses][0], pipeCommands[numProcesses]);
				}
				else if(pids[numProcesses] > 0 && numProcesses < numPipes)//not on last pipe
				{
					dup2(fd[0], fileno(stdin));
					dup2(fd[1], fileno(stdout));
					close(fd[0]);
					close(fd[1]);
					execvp(pipeCommands[numProcesses][0], pipeCommands[numProcesses]);
					perror("execvp");
				}
				else if(pids[numProcesses] > 0 && numProcesses == numPipes)//last pipe
				{
					dup2(fd[0], fileno(stdin));
					close(fd[1]);
					close(fd[0]);
					execvp(pipeCommands[numProcesses][0], pipeCommands[numProcesses]);
					perror("execvp");
				}
				else//parent process
				{
					exit(0);
				}
			}
		}
		else
		{
			wait(&childProcess);
		}

	}
	exit(0);
}

void onePipe(const int size, char *commands[][size])
{
	int fd[2];
	int childProcess;
	if((pipe(fd)) == -1)
	{
		perror("pipe");
		exit(1);
	}
	switch (fork())
	{
		case -1:
			perror("fork");
			exit(1);
		case 0:
			dup2(fd[1], fileno(stdout));
			close(fd[0]);
			close(fd[1]);
			execvp(commands[0][0], commands[0]); 
			perror("execvp onePipe method");
		default:
			wait(&childProcess);
			dup2(fd[0], fileno(stdin));
			close(fd[1]);
			close(fd[0]);
			execvp(commands[1][0], commands[1]);
			perror("evecvp onePipe method");
	}
}
