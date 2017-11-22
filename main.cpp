#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#define COMMAND_MAX_CHARS 512
#define PARAM_MAX_SIZE 60

int read_command(char *com, char **par)
{
    char dir [1024];
    fprintf(stdout, "%s > ", getcwd(dir, 1024));

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

    par[0] = (char *) malloc(sizeof(char) * PARAM_MAX_SIZE);
    strcpy(par[0], tok);

    //all following tokens are parameters for the command
    int i = 1;
    for(; (tok = strtok(NULL, " ")) != NULL; i++)
    {
        par[i] = (char *) malloc(sizeof(char) * PARAM_MAX_SIZE);
        strcpy(par[i], tok);
    }
    par[i] = (char *) malloc(1);
    par[i] = NULL;

    return i;
}

void handleQuit(int)
{
    fprintf(stdout, "\nDo you really want to quit (y/n)?");
    fflush(stdout);

    char buffer [8];
    fgets(buffer, 8, stdin);

    if(buffer[0] == 'Y' || buffer[0] == 'y')
    {
        exit(0);
    }
    fflush(stdin);

    return;
}

void handleSegv(int)
{
    fclose(stdin);
    fprintf(stdout, "Encountered segmentation fault.\n");
    exit(2);
}

void log(FILE * logFile, char *com, char **parameters)
{
    time_t t;
    struct tm * timeinfo;

    time(&t);
    timeinfo = localtime(&t);

    //log command
    char time [80];
    strftime(time, 80, "%a %e %b %Y %T", timeinfo);
    fprintf(logFile, "[%s]", time);

    for(int i = 0; parameters[i] != NULL; i++)
    {
        fprintf(logFile," %s", parameters[i]);
    }
    fprintf(logFile, "\n");

    fflush(logFile);
}

int main()
{
    int childPid;
    int status;
    char command[20];
    char * parameters[PARAM_MAX_SIZE];
    int logFd;
    char logFileName [256] = "input.log";

    signal(SIGINT, handleQuit);
    signal(SIGSEGV, handleSegv);

    //Create a log file for all user input
    if((logFd = creat(logFileName, 0777)) < 0)
    {
      fprintf(stderr, "log create error\n");
      exit(2);
    }

    //open log FILE
    FILE * logFile;
    if((logFile = fdopen(logFd, "a")) == NULL)
    {
        fprintf(stderr, "error opening log file\n");
        exit(2);
    }

    while(1)
    {
        int numParams = read_command(command, parameters);

        log(logFile, command, parameters);

        //flush stream before forking to avoid stream errors
        fflush(stdin);
        fflush(stdout);

        //check for cd
        if(strcmp(command, "cd") == 0)
        {
            if(chdir(parameters[1]) < 0)
            {
                int errsv = errno;
                fprintf(stderr, "can't change directory. error code: %d \n", errsv);
            }

            //don't fork on cd, no child process required
            continue;
        }

        if((childPid = fork()) == -1)
        {
            fprintf(stderr, "can't fork\n");
            exit(1);
        }
        else if(childPid == 0) //child
        {
            if(execvp(command, parameters) < 0)
            {
                //an error occured executing command
                int errsv = errno;
                fprintf(stderr, "error executing command. error code: %d\n", errsv);
            }
            exit(0);
        }
        else // parent process
        {
            if(parameters[numParameters-1] != "&")
            {
                waitpid(childPid, &status, WUNTRACED | WCONTINUED);
            }
        }
    }

    //close log file
    fclose(logFile);
}
