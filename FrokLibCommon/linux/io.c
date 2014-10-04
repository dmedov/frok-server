#include "io.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#define MODULE_NAME     "IO"

// don't forget '/' in the end of dir name. Example: good = "/home/workspace/" bad: "/home/workspace"
BOOL getFilesFromDir(const char *dir, char ***files, unsigned *filesNum)
{
    int             i;
    DIR            *dirStream;
    struct dirent  *file;
    struct stat     fileInfo;
    char            *fullname;
    size_t          dirNameSize;
    size_t          fileNameSize;
    char          **foundFiles = 0;
    size_t          foundFilesNum = 0;
    char          **tmp_reallocPointer;
    if((dir == NULL) || (files == NULL) || (filesNum == NULL))
    {
        TRACE_F("Invalid parameters: dir = %p, files = %p, filesNum = %p\n", dir, files, filesNum);
        return FALSE;
    }

    if(NULL == (dirStream = opendir(dir)))
    {
        TRACE_F("failed to load dir %s stream on error %s\n", dir, strerror(errno));
        return FALSE;
    }

    dirNameSize = strlen(dir);

    while (NULL != (file = readdir(dirStream)))
    {
        fileNameSize = strlen(file->d_name);
        fullname = calloc(dirNameSize + fileNameSize + 1, 1);
        if(fullname == NULL)
        {
            TRACE_F("calloc failed on error %s", strerror(errno));
            for(i = 0; i < foundFilesNum; i++)
            {
                free(foundFiles[i]);
            }
            free(foundFiles);
            return FALSE;
        }

        memcpy(fullname, dir, dirNameSize);
        memcpy(fullname + dirNameSize, file->d_name, fileNameSize);

        if(-1 == stat(fullname, &fileInfo))
        {
            TRACE_W("failed to get %s stats on error %s\n", file->d_name, strerror(errno));
            free(fullname);
            continue;
        }

        // Save only regular files
        if(S_ISREG(fileInfo.st_mode))
        {
            foundFilesNum++;

            if(foundFiles != NULL)
            {
                tmp_reallocPointer = foundFiles;
                foundFiles = realloc(tmp_reallocPointer, sizeof(char*) * (foundFilesNum));
                if(!foundFiles)
                {
                    TRACE_F("realloc failed on error %s", strerror(errno));
                    free(fullname);
                    for(i = 0; i < foundFilesNum - 1; i++)
                    {
                        free(tmp_reallocPointer[i]);
                    }
                    free(tmp_reallocPointer);
                    return FALSE;
                }
            }
            else
            {
                foundFiles = malloc(sizeof(char*) * foundFilesNum);
                if(!foundFiles)
                {
                    TRACE_F("malloc failed on error %s", strerror(errno));
                    free(fullname);
                    return FALSE;
                }
            }

            foundFiles[foundFilesNum - 1] = calloc(fileNameSize + 1, 1);
            strcpy(foundFiles[foundFilesNum - 1], file->d_name);
        }
        free(fullname);

    }
    if(-1 == closedir(dirStream))
    {
        TRACE_F("failed to close dir stream on error %s\n", strerror(errno));

        for(i = 0; i < foundFilesNum; i++)
        {
            free(foundFiles[i]);
        }
        free(foundFiles);

        return FALSE;
    }

    *files = NULL;
    *filesNum = 0;
    if(foundFilesNum != 0)
    {
        *files = foundFiles;
        *filesNum = foundFilesNum;
    }
    return TRUE;
}

BOOL getSubdirsFromDir(const char *dir, char ***files, unsigned *filesNum)
{
    int             i;
    DIR            *dirStream;
    struct dirent  *file;
    struct stat     fileInfo;
    char            *fullname;
    size_t          dirNameSize;
    size_t          fileNameSize;
    char          **foundFiles = 0;
    size_t          foundFilesNum = 0;
    char          **tmp_reallocPointer;
    if((dir == NULL) || (files == NULL) || (filesNum == NULL))
    {
        TRACE_F("Invalid parameters: dir = %p, files = %p, filesNum = %p\n", dir, files, filesNum);
        return FALSE;
    }

    if(NULL == (dirStream = opendir(dir)))
    {
        TRACE_F("failed to load dir stream %s on error %s\n", dir, strerror(errno));
        return FALSE;
    }

    dirNameSize = strlen(dir);

    while (NULL != (file = readdir(dirStream)))
    {
        if((0 == strcmp(file->d_name, ".")) || (0 == strcmp(file->d_name, "..")))
        {
            continue;
        }
        fileNameSize = strlen(file->d_name);
        fullname = calloc(dirNameSize + fileNameSize + 1, 1);
        if(fullname == NULL)
        {
            TRACE_F("calloc failed on error %s", strerror(errno));
            for(i = 0; i < foundFilesNum; i++)
            {
                free(foundFiles[i]);
            }
            free(foundFiles);
            return FALSE;
        }

        memcpy(fullname, dir, dirNameSize);
        memcpy(fullname + dirNameSize, file->d_name, fileNameSize);

        if(-1 == stat(fullname, &fileInfo))
        {
            TRACE_W("failed to get %s stats on error %s\n", file->d_name, strerror(errno));
            free(fullname);
            continue;
        }

        // Save only regular files
        if(S_ISDIR(fileInfo.st_mode))
        {
            foundFilesNum++;

            if(foundFiles != NULL)
            {
                tmp_reallocPointer = foundFiles;
                foundFiles = realloc(tmp_reallocPointer, sizeof(char*) * (foundFilesNum));
                if(!foundFiles)
                {
                    TRACE_F("realloc failed on error %s", strerror(errno));
                    free(fullname);
                    for(i = 0; i < foundFilesNum - 1; i++)
                    {
                        free(tmp_reallocPointer[i]);
                    }
                    free(tmp_reallocPointer);
                    return FALSE;
                }
            }
            else
            {
                foundFiles = malloc(sizeof(char*) * foundFilesNum);
                if(foundFiles == NULL)
                {
                    TRACE_F("malloc failed on error %s", strerror(errno));
                    free(fullname);
                    return FALSE;
                }
            }

            foundFiles[foundFilesNum - 1] = calloc(fileNameSize + 1, 1);
            if(foundFiles[foundFilesNum - 1] == NULL)
            {
                TRACE_F("calloc failed on error %s", strerror(errno));
                free(fullname);
                for(i = 0; i < foundFilesNum - 1; i++)
                {
                    free(tmp_reallocPointer[i]);
                }
                free(tmp_reallocPointer);
                return FALSE;
            }

            strcpy(foundFiles[foundFilesNum - 1], file->d_name);
        }
        free(fullname);
    }
    if(-1 == closedir(dirStream))
    {
        TRACE_F("failed to close dir stream on error %s\n", strerror(errno));

        for(i = 0; i < foundFilesNum; i++)
        {
            free(foundFiles[i]);
        }
        free(foundFiles);

        return FALSE;
    }

    *files = NULL;
    *filesNum = 0;
    if(foundFilesNum != 0)
    {
        *files = foundFiles;
        *filesNum = foundFilesNum;
    }
    return TRUE;
}

BOOL do_mkdir(const char *path, mode_t mode)
{
    struct stat     st;
    BOOL            result = TRUE;

    if (stat(path, &st) != 0)
    {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            result = FALSE;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;
        result = FALSE;
    }

    return result;
}

BOOL mkpath(const char *path, mode_t mode)
{
    char           *pp;
    char           *sp;
    BOOL            result;
    char           *copypath;
    if(path == NULL)
    {
        TRACE_F("Invalid parameter: path = %p", path);
        return FALSE;
    }
    copypath = calloc(strlen(path) + 1, 1);
    if(copypath == NULL)
    {
        TRACE_F("calloc failed on error %s", strerror(errno));
        return FALSE;
    }
    strcpy(copypath, path);

    result = TRUE;
    pp = copypath;
    while (result == TRUE && (sp = strchr(pp, '/')) != 0)
    {
        if (sp != pp)
        {
            /* Neither root nor double slash in path */
            *sp = '\0';
            result = do_mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (result == TRUE)
        result = do_mkdir(path, mode);
    free(copypath);
    return result;
}
