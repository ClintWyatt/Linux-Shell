#include "redirection.h"
#include <string.h>

void writeRedirection(char *cmd, const short size)
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
                        allArgs[index] = command;       //add the argument to allArgs string array
                        index++;                        //going to the next index
                        command = strtok(NULL, " \n");
                }
                else
                {
                        if(strcmp(command, ">") ==0)
                        {
                                allArgs[index] = NULL; //setting the last index to NULL
                                collided++;
                                command = strtok(NULL, " \n");  //going to the next string, which is likely the file
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

void readRedirection(char *cmd, const short size)
{
        FILE *fp; //file to be opened
        char *allArgs[size]; //array of strings
        char *command; //string to help parse the input from the cmd input buffer
        int index =0; //used to access indexes for allArgs
        command = strtok(cmd, " ");//tokenizing characters from the cmd input stream

        while(command != NULL)
        {
                if(memcmp(command, "<", 1) != 0) //if command does not equal "<"
                {

                        allArgs[index] = command;//having the command equal a index in the string array
                        index++; //going to the next index
                        command = strtok(NULL, " \n"); // going to the next non space string
                }
                else
                {
                        allArgs[index] = NULL; //having the last index of the string array be null in order to work with execvp
                        command = strtok(NULL, " \n");//going to the next non character string, which will be the file
                        break;
                }

        }

        if((fp = fopen(command, "r")) == NULL)
        {
                printf("Error: %s is not a valid file name \n", command);
                exit(1);
        }
        dup2(fileno(fp), fileno(stdin)); //copying file descriptor for standard in
        fclose(fp); //closing the file
        execvp(allArgs[0], allArgs);
        //for the file, the file will use dup2(file, 0) for standard input
        // use r for < when opening the file. Use r+ for << when opening the file
}

