#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#include "io.h"
#include "faceCommonLib.h"

char *log_file;

void FilePrintMessage(const char* expr...)
{
    UNREFERENCED_PARAMETER(file);
    char message[LOG_MESSAGE_MAX_LENGTH];
    va_list args;

    if (expr)
    {
        va_start(args, expr);

        vsprintf(message, expr, args);

        pthread_mutex_lock(&filePrint_cs);

        printf("%s", message);
        pthread_mutex_unlock(&filePrint_cs);

        if (log_file)
        {
            pthread_mutex_lock(&filePrint_cs);

            FILE* fl = NULL;

            if ((fl = fopen(log_file, "a+")) != NULL)
            {
                if (fl)
                {
                    fprintf(fl, "%s", message);
                }

                fclose(fl);
            }
            else
            {
                printf("Failed to open file %s, error = %u", log_file, errno);
            }

            pthread_mutex_unlock(&filePrint_cs);
        }
    }
}

// don't forget '/' in the end of dir name. Example: good = "/home/workspace/" bad: "/home/workspace"
int getFilesFromDir(const char *dir, std::vector<std::string> &files)
{
    if(dir == NULL)
    {
        FilePrintMessage(_FAIL("Invalid parameter dir == NULL in getFilesFromDir\n"));
        return -1;
    }

    DIR            *dirStream;
    struct dirent  *file;
    struct stat     fileInfo;
    if(NULL == (dirStream = opendir(dir)))
    {
        FilePrintMessage(_FAIL("failed to load dir stream on error %s\n"), strerror(errno));
        return -1;
    }

    while (NULL != (file = readdir(dirStream)))
    {
        char *fullname = new char[strlen(dir) + strlen(file->d_name) + 1];
        memset(fullname, 0, strlen(dir) + strlen(file->d_name) + 1);
        memcpy(fullname, dir, strlen(dir));
        memcpy(fullname + strlen(dir), file->d_name, strlen(file->d_name));
        if(-1 == stat(fullname, &fileInfo))
        {
            FilePrintMessage(_WARN("failed to get %s stats on error %s\n"), file->d_name, strerror(errno));
            continue;
        }

        // Save only regular files
        if(S_ISREG(fileInfo.st_mode))
        {
            files.push_back(std::string(file->d_name));
        }

    }
    if(-1 == closedir(dirStream))
    {
        FilePrintMessage(_FAIL("failed to close dir stream on error %s\n"), strerror(errno));
        return -1;
    }
    return 0;
}

int getSubdirsFromDir(const char *dir, std::vector<std::string> &subdirs)
{
    if(dir == NULL)
    {
        FilePrintMessage(_FAIL("Invalid parameter dir == NULL in getFilesFromDir\n"));
        return -1;
    }

    DIR            *dirStream;
    struct dirent  *file;
    struct stat     fileInfo;
    if(NULL == (dirStream = opendir(dir)))
    {
        FilePrintMessage(_FAIL("failed to load dir stream on error %s\n"), strerror(errno));
        return -1;
    }

    while (NULL != (file = readdir(dirStream)))
    {
        if(!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
        {
            continue;
        }
        char *fullname = new char[strlen(dir) + strlen(file->d_name) + 1];
        memset(fullname, 0, strlen(dir) + strlen(file->d_name) + 1);
        memcpy(fullname, dir, strlen(dir));
        memcpy(fullname + strlen(dir), file->d_name, strlen(file->d_name));
        if(-1 == stat(fullname, &fileInfo))
        {
            FilePrintMessage(_WARN("failed to get %s stats on error %s\n"), file->d_name, strerror(errno));
            continue;
        }

        // Save only regular files
        if(S_ISDIR(fileInfo.st_mode))
        {
            subdirs.push_back(std::string(file->d_name));
        }

    }
    if(-1 == closedir(dirStream))
    {
        FilePrintMessage(_FAIL("failed to close dir stream on error %s\n"), strerror(errno));
        return -1;
    }
    return 0;
}
