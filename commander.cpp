#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 System Commander
 */

#define PIPE_READ 0
#define PIPE_WRITE 1

int main(int argc, char** argv) {

    int pipefd[2];
    char buf;
    
    pid_t cpid;
    
    if(pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    cpid = fork();
    if(cpid == -1) 
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if(cpid == 0) 
    { /* Process manager */
        
        dup2(pipefd[PIPE_READ], STDIN_FILENO);
        
    }
    else if(cpid > 0)
    {
        int status;
        pid_t w = waitpid(cpid, &status, WUNTRACED | WCONTINUED);
        if(w == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
    }
    
    return 0;
}

