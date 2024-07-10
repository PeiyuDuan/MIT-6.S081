#include "kernel/types.h"
#include "user/user.h"

const int READ = 0;
const int WRITE = 1;

int
main(int argc, char* argv[])
{
    // format: pingpong
    if (argc != 1)
    {
        fprintf(2, "Usage: pingpong\n");
        exit(1);
    }

    int pid = fork();

    // byte to transform
    char byte = ' ';

    int child_to_parent[2];
    int parent_to_child[2];
    pipe(child_to_parent);
    pipe(parent_to_child);
    
    if (pid < 0)
    {
        // fork error
        fprintf(2, "Error\n");
        close(child_to_parent[READ]);
        close(child_to_parent[WRITE]);
        close(parent_to_child[READ]);
        close(parent_to_child[WRITE]);
    }
    else if (pid == 0)
    {
        // child process

        // close unessential pipe first
        close(parent_to_child[WRITE]);
        close(child_to_parent[READ]);

        // close after using to avoid deadlock
        read(parent_to_child[READ], &byte, sizeof(byte));
        close(parent_to_child[READ]);

        printf("%d: received ping\n", getpid());

        write(child_to_parent[WRITE], &byte, sizeof(byte));
        close(child_to_parent[WRITE]);

        exit(0);
    }
    else
    {
        // parent process
        close(parent_to_child[READ]);
        close(child_to_parent[WRITE]);

        write(parent_to_child[WRITE], &byte, sizeof(byte));
        close(parent_to_child[WRITE]);
        
        // run after child is over
        wait(0);

        read(child_to_parent[READ], &byte, sizeof(byte));
        close(child_to_parent[READ]);

        printf("%d: received pong\n", getpid());

        exit(0);
    }

    exit(1);
}