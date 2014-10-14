#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "FrokAPI.h"
#include "frokLibCommon.h"
#include "../FrokJsonlib/json.h"
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#define MODULE_NAME     "GRUB"

const char GRUBBER_PATH[] = "/home/zda/grubber/";
const char RESULT_FILE_PATH[] ="/home/zda/grubber/result.txt";
const char SAVE_FILE_PATH[] ="/home/zda/grubber/load.txt";
#define MAX_USERS_NUM       10000
#define THREADS_NUM         8
#define SAVE_EREVRY_N_TURN  200

#define MAX_USERNAME_LEN    20

pthread_mutex_t results_lock = PTHREAD_MUTEX_INITIALIZER;

struct results
{

    unsigned main_alb_faces_num[1024];
    unsigned marks_faces_num[1024];
    unsigned total_faces_num[1024];
    char users[MAX_USERS_NUM][MAX_USERNAME_LEN];
    unsigned lastUserPos;
};

struct thread_params
{
    int i;
    struct results *res;
    char *userName;
};
struct thread_semas
{
    sem_t semaInit;
    sem_t semaStarted;
    sem_t semaFinished;
};

BOOL getJsonFromFile(const char *filePath, json::Object &json);
BOOL writeJsonToFile(const char *filePath, json::Object &json);
void *getUserStats(void *params);

BOOL save(const char *filename, struct results *res)
{
    ssize_t ret, nr;
    size_t len;
    char *buf;
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if(fd == -1)
    {
        TRACE_F("Failed to open file %s on error %s", filePath, strerror(errno));
        return FALSE;
    }

    len = sizeof(struct results);
    buf = (char*)res;
    while(len != 0 && (ret = write(fd, buf, len)) != 0)
    {
        if(ret == -1)
        {
            if(errno == EINTR)
                continue;
            TRACE("write failed on error %s", strerror(errno));
            return FALSE;
        }
        len -= ret;
        buf += ret;
    }

    close(fd);
    return TRUE;
}
BOOL load(const char *filename, struct results *res)
{
    int fd = open(filename, O_RDONLY);
    struct stat sb;
    if(fd == -1)
    {
        TRACE_F("Failed to open file %s on error %s", filePath, strerror(errno));
        return FALSE;
    }

    if(-1 == fstat(fd, &sb))
    {
        TRACE_F("fstat failed on error %s", strerror(errno));
        close(fd);
        return FALSE;
    }

    if(!S_ISREG(sb.st_mode))
    {
        TRACE_F("%s is not a file", filePath);
        close(fd);
        return FALSE;
    }

    char *membuffer = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if(membuffer == MAP_FAILED)
    {
        TRACE_F("mmap failed on error %s", strerror(errno));
        close(fd);
        return FALSE;
    }

    if(sb.st_size < sizeof(struct results))
    {
        TRACE("Invalid load file size\n");
        return FALSE;
    }

    memcpy(res, membuffer, sb.st_size);

    munmap(membuffer, sb.st_size);
    close(fd);
    return TRUE;
}

struct thread_params params;

int main(void)
{
    if(FROK_RESULT_SUCCESS != frokLibCommonInit(FROK_LIB_COMMON_DEFAULT_CONFIG_FILENAME))
    {
        TRACE("frokLibCommonInit\n");
        return -1;
    }

    char **users = NULL;
    unsigned usersNum = 0;
    unsigned userOffset = 0;
    struct results *res = (struct results*)calloc(sizeof(struct results), 1);

    load(SAVE_FILE_PATH, res);
    userOffset = res->lastUserPos;

    FILE *resfs = fopen(RESULT_FILE_PATH, "w");
    if(!resfs)
    {
        TRACE("Failed to open file %s for writing\n", RESULT_FILE_PATH);
        return -1;
    }

    if(TRUE != getSubdirsFromDir(GRUBBER_PATH, &users, &usersNum))
    {
        TRACE("Failed to get users\n");
        return -1;
    }

    if(usersNum > MAX_USERS_NUM)
    {
        TRACE("Too much users\n");
        return -1;
    }

    for(int j = 0; j < usersNum; j++)
    {
        for(int k = 0; k < MAX_USERS_NUM; k++)
        {
            if(0 == strcmp(users[j], res->users[k]))
            {
                free(users[j]);
                users[j] = NULL;
                break;
            }
        }
    }

    pthread_t *pthreads = (pthread_t*)malloc(THREADS_NUM * sizeof(pthread_t));
    struct thread_semas *semas = (struct thread_semas*)malloc(THREADS_NUM * sizeof(struct thread_semas));
    memset(pthreads, 0, THREADS_NUM * sizeof(pthread_t));
    // Start the threads
    for(int j = 0; j < THREADS_NUM; j++)
    {
        if (sem_init(&semas[j].semaInit, 0, 0) == -1)
        {
            TRACE("sem_init failed on error %s\n", strerror(errno));
            return -1;
        }
        if (sem_init(&semas[j].semaFinished, 0, 0) == -1)
        {
            TRACE("sem_init failed on error %s\n", strerror(errno));
            return -1;
        }
        if (sem_init(&semas[j].semaStarted, 0, 0) == -1)
        {
            TRACE("sem_init failed on error %s\n", strerror(errno));
            return -1;
        }
        if(sem_post(&semas[j].semaFinished) == -1)
        {
            TRACE("sem_post failed on error %s\n", strerror(errno));
            return -1;
        }
        if(0 != pthread_create(&pthreads[j], NULL, getUserStats, (void*)&semas[j]))
        {
            TRACE("pthread_create failed\n");
            return -1;
        }
    }

    int i = 0;
    while(i != usersNum)
    {
        if(users[i] == NULL)
        {
            i++;
            continue;
        }
        if(i%SAVE_EREVRY_N_TURN == 0)
        {
            TRACE("SAVE_PROCESS_STARTED\n");
            signal(SIGINT, SIG_IGN);
            for(int j = 0; j < THREADS_NUM; j++)
            {
                sem_wait(&semas[j].semaFinished);
            }
            res->lastUserPos = userOffset + i + 1;
            save(SAVE_FILE_PATH, res);
            for(int j = 0; j < THREADS_NUM; j++)
            {
                sem_post(&semas[j].semaFinished);
            }
            signal(SIGINT, SIG_DFL);
            TRACE("SAVE_PROCESS_FINISHED\n");
        }
        //int n = i%THREADS_NUM;
        int n = 0;
        while(1)
        {
            if(n>=THREADS_NUM) n = 0;
            if(sem_trywait(&semas[n].semaFinished) == 0)
            {
                break;
            }
            n++;
        }
        TRACE_TIMESTAMP("[%02d%%] In progress...\n", 100 * i / usersNum);
        params.i = userOffset + i + 1;
        params.res = res;
        params.userName = users[i];
        while(-1 == sem_post(&semas[n].semaInit))
        {
            if(errno != EAGAIN && errno != EINTR)
            {
                TRACE("sem_post failed on error %s\n", strerror(errno));
                return -1;
            }
        }

        while(-1 == sem_wait(&semas[n].semaStarted))
        {
            if(errno != EAGAIN && errno != EINTR)
            {
                TRACE("sem_wait failed on error %s\n", strerror(errno));
                return -1;
            }
        }

        i++;
    }

    for(i = 0; i < THREADS_NUM; i++)
    {
        sem_wait(&semas[i].semaFinished);
        pthread_cancel(pthreads[i]);
        pthread_join(pthreads[i], NULL);
    }

    TRACE_TIMESTAMP("[100%%] Done.\n");

    TRACE("SAVE_PROCESS_STARTED\n");
    signal(SIGINT, SIG_IGN);
    save(SAVE_FILE_PATH, res);
    signal(SIGINT, SIG_DFL);
    TRACE("SAVE_PROCESS_FINISHED\n");

    for(i = 0; i < usersNum; i++)   free(users[i]);
    free(semas);
    free(pthreads);
    free(users);

    // Print results
    unsigned numOfPeopleWith1ValidFaces = 0;
    unsigned numOfPeopleWith5ValidFaces = 0;
    unsigned numOfPeopleWith10ValidFaces = 0;
    unsigned numOfPeopleWith20ValidFaces = 0;

    for(int i = 0; i < 1024; i++)
    {
        if(i >= 1)  numOfPeopleWith1ValidFaces += res->total_faces_num[i];
        if(i >= 5)  numOfPeopleWith5ValidFaces += res->total_faces_num[i];
        if(i >= 10)  numOfPeopleWith10ValidFaces += res->total_faces_num[i];
        if(i >= 20)  numOfPeopleWith20ValidFaces += res->total_faces_num[i];
    }

    unsigned numOfPeopleWith1ValidFacesFromMainAlbum = 0;
    unsigned numOfPeopleWith5ValidFacesFromMainAlbum = 0;
    unsigned numOfPeopleWith10ValidFacesFromMainAlbum = 0;
    unsigned numOfPeopleWith20ValidFacesFromMainAlbum = 0;

    for(int i = 0; i < 1024; i++)
    {
        if(i >= 1)  numOfPeopleWith1ValidFacesFromMainAlbum += res->main_alb_faces_num[i];
        if(i >= 5)  numOfPeopleWith5ValidFacesFromMainAlbum += res->main_alb_faces_num[i];
        if(i >= 10)  numOfPeopleWith10ValidFacesFromMainAlbum += res->main_alb_faces_num[i];
        if(i >= 20)  numOfPeopleWith20ValidFacesFromMainAlbum += res->main_alb_faces_num[i];
    }

    unsigned numOfPeopleWith1ValidFacesFromMarks = 0;
    unsigned numOfPeopleWith5ValidFacesFromMarks = 0;
    unsigned numOfPeopleWith10ValidFacesFromMarks = 0;
    unsigned numOfPeopleWith20ValidFacesFromMarks = 0;

    for(int i = 0; i < 1024; i++)
    {
        if(i >= 1)  numOfPeopleWith1ValidFacesFromMarks += res->marks_faces_num[i];
        if(i >= 5)  numOfPeopleWith5ValidFacesFromMarks += res->marks_faces_num[i];
        if(i >= 10)  numOfPeopleWith10ValidFacesFromMarks += res->marks_faces_num[i];
        if(i >= 20)  numOfPeopleWith20ValidFacesFromMarks += res->marks_faces_num[i];
    }

    TRACE("%02d%% - people with 20 valid faces from marks or higher\n", (100 * numOfPeopleWith20ValidFacesFromMarks / (usersNum)));
    TRACE("%02d%% - people with 10 valid faces from marks or higher\n", (100 * numOfPeopleWith10ValidFacesFromMarks / (usersNum)));
    TRACE("%02d%% - people with 5 valid faces from marks or higher\n", (100 * numOfPeopleWith5ValidFacesFromMarks / (usersNum)));
    TRACE("%02d%% - people with 1 valid faces from marks or higher\n", (100 * numOfPeopleWith1ValidFacesFromMarks / (usersNum)));

    TRACE("%02d%% - people with 20 valid faces from main album or higher\n", (100 * numOfPeopleWith20ValidFacesFromMainAlbum / (usersNum)));
    TRACE("%02d%% - people with 10 valid faces from main album or higher\n", (100 * numOfPeopleWith10ValidFacesFromMainAlbum / (usersNum)));
    TRACE("%02d%% - people with 5 valid faces from main album or higher\n", (100 * numOfPeopleWith5ValidFacesFromMainAlbum / (usersNum)));
    TRACE("%02d%% - people with 1 valid faces from main album or higher\n", (100 * numOfPeopleWith1ValidFacesFromMainAlbum / (usersNum)));

    TRACE("%02d%% - people with 20 valid faces total or higher\n", (100 * numOfPeopleWith20ValidFaces / (usersNum)));
    TRACE("%02d%% - people with 10 valid faces total or higher\n", (100 * numOfPeopleWith10ValidFaces / (usersNum)));
    TRACE("%02d%% - people with 5 valid faces total or higher\n", (100 * numOfPeopleWith5ValidFaces / (usersNum)));
    TRACE("%02d%% - people with 1 valid faces total or higher\n", (100 * numOfPeopleWith1ValidFaces / (usersNum)));

    fclose(resfs);
    frokLibCommonDeinit();
    return 0;
}

BOOL getJsonFromFile(const char *filePath, json::Object &json)
{
    int fd = open(filePath, O_RDONLY);
    struct stat sb;
    if(fd == -1)
    {
        TRACE_F("Failed to open file %s on error %s", filePath, strerror(errno));
        return FALSE;
    }

    if(-1 == fstat(fd, &sb))
    {
        TRACE_F("fstat failed on error %s", strerror(errno));
        close(fd);
        return FALSE;
    }

    if(!S_ISREG(sb.st_mode))
    {
        TRACE_F("%s is not a file", filePath);
        close(fd);
        return FALSE;
    }

    char *membuffer = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if(membuffer == MAP_FAILED)
    {
        TRACE_F("mmap failed on error %s", strerror(errno));
        close(fd);
        return FALSE;
    }
    char *buffer = (char*)calloc(sb.st_size + 1, 1);
    if(buffer == NULL)
    {
        TRACE_F("calloc failed on error %s", strerror(errno));
        munmap(membuffer, sb.st_size);
        close(fd);
        return FALSE;
    }
    memcpy(buffer, membuffer, sb.st_size);

    try {
        json = json::Deserialize(buffer);
    } catch(...) {
        TRACE_F("Failed to deserialize jsonFile");
        free(buffer);
        munmap(membuffer, sb.st_size);
        close(fd);
        return FALSE;
    }

    free(buffer);
    munmap(membuffer, sb.st_size);
    close(fd);
    return TRUE;
}

BOOL writeJsonToFile(const char *filePath, json::Object &json)
{
    size_t len;
    ssize_t ret;
    int fd;
    const char *buffer = NULL;
    std::string tmp;
    try {
        tmp = json::Serialize(json);
    } catch(...) {
        TRACE("Failed to serialize jsonFile\n");
        return FALSE;
    }

    buffer = tmp.c_str();

    if(buffer != NULL)
    {
        fd = open(filePath, O_WRONLY | O_TRUNC, O_CREAT, 0664);
        if(fd == -1)
        {
            TRACE_F("Failed to open file %s on error %s", filePath, strerror(errno));
            return FALSE;
        }
        len = strlen(buffer);

        while(len != 0 && (ret = write(fd, buffer, len)) != 0)
        {
            if(ret == -1)
            {
                if(errno == EINTR || errno == EAGAIN)
                {
                    continue;
                }
                TRACE("write failed on error %s\n", strerror(errno));
                return FALSE;
            }
            len -= ret;
            buffer += ret;
        }
        close(fd);
    }

    return TRUE;
}

void *getUserStats(void *sema)
{
    struct thread_semas *semas = (struct thread_semas*)sema;
    while(1)
    {
        while(-1 == sem_wait(&semas->semaInit))
        {
            if(errno != EAGAIN && errno != EINTR)
            {
                TRACE_F("OMG THREAD STOPPED");
                return NULL;
            }
        }
        struct thread_params params_copy;
        params_copy.i = params.i;
        params_copy.res = params.res;
        params_copy.userName = params.userName;

        while(-1 == sem_post(&semas->semaStarted))
        {
            if(errno != EAGAIN && errno != EINTR)
            {
                TRACE_F("OMG THREAD STOPPED");
                return NULL;
            }
        }
        try
        {
            do
            {
                struct results *res = params_copy.res;
                char *userName = params_copy.userName;
                int i = params_copy.i;
                char *userPath = NULL;
                char *jsonFilePath = NULL;
                json::Object json;

                json::Object result;    // Final result that would be printed in users' json file
                json::Array mainAlbumResult;
                json::Array markedPhotosResult;

                json::Array markedPhotos;
                json::Array friends;
                size_t numOfMarks = 0;
                size_t numOfFriends = 0;
                unsigned numOfPhotos = 0;
                char **photos = NULL;
                char *photoPath = NULL;

                FaceDetectorAbstract *detector = new FrokFaceDetector;

                userPath = (char*)calloc(strlen(GRUBBER_PATH) + strlen(userName) + strlen("/") + 100, 1);
                if(userPath == NULL)
                {
                    TRACE("calloc failed on error %s\n", strerror(errno));
                    goto getUserStats_finish;
                }

                strcpy(userPath, GRUBBER_PATH);
                strcat(userPath, userName);
                strcat(userPath, "/");

                jsonFilePath = (char*)calloc(strlen(userPath) + strlen(userName) + strlen(".json") + 100, 1);
                if(jsonFilePath == NULL)
                {
                    TRACE("calloc failed on error %s\n", strerror(errno));
                    goto getUserStats_finish;
                }

                strcpy(jsonFilePath, userPath);
                strcat(jsonFilePath, userName);
                strcat(jsonFilePath, ".json");

                if(FALSE == getJsonFromFile(jsonFilePath, json))
                {
                    TRACE("getJsonFromFile failed\n");
                    goto getUserStats_finish;
                }
//              Preparation finished
//              Get marked photos
                try {
                    markedPhotos = json[(std::string)"markedPhotos"].ToArray();
                    friends = json[(std::string)"friends"].ToArray();
                } catch(...) {
                    TRACE("Invalid json\n");
                    goto getUserStats_finish;
                }

//              Main album - find photos with one face

                if(FALSE == getFilesFromDir(userPath, &photos, &numOfPhotos))
                {
                    goto getUserStats_finish;
                }

                for(int cnt1 = 0; cnt1 < numOfPhotos; cnt1++)
                {
                    for(int cnt = 0; cnt < markedPhotos.size(); cnt++)
                    {
                        json::Object markedPhoto = markedPhotos[cnt];
                        if(0 == strcmp(photos[cnt1], markedPhoto["photoId"].ToString().c_str()))
                        {
                            free(photos[cnt1]);
                            photos[cnt1] = NULL;
                            break;
                        }
                    }
                }

                for(int cnt = 0; cnt < numOfPhotos; cnt++)
                {
                    if(photos[cnt] == NULL)
                    {
                        continue;
                    }

                    std::vector<cv::Rect> faces;
                    std::vector<cv::Mat> normFaces;
                    photoPath = (char*)calloc(strlen(userPath) + strlen(photos[cnt]) + 1, 1);
                    if(photoPath == NULL)
                    {
                        continue;
                    }
                    strcpy(photoPath, userPath);
                    strcat(photoPath, photos[cnt]);
                    if(FROK_RESULT_SUCCESS != detector->SetTargetImage(photoPath))
                    {
                        goto next_photo;
                    }

                    if(FROK_RESULT_SUCCESS != detector->GetFacesFromPhoto(faces))
                    {
                        goto next_photo;
                    }

                    if(FROK_RESULT_SUCCESS != detector->GetNormalizedFaceImages(faces, normFaces))
                    {
                        goto next_photo;
                    }

                    if(normFaces.size() > 0)
                    {
                        if(normFaces.size() == 1)
                        {
                            try {
                                json::Object obj;
                                cv::Rect faceRect = faces.at(0);
                                obj["photoId"] = photos[cnt];
                                obj["x"] = faceRect.x + (faceRect.width / 2);
                                obj["y"] = faceRect.y + (faceRect.height / 2);
                                mainAlbumResult.push_back(obj);
                            } catch(...) {}
                        }
                    }
            next_photo:
                    free(photoPath);
                }

//              Main album - finished

//              Marked photos - verify any face and find closest one.

                for(int cnt = 0; cnt < markedPhotos.size(); cnt++)
                {
                    try {
                        json::Object obj = markedPhotos[cnt];
                        if(obj["marksNum"].ToInt() >= 7)   // Fuck photos with 7+ marks
                        {
                            continue;
                        }

                        std::string photoName = obj["photoId"].ToString();
                        std::vector<cv::Rect> faces;
                        std::vector<cv::Mat> normFaces;

                        photoPath = (char*)calloc(strlen(userPath) + photoName.size() + strlen(".jpg") + 1, 1);
                        if(photoPath == NULL)
                        {
                            continue;
                        }
                        strcpy(photoPath, userPath);
                        strcat(photoPath, photoName.c_str());
                        strcat(photoPath, ".jpg");

                        if(FROK_RESULT_SUCCESS != detector->SetTargetImage(photoPath))
                        {
                            goto next_marked_photo;
                        }

                        if(FROK_RESULT_SUCCESS != detector->GetFacesFromPhoto(faces))
                        {
                            goto next_marked_photo;
                        }

                        if(FROK_RESULT_SUCCESS != detector->GetNormalizedFaceImages(faces, normFaces))
                        {
                            goto next_marked_photo;
                        }

                        if(normFaces.size() > 0)
                        {
                            for(int cnt1 = 0; cnt1 < normFaces.size(); cnt1++)
                            {
                                try {
                                    cv::Rect curRect = faces.at(cnt1);
                                    json::Object curUserMark;
                                    // Verify that im the closest dude on photo
                                    json::Array allMarksOnPhoto =  obj["marks"];
                                    double distanceToTheFace = DBL_MAX;
                                    int cnt2;
                                    bool userFound = false;
                                    for(cnt2 = 0; cnt2 < allMarksOnPhoto.size(); cnt2++)
                                    {

                                        json::Object curMark = allMarksOnPhoto[cnt2];
                                        if(0 == strcmp(curMark["uid"].ToString().c_str(), photoName.c_str()))
                                        {
                                            userFound = true;
                                            curUserMark = curMark;
                                            double tmp = pow(pow((curMark["x"].ToInt() - (curRect.x + curRect.width / 2)), 2) +
                                                    pow((curMark["y"].ToInt() - (curRect.y + curRect.height / 2)), 2), 0.5);
                                            if(tmp <= distanceToTheFace)
                                            {
                                                distanceToTheFace = tmp;
                                                continue;
                                            }
                                            else
                                            {
                                                break;
                                            }
                                        }

                                        if(userFound == false)
                                        {
                                            // Just save the closest user and hope that user is closier to the curRect
                                            double tmp = pow(pow((curMark["x"].ToInt() - (curRect.x + curRect.width / 2)), 2) +
                                                    pow((curMark["y"].ToInt() - (curRect.y + curRect.height / 2)), 2), 0.5);
                                            if(tmp < distanceToTheFace)
                                            {
                                                distanceToTheFace = tmp;
                                            }
                                            continue;
                                        }
                                        else
                                        {
                                            if(pow(pow((curMark["x"].ToInt() - (curRect.x + curRect.width / 2)), 2) +
                                                   pow((curMark["y"].ToInt() - (curRect.y + curRect.height / 2)), 2), 0.5) < distanceToTheFace)
                                            {       // oh nooooo, some fake dude on photo
                                                break;
                                            }
                                        }
                                    }
                                    if(cnt != allMarksOnPhoto.size())
                                    {
                                        continue;       // Ok we are absolutely sure that this is some other guy. So gay.
                                    }
                                    json::Object curRes;
                                    std::stringstream sstream;
                                    sstream << cnt1;
                                    curRes["photoId"] = photoName;
                                    curRes["faceNum"] = sstream.str();
                                    curRes["x"] = curUserMark["x"];
                                    curRes["y"] = curUserMark["y"];
                                    markedPhotosResult.push_back(curRes);
                                    break;
                                } catch(...) {}
                            }
                        }
                next_marked_photo:
                        free(photoPath);
                    } catch(...) {
                        int asdasd = 0;
                                asdasd++;
                    }
                }
//              Marked photos - finished

//              Create output json
                result["mainAlbum"] = mainAlbumResult;
                result["marks"] = markedPhotosResult;
                json["result"] = result;
//              Write json to file
                if(FALSE == writeJsonToFile(jsonFilePath, json))
                {
                    TRACE("writeJsonToFile failed\n");
                    goto getUserStats_finish;
                }
//              Done

//              Write results to global scope
                pthread_mutex_lock(&results_lock);
                strcpy(res->users[i], userName);
                if(mainAlbumResult.size() > 1023)
                    res->main_alb_faces_num[1023] ++;
                else
                    res->main_alb_faces_num[mainAlbumResult.size()]++;
                if(markedPhotosResult.size() > 1023)
                    res->marks_faces_num[1023] ++;
                else
                    res->marks_faces_num[markedPhotosResult.size()]++;
                if((mainAlbumResult.size() + markedPhotosResult.size()) > 1023)
                    res->total_faces_num[1023] ++;
                else
                    res->total_faces_num[mainAlbumResult.size() + markedPhotosResult.size()]++;

                pthread_mutex_unlock(&results_lock);
//              Done
            getUserStats_finish:
                for(int cnt = 0;cnt < numOfPhotos; cnt++)
                {
                    free(photos[cnt]);
                }
                free(photos);
                free(jsonFilePath);
                free(userPath);
                delete detector;
            }while(0);
        }
        catch(...)
        {
            TRACE("Ooops, not as bad as real ooops but something failed. Kepp going\n");
        }

        while(-1 == sem_post(&semas->semaFinished))
        {
            if(errno != EAGAIN && errno != EINTR)
            {
                TRACE_F("OMG THREAD STOPPED");
                return NULL;
            }
        }
    }
    return NULL;
}
