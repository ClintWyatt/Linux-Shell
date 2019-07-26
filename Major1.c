/*
Group 22
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
#include <signal.h>
#include <assert.h>


#define size 512
void testArgs();
void directoryChange(int x); //method changes the directory
void myhistory(); //Function to display a list of all commands the user has enterred.
void parentSignal(); //used to set the parent process to where any signal cant kill the shell, which means ignoring signals. 
void childSignal(); //used to set the signals to their default behavior. 
void pipeMethod(int numPipes);//method used for the piping
void pathname();
void readRedirection();
void writeRedirection();
void redirectionPipe();
void exitShell();

struct passwd *usrAccount; //struct passwd holds data about the user currently runnint the shell. The usrAccount struct holds information about the user from the operation system.
struct sigaction act;//used for the signaling
pid_t pid, pid1, pid2, pid3;//used for creating child processes
uid_t uid;
int pos;
int startIndex =0;
int endIndex =0;
int records = 0; //Variable to keep track of the number of commands the user enterred up to 20 commands.
int allRecs = 0; //Variable to keep track of the total number of user enterred commands.
int pipeNum = 0;
int readRedirect =0;
int writeRedirect = 0;
enum { READ, WRITE };
char recArr[20][512]; //Array to keep a list of all commands the user has enterred.
char *_pipe = "|";
char *arg; //used to check if any pipes or redirection symbols have appeared
char cmd[size];
char *cd = "cd";
char directory[size];//used to print current directory
char *wspace(char *word); //Function to clear leading whitespace from batch file commands
char *exitChar = "exit";

int main(int argc, char *argv[])
{
	

    	for( ; ; )
	{

		
	parentSignal(); //Having the current process ignore all signals (except kill -9) that can kill the shell
	
	    //getcwd copies the current directory to "directory"
	printf("%s ", getcwd(directory, sizeof(directory)));
	gets(cmd);	
	//printf("%s \n", cmd);

	    //Add command to history records
	if (records > 19)
	{
		records = 0;
		strcpy(recArr[records], cmd);
		records++;
		allRecs++;
	}
	else 
	{
		strcpy(recArr[records], cmd);
		records++;
		allRecs++;
	}
	
	//checking to see if the exit was typed by the user. If so, then the shell ends
	if(strcmp(cmd, "exit") ==0)
	{
		exitShell();
	} 
	startIndex =0;

	//checking for the first character. This is done for the directory change 
	for(startIndex; startIndex < strlen(cmd); startIndex++){

		if( cmd[startIndex] != ' ')
		{
			break;
		}

	}


	int pos = startIndex;
	//checking for a pipes and redirection in the cmd stream
	for(pos; pos < strlen(cmd); pos++)
	{
	

		if(cmd[pos] == '|')
		{

			pipeNum++;
		}
		else if(cmd[pos] == '>')
		{
			writeRedirect++;
		}
		else if(cmd[pos] == '<')
		{
			readRedirect++;
		}
	}
	
	endIndex = strlen(cmd); //used for changing the directory
	
        
	if((memcmp(cd, &cmd[startIndex], 2) ==0)){

	directoryChange(startIndex + 2);

	}    
	else if(strcmp("history", cmd) == 0)
	{
	
		myhistory();
	} 
	else if(pipeNum !=0)
	{
			childSignal();
			pipeMethod(pipeNum);
	}	
	//executing simple linux commands
    	else{	
		//Error checking the fork system call
		if ((pid = fork()) == -1)
		{
			perror("fork() error");
		}
		else{
			
	    		if(pid == 0)
			{
					
				childSignal(); //setting the child process to the foreground
				/*if(pipeNum != 0)
				{
					pipeMethod();
				}
				*/
				if(writeRedirect !=0) //a > or >> character appear
				{
					writeRedirection();
				}
				else if(readRedirect != 0)
				{
					readRedirection();
				}
				else
				{
					testArgs();
				}	
	    		}
	    		else
			{
			
	    	   		setpgid(pid, pid); 
				signal(SIGTTOU, SIG_IGN); //SIGTTOU is generated when a process in a background job attempts to write to the terminal
				tcsetpgrp(fileno(stdin), getpgid(pid)); 
				wait( (int *)0 );
	    		}
		}
    	}	
	
	//resetting variables
	readRedirect =0;
	writeRedirect =0;
	endIndex =0;
	pipeNum =0;	
	memset(cmd, '\0', size);    
    
    }
} // end of main

void testArgs()
{
	
	char *allArgs[size];//is essentailly a array of "strings"
	int index =0;//used to add strings to indexes of the allArgs array of strings
	char * command;//command will be added to the allArgs array until the end of the cmd buffer is processed
	command = strtok(cmd, " "); //tokenizing the cmd buffer for spaces
	while(command != NULL)
	{	
		
		//printf("%s \n", command);
		allArgs[index] = command;
		index++;
		command = strtok(NULL, " \n");			
	
	}
	allArgs[index] = NULL;//setting the last index to nul
	
    	execvp(allArgs[0], allArgs);
    	printf("Command not found \n");
    	exit(EXIT_FAILURE);

}

void pipeMethod(int numPipes)
{	
	
	char *args[size]; //used for copying the argument into all pipe arguments that will be executed on either side of the pipe 		
	char *pipeCommands[numPipes+1][size]; //2d pipe array that will hold the commands of each command on the right and left of the pipe symbol
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
		if(strcmp(command, _pipe) !=0) //the string is not a pipe
		{
			args[index] = command;		//adding the command to the array of strings
			index++;			//going to the next index
			command = strtok(NULL, " ");		// move to the next string that is not a space
	
		}
		else	//string is a pipe
		{
		pipeIndex[pos] = index;			//setting the current index "pos" to the 
		pos++;					//going to the next index
		command = strtok(NULL, " ");		// move to the next string that is not a space
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
			pipeCommands[pos][i] = args[f];//assign lefts index to args index
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
							;
							dup2(/*fd1[0]*/ allPipes[0][0], fileno(stdin));//overide stdin, second command, which is to the right of the pipe symbol

							if(pipeNum > 1) //if there is more than 1 pipe
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
			
}
			

void directoryChange(int x){

	int nextIndex =0;
	int num_Chars =0;
	int startInd =x;
	char *home = getenv("HOME");

	//check for the first character after cd
	//int c =x;
	for(x; x < strlen(cmd); x++){
		if(cmd[x] != ' '){
			startInd = x;
			break;
		}

	}
	//setting the start index to the first charcter after cd
	nextIndex =x;
	nextIndex++;
	//now checking for the number of characters
	for(x; x < strlen(cmd); x++){

		if(cmd[x] != ' '){
		num_Chars++;
		}
		else if(cmd[nextIndex] == ' '){
			//here, when the direstory changes, bash ignores everyting after the white space when directories change, which is what is done here with breaking from the loop
			break;
		}

	}

	//if the input is just cd, then we get the user name and combine it with /home/
	if(num_Chars ==0){

		
		if((usrAccount= getpwuid(uid = getuid()))==NULL){
		perror("getpwuid() error");
		return;
		}

		 
		if((chdir(usrAccount->pw_dir))!=0){
			perror("usrAccount->pw_dir error");
			return;
		}
		//may need to change this later
		memset(directory, '\0', sizeof(directory));
		//clearing cmd
		//getcwd(directory, sizeof(directory));
		
	}


	//case that more than "cd" was typed 
	else{

	memset(directory, 0, sizeof(directory));	//clearing the directory char buffer
	memcpy(directory, &cmd[startInd], num_Chars);	//copying user input into the directory buffer
	//chdir attempts to change the directory
		if((chdir(directory))!=0){
		perror("cant change directory");
		}
	
	}
}


void myhistory(){
	//print history of enterred commands
	int i = 0;
	if (allRecs > 19){
		while (i < 19){
			printf("Record %d is %s\n", i+1 , recArr[i]);
			i++;

		}
	}
	else{
		while (i < records){
                        printf("Record %d is %s\n", i+1 , recArr[i]);
                        i++;
                }
        }
}


void parentSignal(){
	/* After this method executes, all signals for the parent process that can be overwritten will be ignored */
	
	tcsetpgrp(fileno(stdin), getpgrp()); //makes the process with the process group id pgrp the foreground process group on the terminal.
	act.sa_handler = SIG_IGN; //specifies the action to be associated with signum. sig_ign means to ignore the signal.
	assert(sigaction(SIGHUP, &act, NULL) ==0);
	assert(sigaction(SIGINT, &act, NULL) ==0); //prevents ctrl c from terminating the sehll
	assert(sigaction(SIGQUIT, &act, NULL) ==0); //prevents quit form the keyboard from terminating the shell
	assert(sigaction(SIGILL, &act, NULL) ==0); //prevents an illegal instruction from killing the shell
	assert(sigaction(SIGABRT, &act, NULL) ==0); //prevents a abort signal from killing the shell
	assert(sigaction(SIGFPE, &act, NULL) ==0); //prevents a floating point exception from killing the shell
	assert(sigaction(SIGSEGV, &act, NULL) ==0); //prevents invalid memory reference from killing the shell
	assert(sigaction(SIGPIPE, &act, NULL) ==0); //prevents a broken pipe from killing the shell
	assert(sigaction(SIGALRM, &act, NULL) ==0);
	assert(sigaction(SIGTERM, &act, NULL) ==0);
	assert(sigaction(SIGUSR1, &act, NULL) ==0);
	assert(sigaction(SIGUSR2, &act, NULL) ==0);
	assert(sigaction(SIGTSTP, &act, NULL) ==0); //prevents ctrl z from terminating the shell.
	assert(sigaction(SIGTTIN, &act, NULL) ==0);
	assert(sigaction(SIGTTOU, &act, NULL) ==0);
	assert(sigaction(SIGBUS, &act, NULL) ==0); //prevents bad memory access from killing the shell
	assert(sigaction(SIGPOLL, &act, NULL) ==0);
	assert(sigaction(SIGPROF, &act, NULL) ==0);
	assert(sigaction(SIGSYS, &act, NULL) ==0);
	assert(sigaction(SIGTRAP, &act, NULL) ==0);
	assert(sigaction(SIGVTALRM, &act, NULL) ==0); //prevents virtual alarm clock from killing the shell
	assert(sigaction(SIGXCPU, &act, NULL) ==0); //prevents CPU time limit exceeded from killing the shell
	assert(sigaction(SIGXFSZ, &act, NULL) ==0); //prevents file size limit exceeded from killing the shell
	assert(sigaction(SIGIOT, &act, NULL) ==0);
	assert(sigaction(SIGSTKFLT, &act, NULL) ==0);
	assert(sigaction(SIGIO, &act, NULL) ==0);
	assert(sigaction(SIGPWR, &act, NULL) ==0);
	assert(sigaction(SIGWINCH, &act, NULL) ==0);
	assert(sigaction(SIGUNUSED, &act, NULL) ==0);
	

		
}

void childSignal(){


	setpgrp(); //sets process group of own process to itself
	tcsetpgrp(fileno(stdin), getpgid(pid));//having the child process in the foreground
	act.sa_handler = SIG_DFL; //default signal
	assert(sigaction(SIGHUP, &act, NULL) ==0);
	assert(sigaction(SIGINT, &act, NULL) ==0); //prevents ctrl c from terminating the sehll
	assert(sigaction(SIGQUIT, &act, NULL) ==0); //prevents quit form the keyboard from terminating the shell
	assert(sigaction(SIGILL, &act, NULL) ==0); //prevents an illegal instruction from killing the shell
	assert(sigaction(SIGABRT, &act, NULL) ==0); //prevents a abort signal from killing the shell
	assert(sigaction(SIGFPE, &act, NULL) ==0); //prevents a floating point exception from killing the shell
	assert(sigaction(SIGSEGV, &act, NULL) ==0); //prevents invalid memory reference from killing the shell
	assert(sigaction(SIGPIPE, &act, NULL) ==0); //prevents a broken pipe from killing the shell
	assert(sigaction(SIGALRM, &act, NULL) ==0);
	assert(sigaction(SIGTERM, &act, NULL) ==0);
	assert(sigaction(SIGUSR1, &act, NULL) ==0);
	assert(sigaction(SIGUSR2, &act, NULL) ==0);
	//assert(sigaction(SIGTSTP, &act, NULL) ==0); //prevents ctrl z from terminating the shell.
	assert(sigaction(SIGTTIN, &act, NULL) ==0);
	assert(sigaction(SIGTTOU, &act, NULL) ==0);
	assert(sigaction(SIGBUS, &act, NULL) ==0); //prevents bad memory access from killing the shell
	assert(sigaction(SIGPOLL, &act, NULL) ==0);
	assert(sigaction(SIGPROF, &act, NULL) ==0);
	assert(sigaction(SIGSYS, &act, NULL) ==0);
	assert(sigaction(SIGTRAP, &act, NULL) ==0);
	assert(sigaction(SIGVTALRM, &act, NULL) ==0); //prevents virtual alarm clock from killing the shell
	assert(sigaction(SIGXCPU, &act, NULL) ==0); //prevents CPU time limit exceeded from killing the shell
	assert(sigaction(SIGXFSZ, &act, NULL) ==0); //prevents file size limit exceeded from killing the shell
	assert(sigaction(SIGIOT, &act, NULL) ==0);
	assert(sigaction(SIGSTKFLT, &act, NULL) ==0);
	assert(sigaction(SIGIO, &act, NULL) ==0);
	assert(sigaction(SIGPWR, &act, NULL) ==0);
	assert(sigaction(SIGWINCH, &act, NULL) ==0);
	assert(sigaction(SIGUNUSED, &act, NULL) ==0);
	



}
// pathname
void pathname(){
        printf("path\n");
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

void writeRedirection()
{
	FILE *fp;  
	char *allArgs[size]; //string array
	char *command; //string to tokenize
	char *arg;
	int index =0;
	int collided =0; //used to know whether a > has been collided yet or not
	command = strtok(cmd, " "); //tokenizing all spaces
	while(command != NULL)
	{

			if(strcmp(command, ">") !=0 && strcmp(command, ">>")!=0) //the string is not a > and is not a
			{
				allArgs[index] = command;	//add the argument to allArgs string array
				index++;			//going to the next index
				command = strtok(NULL, " \n");	
			}
			else
			{
				if(strcmp(command, ">") ==0)
				{
					allArgs[index] = NULL; //setting the last index to NULL
					collided++;
					command = strtok(NULL, " \n");	//going to the next string, which is likely the file
					break;

				}
				else
				{
					allArgs[index] = NULL; //setting the last index to NULL
					collided+=2;
					command = strtok(NULL, " \n");
					break;
				}
			}
			
		
	}
	if(collided ==1)//if >
	{			
		if((fp = fopen(command, "w")) == NULL) //w means writing to the file and overides all previous data in the file
		perror("file error\n");
	}

	else // >>
	{
		if((fp = fopen(command, "a")) == NULL) //adds more to the file
		perror("file error\n");
	}
	dup2(fileno(fp), fileno(stdout)); //copying the file descriptor for standard out
	fclose(fp);
	execvp(allArgs[0], allArgs);
	
}

void readRedirection()
{
	FILE *fp; //file to be opened
	char *allArgs[size]; //array of strings
	char *command; //string to help parse the input from the cmd input buffer
	int index =0; //used to access indexes for allArgs
	command = strtok(cmd, " ");//tokenizing characters from the cmd input stream

	while(command != NULL)
	{
		if(strcmp(command, "<")) //if command does not equal "<"
		{
			allArgs[index] = command;//having the command equal a index in the string array
			index++; //going to the next index
			command = strtok(NULL, " "); // going to the next non space string
		}
		else
		{
			allArgs[index] = NULL; //having the last index of the string array be null in order to work with execvp
			command = strtok(NULL, " ");//going to the next non character string
			break;
		}
		
	}
	if((fp = fopen(command, "r")) == NULL)
	{
		perror("file name does not exist \n");
	}
	dup2(fileno(fp), fileno(stdin)); //copying file descriptor for standard in
	fclose(fp); //closing the file
	execvp(allArgs[0], allArgs);
	//for the file, the file will use dup2(file, 0) for standard input
	// use r for < when opening the file. Use r+ for << when opening the file
}

void redirectionPipe()
{



}

void exitShell()
{

	exit(0);

}
