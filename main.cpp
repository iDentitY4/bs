#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int main(int argc, char** argv) {

    int childPid;
    if((childPid = fork()) == -1)
    {
        fprintf(stderr, "can't fork\n");
        _exit(1);
    } 
    else if (childPid == 0)
    {
        fprintf(stdout, "child: child pid = %d, parent pid = %d\n",
                getpid(), getppid());
        _exit(0);
    }
    else
    {
        fprintf(stdout, "parent: child pid = %d, parent pid = %d\n",
                childPid, getpid());
        _exit(0);
    }
}

