#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define COMMAND_MAX_CHARS 512
#define PARAM_MAX_SIZE 60

void read_command(char *com, char **par)
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

    return;
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

void log(char *filename, char *com, char **parameters)
{
    FILE * f;
    if((f = fopen(filename, "a")) == NULL)
    {
        fprintf(stderr, "error opening log file\n");
        exit(2);
    }

    time_t t;
    struct tm * timeinfo;
    
    time(&t);
    timeinfo = localtime(&t);
    
    //log command
    char time [80];
    strftime(time, 80, "%a %e %b %Y %T", timeinfo);
    fprintf(f, "[%s]", time);

    for(int i = 0; parameters[i] != NULL; i++)
    {
        fprintf(f," %s", parameters[i]);
    }
    fprintf(f, "\n");
    
    fflush(f);
    fclose(f);
}

int main()
{
    int childPid;
    int status;
    char command[20];
    char * parameters[PARAM_MAX_SIZE];
    int logFd;
    char logFileName [10] = "input.log";
    
    signal(SIGINT, handleQuit);
    signal(SIGSEGV, handleSegv);

    //Create a log file for all user input
    if((logFd = creat(logFileName, 0777)) < 0)
    {
      fprintf(stderr, "log create error\n");
      exit(2);
    }

    while(1)
    {
        read_command(command, parameters);

        log(logFileName, command, parameters);

        //flush stream before forking to avoid stream errors
        fflush(stdin);
        fflush(stdout);

        //check for cd
        if(strcmp(command, "cd") == 0)
        {
            //TODO
            continue;
        }
        
        if((childPid = fork()) == -1)
        {
            fprintf(stderr, "can't fork\n");
            exit(1);
        }
        else if(childPid == 0) //child
        {
            execvp(command, parameters);
            exit(0);
        }
        else // parent process
        {
            waitpid(childPid, &status, WUNTRACED | WCONTINUED);
        }
    }
}
