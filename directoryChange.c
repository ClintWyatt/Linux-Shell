#include "directoryChange.h"
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

