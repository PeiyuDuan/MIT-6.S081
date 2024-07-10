#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void
find(char* path, char* file_name)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    fd = open(path, 0);

    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
    {
        fprintf(2, "ls: path too long\n");
        close(fd);
        return;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/'; // p point to the position after the last /
    while (read(fd, &de, sizeof de) == sizeof de) 
    {
        if (de.inum == 0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if (stat(buf, &st) < 0) 
        {
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }
        if (st.type == T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..") != 0) 
        {
            find(buf, file_name);
        } 
        else if (strcmp(file_name, p) == 0)
        {
            printf("%s\n", buf);
        }
    }

  close(fd);
}

int
main(int argc, char* argv[])
{
    char* path = argv[1];
    char* file_name = argv[2];

    int fd;
    struct stat st;

    if (argc != 3)
    {
        fprintf(2, "Usage: find [path] [file_name]\n");
        exit(1);
    }

    // check if path is valid
    if((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        exit(1);
    }

    if(fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        exit(1);
    }

    if (st.type != T_DIR)
    {
        fprintf(2, "find: %s is not a directory\n", path);
        close(fd);
        exit(1);
    }

    find(path, file_name);
    exit(0);
}