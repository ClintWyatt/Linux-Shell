/*
Linux Bash Shell
Author: Clint Wyatt
Version: 0.7.4
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

#define SIZE 512
#define recordLimit 40
void directoryChange(char *cmd); //method changes the directory
void pipeMethod(int numPipes, int write, int read, char *cmd);//method used for the piping
void pathname(char *cmd);
void redirectionPipe();
void exitShell();
char *wspace(char *word); //Function to clear leading whitespace from batch file commands

pid_t pid1, pid2, pid3;//used for creating child processes

enum { READ, WRITE };

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
	struct commandHistory *last = NULL;
	memset(head->command, '\0', sizeof(head->command));

	for( ; ; )
	{
		parent(); //Having the current process ignore all signals (except kill -9) that can kill the shell
		//getcwd copies the current directory to "directory"
		printf("%s ", getcwd(dir, sizeof(dir)));
		fgets(cmd, sizeof(cmd), stdin); //getting input from the user	
		
		addEntry(head, last, numRecords, recordLimit, cmd);
		numRecords++;

		//checking to see if the exit was typed by the user. If so, then the shell ends
		if(memcmp(cmd, "exit", 4) ==0)
		{
			if(head != NULL)
			{
				deleteHistory(head, last);
			}
			printf("here1\n");
			exitShell();
		} 




		//checking for a pipes and redirection in the cmd stream
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
						pipeMethod(pipeNum, writeRedirect, readRedirect, cmd);
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

void pipeMethod(int numPipes, int write, int read, char *cmd)
{	
	int childProcess;
	if(numPipes ==1)
	{
		pid_t pid; //used for the fork system call
		char *leftSide[SIZE];//left side of the pipe symbol
		char *rightSide[SIZE];//right side side of the pipe symbol
		char *command;//command for each argument entered by the user
		short a =0;
		short b =0;
		short pipeIndex =0;
		short index =0;//used for the leftSide and rightSide buffers
		short writeDirect =0; //used to determine if > or >> was in stdin
		FILE *fp;
		int fd[2]; //used for dup2 system call
		command = strtok(cmd, " \n");

		while(command != NULL)
		{
			if((memcmp(command, "|", 1)) == 0) //if command is a pipe
			{
				pipeIndex++;
				leftSide[index] = NULL;//setting the last argument to null
				index =0;//reset the index after the pipe appeared. Index will now be used for the right side of the pipe
			}
			else if(pipeIndex ==0) //if the pipe symbol hasnt appeared yet
			{
				leftSide[index] = command;
				index++;	
			}
			else //if the pipe symbol has appeared
			{
				if(write > 0) //seeing if a > or >> is in the input stream
				{
					if((memcmp(command, ">>", 2) ==0)) //if command is >
					{ 
						writeDirect =2; //setting writeDirect to 2
						command = strtok(NULL, " \n");//going to the next argument from stdin
						break; //exit the loop to prevent adding the file name to the end of the rightSide of the pipe
					}
					else if((memcmp(command, ">", 1) ==0))//if command is >>
					{
						writeDirect =1; //writeDirect is 1
						command = strtok(NULL, " \n"); //going to the next argument from stdin
						break;
					}
					else
					{
						rightSide[index] = command;
						index++;
					}

				}	
				else
				{	
					rightSide[index] = command;//adding hte string to the rightSide
					index++;//going to the next index
				}
			}
			command = strtok(NULL, " \n");
		}

		rightSide[index] = NULL;

		if((pipe(fd) == -1)) //if creating the pipe failed
		{
			perror("pipe error");
			exit(1); //exit the process
		}	
		pid = fork();
		if(pid == -1)
		{
			perror("fork error");
			exit(1); //exit the process
		}
		else if(pid ==0) //child process
		{
			dup2(fd[1], fileno(stdout));
			close(fd[0]);//closing read end of the pipe
			close(fd[1]);//closing write end of the pipe	
			execvp(leftSide[0], leftSide);
			perror("execvp error on left side of the pipe \n");	
		}
		else
		{
			wait(&childProcess); //wait for the child process to finish

			if(writeDirect >0)//if there was a > or >>
			{
				//printf("%d \n", writeDirect);	
				if(writeDirect ==1)
				{
					if((fp = fopen(command, "w")) ==NULL)
					{
						perror("file not found \n");
					}
				}
				else
				{
					if((fp = fopen(command, "a")) ==NULL)
					{
						perror("file not found \n");
					}	
				}
				dup2(fileno(fp), fileno(stdout));//redirecting the output to the file
				fclose(fp);//closing the file
				dup2(fd[0], fileno(stdin));
				close(fd[0]); 
				close(fd[1]);
				execvp(rightSide[0], rightSide);

			}
			else
			{	
				dup2(fd[0], fileno(stdin));
				close(fd[0]);
				close(fd[1]);
				execvp(rightSide[0], rightSide);
				perror("execvp error right of the pipe \n");
			}	
		}
	}
	else
	{	
		char *args[SIZE]; //used for copying the argument into all pipe arguments that will be executed on either side of the pipe. Contains the entire string inputed by the user		
		char *pipeCommands[numPipes+1][SIZE]; //2d string array that will hold the commands of each command on the right and left of the pipe symbol
		char *command;	// used to tokenize the user input while checking for a pipe "|"
		int fd1[2], fd2[2];	//file descriptors
		int allPipes[numPipes][2]; //2d array of file descriptors
		int pipeIndex[numPipes]; //used as a flag for all pipes in the command argument
		command = strtok(cmd, " "); //tokenizing all spaces
		int index =0; //used to set the NULL as the last argument	
		int j =0; //used for the right side of the pipe if there are only 2 arguments 
		//pid_t p1, p2; 
		int pos =0; //used for the pipeIndex
		int t =0; //used for the loop that can handle a large number of pipes

		//here, we are putting all input without a space into a array of strings. If the current substring is a pipe("|"), then the string will set to NULL and added to the array of
		//strings, which is the args variable. strtok tokenizes the cmd buffer, which is what proceses the user input.
		while(command != NULL)
		{
			if(strcmp(command, "|") !=0) //the string is not a pipe symbol
			{
				args[index] = command;		//adding the command to the array of strings
				index++;			//going to the next index
				command = strtok(NULL, " \n");	// move to the next string that is not a space. Recently changed
			}
			else	//string is a pipe
			{
				pipeIndex[pos] = index;			//setting the current index "pos" to the 
				pos++;					//going to the next index
				command = strtok(NULL, " \n");		// move to the next string that is not a space. Recently added \n
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

		int status;//used to synchnorize the process at the end of this method by using the wait system call


		//printf("t: %d  numPipes: %d\n", t, numPipes);
		switch (fork()) //creating a child process for the left/right side of the pipe
		{
			case -1:	//error in the fork
				perror("Fork");
				exit(EXIT_FAILURE);
			case 0:// in child /
				//close all available pipes 
				//if(t ==0)
				//{

				dup2(/*fd1[1]*/allPipes[0][1] , fileno(stdout)); //overide stdout

				for(k =0; k < numPipes; k++)  //close all used pipes
				{
					close(allPipes[k][0]);
					close(allPipes[k][1]);
				}

				execvp(pipeCommands[pos][0], pipeCommands[pos]);//first command
				exit(3);
				/*	}
					else
					{
					dup2(allPipes[t][0], fileno(stdin)); //overide stdout
					k =0;
					for(k; k < numPipes; k++)  //close all used pipes
					{
					close(allPipes[k][0]); //close read end
					close(allPipes[k][1]); //close write end
					}
					execvp(pipeCommands[pos][0], pipeCommands[pos]);
					exit(3);


					}*/

			default: // in parent //

				pos++;//going to the next index of pipeCommands 2d string array, which will be processed in the next fork
				pid2 = fork();//
				if(pid2 == -1)
				{
					perror("fork");
					return;
				}
				if(pid2 ==0)
				{

					//printf("here \n");	 
					
					dup2(/*fd1[0]*/ allPipes[0][0], fileno(stdin));//overide stdin, second command, which is to the right of the pipe symbol

					if(numPipes > 1) //if there is more than 1 pipe
					{

						dup2(/*fd2[1]*/ allPipes[1][1], fileno(stdout));//sending the output to the other side of the second pipe
					}

					for(k =0; k < numPipes; k++) //closing all pipes that are to be used 
					{
						close(allPipes[k][0]);
						close(allPipes[k][1]);
					}

					if((execvp(pipeCommands[pos][0], pipeCommands[pos])) == -1) //executing second command
					{
						perror("evecvp");
						exit(EXIT_FAILURE);
					}					
				}
				else //parent process
				{
					if(numPipes > 1) //if there are more than 1 pipe, attempt to process 3rd command, which is right of the second pipe
					{
						k =0;

						pos++;
						if((pid3 = fork()) ==-1) //error in fork
						{
							perror("fork");
							exit(EXIT_FAILURE);
						}
						else if(pid3 == 0) //child process
						{
							dup2(allPipes[1][0], fileno(stdin));
							for(k = 0; k < numPipes; k++)
							{
								close(allPipes[k][0]);
								close(allPipes[k][1]);
							}

							execvp(pipeCommands[pos][0], pipeCommands[pos]);
						}

					}
				}

		}

		for(k=0; k < numPipes; k++) //closing all the pipes
		{
			close(allPipes[k][0]);
			close(allPipes[k][1]);
		}

		for(k =0; k < numPipes +1; k++)
		{
			wait(&status);
		}
		exit(0); //Ending the process. Without this exit, this process will not be terminated. 	
	}		
}


void directoryChange(char *cmd)
{
	uid_t uid;//users id
	struct passwd *usrAccount; //struct passwd holds data about the user currently runnint the shell. The usrAccount struct holds information about the user from the operation system.
	int numArgs =0;//used to determine if just "cd" was typed
	//char *home = getenv("HOME");use this to get the enviromental variable of the home dirrectory
	char *command;
	char *address;
	command = strtok(cmd, " \n");//tokenizing the cmd input for spaces and the next line(\n)

	while(command != NULL)//tokenizing the command char pointer 
	{
		if(numArgs ==1)//if there is more than just "cd"
		{
			address = command;//have address equal the directory entered by the user
		}	
		numArgs++;
		command = strtok(NULL, " \n");//going to the next argument
	}	

	//if the input is just cd, then we get the user name and combine it with /home/
	if(numArgs == 1)
	{

		if((usrAccount = getpwuid(uid = getuid()))== NULL)//if there is a error in retrieving the user in the database
		{
			perror("getpwuid() error");
			return;
		}

		if((chdir(usrAccount->pw_dir))!=0)//if there is a error in going to the users login directory (usually the home directory)
		{
			perror("usrAccount->pw_dir error");
			return;
		}
	}


	//case that more than "cd" was typed 
	else
	{

		if((chdir(address))!=0)//if the directory does not exist 
		{
			perror("cant change directory\n");		
		}

	}
}

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
