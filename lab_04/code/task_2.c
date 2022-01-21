#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

#define FORK_ERROR -1
#define FORK_OK 0

#define COUNT_CHILDS 3

int main(void)
{
    pid_t childs_pid[COUNT_CHILDS];
    pid_t child_pid;
    
    printf("Parent - pid: %d, pgrp: %d\n", getpid(), getpgrp());
    
    for (int i = 0; i < COUNT_CHILDS; i++)
    {
        child_pid = fork();
        if (child_pid == FORK_ERROR)
        {
            perror("Can't fork.\n");
            return 1;
        }
        else if (child_pid == FORK_OK)
        {
            sleep(2);
            printf("Child - pid: %d, ppid: %d, pgrp: %d\n", getpid(), getppid(), getpgrp());
            return 0;
        }
        else{
            childs_pid[i] = child_pid;
        }
            
    }
    
    for (int i = 0; i < COUNT_CHILDS; i++)
    {
        int status;
        pid_t child_pid;
        child_pid = wait(&status);
    
        printf("Child has finished - pid: %d\n", child_pid);
    
        int stat_val;
        if (WIFEXITED(stat_val)){
	        printf("Child exited with code %d\n", WEXITSTATUS(stat_val));
        }
        else {
	        printf("Child terminated abnormally.\n");
        }
    }
    
    printf("Parent - child_1_pid: %d, child_2_pid: %d, child_3_pid: %d\n", childs_pid[0], childs_pid[1], childs_pid[2]);
    printf("Parent process is dead\n");
    
    return 0;
}
