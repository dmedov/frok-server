#include <pthread.h>

#include "faceCommonLib.h"

pthread_mutex_t filePrint_cs;

void InitFaceDetectionLib()
{
    pthread_mutexattr_t mAttr;
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&filePrint_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);

    /*std::vector<std::string> users = std::vector<std::string>();
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
            string fileName = ((string)ID_PATH).append(users[i]).append("/eigenface.yml");
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
    }*/
}

void DeinitFaceDetectionLib()
{
    /*delete []cascades;
    for (int i = 0; i < MAX_THREADS_AND_CASCADES_NUM; i++)
    {
        cvReleaseHaarClassifierCascade(&cascades[i].face->oldCascade);
        cvReleaseHaarClassifierCascade(&cascades[i].eyes->oldCascade);
        cvReleaseHaarClassifierCascade(&cascades[i].nose->oldCascade);
        cvReleaseHaarClassifierCascade(&cascades[i].mouth->oldCascade);
        cvReleaseHaarClassifierCascade(&cascades[i].eye->oldCascade);
        cvReleaseHaarClassifierCascade(&cascades[i].righteye2->oldCascade);
        cvReleaseHaarClassifierCascade(&cascades[i].lefteye2->oldCascade);
    }*/

    pthread_mutex_destroy(&filePrint_cs);
}
