#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include "copy.h"

void parentProcess(int writeFd, int readFd,
                   char *files[], int fileCount)
{
    char buffer[BUF_SIZE];

    for (int i = 0; i < fileCount; i++)
    {
        char ready;
        read(readFd, &ready, 1);

        int fd = open(files[i], O_RDONLY);

        /*
         * fileSize == -1 означает:
         * файл пропускается.
         */
        if (fd < 0)
        {
            fprintf(stderr,
                    "Файл %s не существует\n",
                    files[i]);

            FileInfo info = {0};
            info.fileSize = -1;

            write(writeFd, &info, sizeof(info));

            continue;
        }

        struct stat st;

        if (fstat(fd, &st) == -1)
        {
            perror("fstat");
            close(fd);

            FileInfo info = {0};
            info.fileSize = -1;

            write(writeFd, &info, sizeof(info));

            continue;
        }

        if (strlen(files[i]) >= NAME_SIZE)
        {
            fprintf(stderr,
                    "Слишком длинное имя файла: %s\n",
                    files[i]);

            close(fd);

            FileInfo info = {0};
            info.fileSize = -1;

            write(writeFd, &info, sizeof(info));

            continue;
        }

        FileInfo info = {0};

        strcpy(info.fileName, files[i]);
        info.fileSize = st.st_size;

        /*
         * Первое сообщение содержит
         * имя файла и его размер.
         */
        write(writeFd, &info, sizeof(info));

        /*
         * После заголовка отправляется
         * содержимое файла блоками.
         */
        ssize_t n;

        while ((n = read(fd, buffer, BUF_SIZE)) > 0)
        {
            write(writeFd, buffer, n);
        }

        close(fd);
    }

    /*
     * Ребёнок готов принять ещё одно сообщение,
     * но файлов больше нет.
     */
    char ready;
    read(readFd, &ready, 1);

    FileInfo info = {0};

    /*
     * fileSize == -2 означает завершение.
     */
    info.fileSize = -2;

    write(writeFd, &info, sizeof(info));
}


void childProcess(int readFd, int writeFd)
{
    char buffer[BUF_SIZE];

    while (1)
    {
        /*
         * Сообщаем родителю о готовности.
         */
        char ready = 'R';

        write(writeFd, &ready, 1);

        FileInfo info;

        read(readFd, &info, sizeof(info));

        /*
         * Родитель завершает работу.
         */
        if (info.fileSize == -2)
            break;

        /*
         * Файл пропущен.
         */
        if (info.fileSize == -1)
            continue;

        char copyName[NAME_SIZE + 6];

        snprintf(copyName,
                 sizeof(copyName),
                 "%s.copy",
                 info.fileName);

        int out = open(copyName,
                       O_WRONLY | O_CREAT | O_TRUNC,
                       0644);

        if (out < 0)
        {
            perror("open");
            return;
        }

        off_t received = 0;

        while (received < info.fileSize)
        {
            size_t need = BUF_SIZE;

            if (info.fileSize - received < BUF_SIZE)
                need = info.fileSize - received;

            ssize_t n = read(readFd, buffer, need);

            if (n <= 0)
                break;

            write(out, buffer, n);

            received += n;
        }

        close(out);
    }
}