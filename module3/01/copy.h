#ifndef COPY_H
#define COPY_H

#include <sys/types.h>

#define BUF_SIZE 1024
#define NAME_SIZE 256

typedef struct
{
    char fileName[NAME_SIZE];
    off_t fileSize;
} FileInfo;

void parentProcess(int writeFd, int readFd,
                   char *files[], int fileCount);

void childProcess(int readFd, int writeFd);

#endif