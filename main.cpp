#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define COMMAND_MAX_CHARS 512

void read_command(char *com, char **par)
{
    fprintf(stdout, "$ ");
    
    //read user command
    char input [COMMAND_MAX_CHARS];
    fgets(input, COMMAND_MAX_CHARS, stdin);

    //remove newline
    input[strcspn(input, "\n")] = 0;


    //first token is the command
    char * tok;
    if((tok = strtok(input, " ")) == NULL)
    {
      return;
    }
    strcpy(com, tok);
    strcpy(par[0], tok);

    //all following tokens are parameters for the command
    int i = 1;
    for(; (tok = strtok(NULL, " ")) != NULL; i++)
    {
      strcpy(par[i], tok);
    }
    par[i] = NULL;

    return;
}

void log(char* filename, char *com, char **parameters)
{
    FILE * f;
    if((f = fopen(filename, "w")) == NULL)
    {
        fprintf(stderr, "error opening log file\n");
        exit(2);
    }

    time_t t = time(NULL);
    time(&t);
    //log command
    fprintf(f, "[%s]", ctime(&t));

    for(int i = 0; i < sizeof(parameters); i++)
    {
        fprintf(f," %s", parameters[i]);
    }
    fprintf(f, "\n");
}

int main()
{
    int childPid;
    int status;
    char command[20];
    char * parameters[60];
    //int logFd;
    //char * logFileName = "input.log";

    //Create a log file for all user input
    /*if((logFd = creat(logFileName, 0777)) < 0)
    {
      fprintf(stderr, "log create error\n");
      exit(2);
    }*/

    while(1)
    {
        read_command(command, parameters);

        //log(logFileName, command, parameters);

        //flush stream before forking to avoid stream errors
        fflush(stdin);
        fflush(stdout);

        if((childPid = fork()) == -1)
        {
            fprintf(stderr, "can't fork\n");
            exit(1);
        }
        else if(childPid == 0) /* child */
        {
            execvp(command, parameters);
            exit(0);
        }
        else /* parent process */
        {
            waitpid(childPid, &status, WUNTRACED | WCONTINUED);
        }
    }
}
