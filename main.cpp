#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define COMMAND_MAX_CHARS 512

void read_command(char *com, char **par)
{
    fprintf(stdout, "$ ");
printf("test");
    char input [COMMAND_MAX_CHARS];
    printf("test");
    fgets(input, COMMAND_MAX_CHARS, stdin);

    //remove newline
    input[strcspn(input, "\n")] = 0;

printf("test");

    //first token is the command
    strcpy(com, strtok(input, " "));



    //all following tokens are parameters for the command
    char * tok;
    int i = 0;
    for(; (tok = strtok(NULL, " ")) != NULL; i++)
    {
      strcpy(par[i], tok);
    }
    par[i] = 0;

    printf("read: %s %s", com, par[0]);

    return;
}

int main()
{
    int childPid;
    int status;
    char command[20];
    char * parameters[60];

    while(1)
    {
        read_command(command, parameters);

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
            printf("executing command: %s %s\n", command, parameters[0]);
            execvp(command, parameters);
            exit(0);
        }
        else /* parent process */
        {
            waitpid(childPid, &status, WUNTRACED | WCONTINUED);
        }
    }
}
