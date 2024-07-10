#include "kernel/types.h"
#include "user/user.h"

const int RD = 0;
const int WR = 1;

void
child(int* pipe_left)
{
    close(pipe_left[WR]);

    int prime;
    if (read(pipe_left[RD], &prime, sizeof(int)) == sizeof(int))
    {
        // the first num must be a prime
        printf("prime %d\n", prime);

        int num;
        int pipe_right[2];
        pipe(pipe_right);

        if (fork() == 0)
        {
            child(pipe_right);
        }
        else
        {
            // close unnecessary resources first
            close(pipe_right[RD]);

            while (read(pipe_left[RD], &num, sizeof(int)) == sizeof(int))
            {
                if (num % prime != 0)
                {
                    // num is a prime, use pipe to inform its child
                    write(pipe_right[WR], &num, sizeof(int));
                }
            }
            close(pipe_left[RD]);
            close(pipe_right[WR]);

            wait(0);
        }
    }
    else
    {
        close(pipe_left[RD]);
    }

    exit(0);
}

int
main(int argc, char* argv[])
{
    if (argc != 1)
    {
        fprintf(2, "Usage: primes\n");
        exit(1);
    }

    int pipe_left[2];
    pipe(pipe_left);

    if (fork() == 0)
    {
        // child process
        child(pipe_left);
    }
    else
    {
        close(pipe_left[RD]);

        for (int i = 2; i < 35; i++)
        {
            write(pipe_left[WR], &i, sizeof(int));
        }
        close(pipe_left[WR]);

        wait(0);
    }

    exit(0);
}