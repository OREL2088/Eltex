#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "copy.h"

int main(int argc, char *argv[])
{
    int useFifo = 0;
    int firstFile = 1;

    int p2c[2];
    int c2p[2];

    char fifoP2C[256];
    char fifoC2P[256];

    if (argc < 2)
    {
        fprintf(stderr,
                "Usage: %s [-p pipe_name] file1 [file2 ...]\n",
                argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-p") == 0)
    {
        if (argc < 4)
        {
            fprintf(stderr,
                    "Usage: %s -p pipe_name file1 [file2 ...]\n",
                    argv[0]);
            return 1;
        }

        useFifo = 1;
        firstFile = 3;

        snprintf(fifoP2C, sizeof(fifoP2C),
                 "%s.p2c", argv[2]);

        snprintf(fifoC2P, sizeof(fifoC2P),
                 "%s.c2p", argv[2]);

        unlink(fifoP2C);
        unlink(fifoC2P);

        if (mkfifo(fifoP2C, 0666) == -1 ||
            mkfifo(fifoC2P, 0666) == -1)
        {
            perror("mkfifo");
            return 1;
        }
    }
    else
    {
        if (pipe(p2c) == -1 || pipe(c2p) == -1)
        {
            perror("pipe");
            return 1;
        }
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        int readFd;
        int writeFd;

        if (useFifo)
        {
            readFd = open(fifoP2C, O_RDONLY);
            writeFd = open(fifoC2P, O_WRONLY);
        }
        else
        {
            close(p2c[1]);
            close(c2p[0]);

            readFd = p2c[0];
            writeFd = c2p[1];
        }

        childProcess(readFd, writeFd);

        close(readFd);
        close(writeFd);

        return 0;
    }

    int writeFd;
    int readFd;

    if (useFifo)
    {
        writeFd = open(fifoP2C, O_WRONLY);
        readFd = open(fifoC2P, O_RDONLY);
    }
    else
    {
        close(p2c[0]);
        close(c2p[1]);

        writeFd = p2c[1];
        readFd = c2p[0];
    }

    parentProcess(
        writeFd,
        readFd,
        &argv[firstFile],
        argc - firstFile
    );

    close(writeFd);
    close(readFd);

    wait(NULL);

    if (useFifo)
    {
        unlink(fifoP2C);
        unlink(fifoC2P);
    }

    return 0;
}