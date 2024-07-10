#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

const int MSGSIZE = 16;

int
main(int argc, char* argv[])
{
    sleep(10);
    // firstly, get the standard output of the fronter command
    // this is the standard input of this command
    char buf[MSGSIZE];
    read(0, buf, MSGSIZE);

    // secondly, get the arguments of this command
    char* xargv[MAXARG];
    int xargc = 0;
    for (int i = 1; i < argc; i++)
    {
        xargv[xargc++] = argv[i];
    }

    char* p = buf;
    for (int i = 0; i < MSGSIZE; i++)
    {
        if (buf[i] == '\n')
        {
            int pid = fork();
            if (pid == 0)
            {
                // lastly, execute the complete command
                buf[i] = 0;
                xargv[xargc++] = p;
                xargv[xargc++] = 0;

                exec(xargv[0], xargv);
                exit(0);
            }
            else
            {
                p = &buf[i + 1];
                wait(0);
            }
        }
    }
    wait(0);
    exit(0);
}