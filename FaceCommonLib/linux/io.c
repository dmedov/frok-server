#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#include "io.h"
#include "faceCommonLib.h"

#define MODULE_NAME     "IO"

pthread_mutex_t trace_cs;

char *log_file;

// don't forget '/' in the end of dir name. Example: good = "/home/workspace/" bad: "/home/workspace"
int getFilesFromDir(const char *dir, char ***files, unsigned *filesNum)
{
    /*
    if(dir == NULL)
    {
        TRACE_F("Invalid parameter dir == NULL in getFilesFromDir\n");
        return -1;
    }

    DIR            *dirStream;
    struct dirent  *file;
    struct stat     fileInfo;
    if(NULL == (dirStream = opendir(dir)))
    {
        TRACE_F("failed to load dir stream on error %s\n", strerror(errno));
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
            TRACE_W("failed to get %s stats on error %s\n", file->d_name, strerror(errno));
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
        TRACE_F("failed to close dir stream on error %s\n", strerror(errno));
        return -1;
    }
    */
    return 0;
}

int getSubdirsFromDir(const char *dir, char ***files, unsigned *filesNum)
{
    /*
    if(dir == NULL)
    {
        TRACE_F("Invalid parameter dir == NULL in getFilesFromDir\n");
        return -1;
    }

    DIR            *dirStream;
    struct dirent  *file;
    struct stat     fileInfo;
    if(NULL == (dirStream = opendir(dir)))
    {
        TRACE_F("failed to load dir stream on error %s\n", strerror(errno));
        return -1;
    }

    while (std::vector<std::string> &filesNULL != (file = readdir(dirStream)))
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
            TRACE_W("failed to get %s stats on error %s\n", file->d_name, strerror(errno));
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
        TRACE_F("failed to close dir stream on error %s\n", strerror(errno));
        return -1;
    }
    */
    return 0;
}

void print_time(struct timespec startTime, struct timespec endTime)
{
    unsigned sec  = endTime.tv_sec - startTime.tv_sec;
    unsigned nsec = endTime.tv_nsec - startTime.tv_nsec;

    if (startTime.tv_nsec > endTime.tv_nsec)
    {
        sec--;
        nsec += 1e9;
    }

    pthread_mutex_lock(&trace_cs);
    fprintf(stdout, "Time elapsed: %u.%09u\n", sec, nsec);
    pthread_mutex_unlock(&trace_cs);
}

