#include "stdafx.h"
#include "LibInclude.h"

pthread_mutex_t faceDetectionCS;
pthread_mutex_t fileCS;

void InitFaceDetectionLib()
{
    cascades = new FaceCascades[MAX_THREADS_AND_CASCADES_NUM];
    for(unsigned i = 0; i < MAX_THREADS_AND_CASCADES_NUM; i++)
    {
        cascades[i].face->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml");
        cascades[i].eyes->load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_eye_tree_eyeglasses.xml");
        /*cascades[i].righteye.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_righteye.xml");
        cascades[i].lefteye.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_lefteye.xml");
        cascades[i].righteye2.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_righteye_2splits.xml");
        cascades[i].lefteye2.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_lefteye_2splits.xml");
        cascades[i].eye.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_eye.xml");
        cascades[i].nose.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_nose.xml");
        cascades[i].mouth.load("/opt/opencv-2.4.9/static/share/OpenCV/haarcascades/haarcascade_mcs_mouth.xml");*/
    }
    pthread_mutexattr_t mAttr;
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&faceDetectionCS, &mAttr);
    pthread_mutex_init(&fileCS, &mAttr);
    pthread_mutexattr_destroy(&mAttr);

    vector<string> users = vector<string>();
    getSubdirsFromDir(ID_PATH, users);

    if(users.empty())
    {
        FilePrintMessage(NULL, _WARN("No trained users found in %s folder"), ID_PATH);
        return;
    }

    for (unsigned int i = 0; i < users.size(); i++)
    {
        Ptr<FaceRecognizer> model = createFisherFaceRecognizer();
        try
        {
            string fileName = users[i].append("//eigenface.yml");
            if (access(fileName.c_str(), 0) != -1)
            {
                model->load(fileName.c_str());
                FilePrintMessage(NULL, _SUCC("Model base for user %s successfully loaded. Continue..."), users[i].c_str());
            }
            else
            {
                FilePrintMessage(NULL, _WARN("Failed to load model base for user %s. Continue..."), users[i].c_str());
                continue;
            }

        }
        catch (...)
        {
            FilePrintMessage(NULL, _WARN("Failed to load model base for user %s. Continue..."), users[i].c_str());
            continue;
        }
        models[users[i]] = model;
    }
}

void DeinitFaceDetectionLib()
{
    delete []cascades;
    /*for (int i = 0; i < MAX_THREADS_AND_CASCADES_NUM; i++)
    {
        cvReleaseHaarClassifierCascade(&cascades[i].face->oldCascade);
        cvReleaseHaarClassifierCascade(&cascades[i].eyes->oldCascade);
        cvReleaseHaarClassifierCascade(&cascades[i].nose->oldCascade);
        cvReleaseHaarClassifierCascade(&cascades[i].mouth->oldCascade);
        cvReleaseHaarClassifierCascade(&cascades[i].eye->oldCascade);
        cvReleaseHaarClassifierCascade(&cascades[i].righteye2->oldCascade);
        cvReleaseHaarClassifierCascade(&cascades[i].lefteye2->oldCascade);
    }*/
    
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

int getSubdirsFromDir(const char *dir, vector<string> &subdirs)
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
            FilePrintMessage(NULL, _WARN("failed to get %s stats on error %s\n"), file->d_name, strerror(errno));
            continue;
        }

        // Save only regular files
        if(S_ISDIR(fileInfo.st_mode))
        {
            subdirs.push_back(string(file->d_name));
        }

    }
    if(-1 == closedir(dirStream))
    {
        FilePrintMessage(NULL, _FAIL("failed to close dir stream on error %s\n"), strerror(errno));
        return -1;
    }
    return 0;
}

