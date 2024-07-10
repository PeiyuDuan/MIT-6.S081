#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char* argv[])
{
    // format: sleep int
    if (argc != 2)
    {
        fprintf(2, "Usage: sleep[number]\n");
        exit(1);
    }
    sleep(atoi(argv[1]));
    exit(0);
}