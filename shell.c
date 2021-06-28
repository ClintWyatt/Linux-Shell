/*
   Linux Bash Shell
Author: Clint Wyatt
Version: 0.7.9
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/socket.h>
#include <pwd.h>
#include "history.h"
#include "signalHandler.h"
#include "redirection.h"
#include "simpleCommand.h"
#include "directoryChange.h"
#include "pipeLine.h"

#define SIZE 512
#define recordLimit 40
void directoryChange(char *cmd); //method changes the directory
void pathname(char *cmd);
void exitShell();
char *wspace(char *word); //Function to clear leading whitespace from batch file commands

int main(int argc, char *argv[])
{
	char cmd[SIZE];//used for taking in commands from the user
	char dir[SIZE];//current directory
	pid_t pid;
	int childProcess;
	int pos =0;
	int pipeNum = 0, numRecords =0, readRedirect =0, writeRedirect =0;
	struct commandHistory *head;
	head = (struct commandHistory *)malloc(sizeof(struct commandHistory));
	head->next = NULL;
	head->previous = NULL;
	struct commandHistory *last = (struct commandHistory *)malloc(sizeof(struct commandHistory));
	last->next = NULL;
	last->previous = NULL;
	memset(head->command, '\0', sizeof(head->command));
	memset(last->command, '\0', sizeof(last->command));
	for( ; ; )
	{
		parent(); //Having the current process ignore all signals (except kill -9) that can kill the shell
		//getcwd copies the current directory to "directory"
		printf("%s ", getcwd(dir, sizeof(dir)));
		fgets(cmd, sizeof(cmd), stdin); //getting input from the user	

		addEntry(&head, &last, numRecords, recordLimit, cmd);
		numRecords++;//may need to change later is the user spams the shell with commands

		//checking to see if the exit was typed by the user. If so, then the shell ends
		if(memcmp(cmd, "exit", 4) ==0)
		{
			if(head != NULL)
			{
				deleteHistory(head, last);
			}
			exitShell();
		} 

		for(pos =0; pos < strlen(cmd); pos++)
		{

			if(cmd[pos] == '|')
			{
				pipeNum++;//indicating there is a pipe symbol
			}
			else if(cmd[pos] == '>')
			{
				writeRedirect++;//indicating there is a write redirection symbol
			}
			else if(cmd[pos] == '<')
			{
				readRedirect++;//indicating there is a read redirection symbol
			}
		}

		if((memcmp(cmd, "cd ", 3) ==0)) //if there is "cd" at the beginning of the string
		{
			directoryChange(cmd);
		}    
		else if(memcmp(cmd,"history", 7) == 0)//if hsitory was typed
		{
			print(head);
		} 
		else if(memcmp(cmd, "$PATH",5) ==0)//if $path was typed
		{
			pathname(cmd);
		}	
		else
		{	
			//Error checking the fork system call
			if ((pid = fork()) == -1)
			{
				perror("fork() error");
			}
			else
			{

				if(pid == 0)//if this is a child process
				{

					child(pid); //setting the child process to the foreground
					if(pipeNum != 0)//if the pipeNum variable (a pipe was typed) is greater than zero, test the pipe method
					{
						parsePipeArgs(SIZE, pipeNum, cmd);
						//pipeMethod(pipeNum, 0, 0, cmd);
					}
					else if(writeRedirect !=0) //is > or >> was typed
					{
						writeRedirection(cmd, SIZE);
					}
					else if(readRedirect != 0)//if < or << was typed
					{
						readRedirection(cmd, SIZE);
					}
					else
					{
						simpleCommand(cmd, SIZE);//simple command without pipes or redirection
					}	
				}
				else //parent process
				{

					setpgid(pid, pid); 
					signal(SIGTTOU, SIG_IGN); //SIGTTOU is generated when a process in a background job attempts to write to the terminal
					tcsetpgrp(fileno(stdin), getpgid(pid)); 
					wait(&childProcess);//wait for child to finish
				}
			}
		}	

		//resetting variables
		readRedirect =0;
		writeRedirect =0;
		pipeNum =0;	
		memset(cmd, 0, SIZE); //resetting the cmd buffer   

	}
} // end of main

// pathname
void pathname(char *cmd)
{
	// printf("path\n");
	char* path = getenv("PATH");
	printf("%s\n", path);

	int i = 4;
	while (cmd[i] == ' '){
		i++;
	}
	if (cmd[i] == '+'){
		//concatenate

	}
	else if (cmd[i] == '-'){
		//remove

	}
}


void exitShell()
{
	printf("Shell Terminated \n");
	exit(0);

}
