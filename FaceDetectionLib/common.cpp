#include "stdafx.h"
#include "LibInclude.h"

pthread_mutex_t faceDetectionCS;
pthread_mutex_t fileCS;

void InitFaceDetectionLib()
{
    pthread_mutexattr_t mAttr;
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&faceDetectionCS, &mAttr);
    pthread_mutex_init(&fileCS, &mAttr);
    pthread_mutexattr_destroy(&mAttr);
/*
    _finddata_t result;

    intptr_t firstHandle = _findfirst(((string)(ID_PATH)).append("*").c_str(), &result);

    UINT_PTR uNumOfThreads = 0;

    if (firstHandle != -1)
    {
        do
        {
            Ptr<FaceRecognizer> model = createFisherFaceRecognizer();
            try
            {
                string fileName = ((string)(ID_PATH)).append(result.name).append("//eigenface.yml");
                if (_access(fileName.c_str(), 0) != -1)
                {
                    model->load(fileName.c_str());
                    FilePrintMessage(NULL, _SUCC("Model base for user %s successfully loaded. Continue..."), result.name);
                }
                else
                {
                    FilePrintMessage(NULL, _WARN("Failed to load model base for user %s. Continue..."), result.name);
                    continue;
                }
                
            }
            catch (...)
            {
                FilePrintMessage(NULL, _WARN("Failed to load model base for user %s. Continue..."), result.name);
                continue;
            }
            models[(string)result.name] = model;
        } while (_findnext(firstHandle, &result) == 0);
    }
    else
    {
        FilePrintMessage(NULL, _WARN("No trained users found in %s folder"), ID_PATH);
    }*/
}

void DeinitFaceDetectionLib()
{
    for (int i = 0; i < MAX_THREADS_AND_CASCADES_NUM; i++)
    {
        cvReleaseHaarClassifierCascade(&cascades[i].face);
        cvReleaseHaarClassifierCascade(&cascades[i].eyes);
        cvReleaseHaarClassifierCascade(&cascades[i].nose);
        cvReleaseHaarClassifierCascade(&cascades[i].mouth);
        cvReleaseHaarClassifierCascade(&cascades[i].eye);
        cvReleaseHaarClassifierCascade(&cascades[i].righteye2);
        cvReleaseHaarClassifierCascade(&cascades[i].lefteye2);
    }
    
    pthread_mutex_destroy(&faceDetectionCS);
    pthread_mutex_destroy(&fileCS);
}

void FilePrintMessage(char* file, const char* expr...)
{
    UNREFERENCED_PARAMETER(file);
    char message[LOG_MESSAGE_MAX_LENGTH];
    va_list args;

    if (expr)
    {
        va_start(args, expr);

        vsprintf(message, expr, args);

        pthread_mutex_lock(&fileCS);

        printf("%s", message);
        pthread_mutex_unlock(&fileCS);

        if (file)
        {
            pthread_mutex_lock(&fileCS);

            FILE* fl = NULL;

            if ((fl = fopen(file, "a+")) != NULL)
            {
                if (fl)
                {
                    fprintf(fl, "%s", message);
                }

                fclose(fl);
            }
            else
            {
                printf("Failed to open file %s, error = %u", file, errno);
            }

            pthread_mutex_unlock(&fileCS);
        }
    }
}

// don't forget '/' in the end of dir name. Example: good = "/home/workspace/" bad: "/home/workspace"
int getFilesFromDir(const char *dir, vector<string> &files)
{
    if(dir == NULL)
    {
        FilePrintMessage(NULL, _FAIL("Invalid parameter dit == NULL in getFilesFromDir\n"));
        return -1;
    }

    DIR            *dirStream;
    struct dirent  *file;
    struct stat     fileInfo;
    if(NULL == (dirStream = opendir(dir)))
    {
        FilePrintMessage(NULL, _FAIL("failed to load dir stream on error %s\n"), strerror(errno));
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
            FilePrintMessage(NULL, _WARN("failed to get %s stats on error %s\n"), file->d_name, strerror(errno));
            continue;
        }

        // Save only regular files
        if(S_ISREG(fileInfo.st_mode))
        {
            files.push_back(string(file->d_name));
        }

    }
    if(-1 == closedir(dirStream))
    {
        FilePrintMessage(NULL, _FAIL("failed to close dir stream on error %s\n"), strerror(errno));
        return -1;
    }
    return 0;
}

