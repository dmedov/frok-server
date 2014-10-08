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

    unsigned photos_with_faces[1024];
    unsigned photos_with_solo_faces[1024];
    unsigned marks_without_faces[1024];
    unsigned marks_with_faces[1024];
    unsigned friends[10240];            // Number of people with ### friends
    unsigned photos[10240];             // Number of people with ### photos
    unsigned user_photos_with_faces[MAX_USERS_NUM];
    unsigned user_photos_with_solo_faces[MAX_USERS_NUM];
    unsigned user_marks[MAX_USERS_NUM];
    unsigned user_marks_with_faces[MAX_USERS_NUM];
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
    unsigned numOfPeopleWith5Marks = 0;
    unsigned numOfPeopleWith10Marks = 0;
    unsigned numOfPeopleWith20Marks = 0;

    fprintf(resfs, "makrs_with_faces:\n");
    for(int i = 0; i < 5000; i++)
    {
        fprintf(resfs, "%i %i\n", res->user_marks[i], res->user_marks_with_faces[i]);
    }

    fprintf(resfs, "makrs without faces:\n");
    for(int i = 0; i < 1024; i++)
    {
        if(i >= 5) numOfPeopleWith5Marks += res->marks_without_faces[i];
        if(i >= 10) numOfPeopleWith10Marks += res->marks_without_faces[i];
        if(i >= 20) numOfPeopleWith20Marks += res->marks_without_faces[i];
        fprintf(resfs, "%i\n", res->marks_without_faces[i]);
    }

    TRACE("%02d%% - people with 20 marks or higher\n", (100 * numOfPeopleWith20Marks / (usersNum)));
    TRACE("%02d%% - people with 10 marks or higher\n", (100 * numOfPeopleWith10Marks / (usersNum)));
    TRACE("%02d%% - people with 5 marks or higher\n", (100 * numOfPeopleWith5Marks / (usersNum)));

    numOfPeopleWith20Marks = 0;
    numOfPeopleWith5Marks = 0;
    numOfPeopleWith10Marks = 0;
    fprintf(resfs, "makrs with faces:\n");
    for(int i = 0; i < 1024; i++)
    {
        if(i >= 5) numOfPeopleWith5Marks += res->marks_with_faces[i];
        if(i >= 10) numOfPeopleWith10Marks += res->marks_with_faces[i];
        if(i >= 20) numOfPeopleWith20Marks += res->marks_with_faces[i];
        fprintf(resfs, "%i\n", res->marks_with_faces[i]);
    }

    TRACE("%02d%% - people with 20 marks with faces or higher\n", (100 * numOfPeopleWith20Marks / (usersNum)));
    TRACE("%02d%% - people with 10 marks with faces or higher\n", (100 * numOfPeopleWith10Marks / (usersNum)));
    TRACE("%02d%% - people with 5 marks with faces or higher\n", (100 * numOfPeopleWith5Marks / (usersNum)));

    int numOfPeopleWith5Faces = 0;
    int numOfPeopleWith5SoloFaces = 0;
    int numOfPeopleWith10Faces = 0;
    int numOfPeopleWith10SoloFaces = 0;
    int numOfPeopleWith20Faces = 0;
    int numOfPeopleWith20SoloFaces = 0;

    for(int i = 0; i < 1024; i++)
    {
        if(i >= 5) numOfPeopleWith5Faces += res->photos_with_faces[i];
        if(i >= 10) numOfPeopleWith10Faces += res->photos_with_faces[i];
        if(i >= 20) numOfPeopleWith20Faces += res->photos_with_faces[i];
        if(i >= 5) numOfPeopleWith5SoloFaces += res->photos_with_solo_faces[i];
        if(i >= 10) numOfPeopleWith10SoloFaces += res->photos_with_solo_faces[i];
        if(i >= 20) numOfPeopleWith20SoloFaces += res->photos_with_solo_faces[i];
    }

    TRACE("%02d%% - people with 20 faces or higher\n", (100 * numOfPeopleWith20Faces / (usersNum )));
    TRACE("%02d%% - people with 10 faces or higher\n", (100 * numOfPeopleWith10Faces / (usersNum )));
    TRACE("%02d%% - people with 5 faces or higher\n", (100 * numOfPeopleWith5Faces / (usersNum )));

    TRACE("%02d%% - people with 20 solo faces or higher\n", (100 * numOfPeopleWith20SoloFaces / (usersNum )));
    TRACE("%02d%% - people with 10 solo faces or higher\n", (100 * numOfPeopleWith10SoloFaces / (usersNum )));
    TRACE("%02d%% - people with 5 solo faces or higher\n", (100 * numOfPeopleWith5SoloFaces / (usersNum )));

    fprintf(resfs, "friends:\n");
    for(int i = 0; i < 10240; i++)
    {
        fprintf(resfs, "%i\n", res->friends[i]);
    }

    fprintf(resfs, "photos:\n");
    for(int i = 0; i < 10240; i++)
    {
        fprintf(resfs, "%i\n", res->photos[i]);
    }

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
        do
        {
            struct results *res = params_copy.res;
            char *userName = params_copy.userName;
            int i = params_copy.i;
            char *userPath = NULL;
            char *jsonFilePath = NULL;
            json::Object json;
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

            try {
                markedPhotos = json[(std::string)"markedPhotos"].ToArray();
                friends = json[(std::string)"friends"].ToArray();
            } catch(...) {
                TRACE("Invalid json\n");
                goto getUserStats_finish;
            }

            numOfMarks = markedPhotos.size();
            if(numOfMarks > 1023) {
                numOfMarks = 1023;
            }
            numOfFriends = friends.size();
            if(numOfFriends > 10239) numOfFriends = 10239;

            pthread_mutex_lock(&results_lock);
            strcpy(res->users[i], userName);
            res->marks_without_faces[numOfMarks]++;
            res->friends[numOfFriends]++;
            pthread_mutex_unlock(&results_lock);

            if(TRUE == getFilesFromDir(userPath, &photos, &numOfPhotos))
            {
                if(numOfPhotos > 10240) numOfPhotos = 10240;
                pthread_mutex_lock(&results_lock);
                res->photos[numOfPhotos]++;
                pthread_mutex_unlock(&results_lock);
            }

            for(int cnt = 0; cnt < numOfPhotos; cnt++)
            {
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
                    pthread_mutex_lock(&results_lock);
                    res->user_photos_with_faces[i]++;
                    if(normFaces.size() == 1)
                    {
                        res->user_photos_with_solo_faces[i]++;
                    }
                    pthread_mutex_unlock(&results_lock);
                }
        next_photo:
                free(photoPath);
            }

            pthread_mutex_lock(&results_lock);
            if(res->user_photos_with_faces[i] > 1023)    res->user_photos_with_faces[i] = 1023;
            res->photos_with_faces[res->user_photos_with_faces[i]]++;
            if(res->user_photos_with_solo_faces[i] > 1023)    res->user_photos_with_solo_faces[i] = 1023;
            res->photos_with_solo_faces[res->user_photos_with_solo_faces[i]]++;
            res->user_marks[i] = markedPhotos.size();
            pthread_mutex_unlock(&results_lock);

            for(int cnt = 0; cnt < markedPhotos.size(); cnt++)
            {
                json::Object obj = markedPhotos[cnt];
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
                    pthread_mutex_lock(&results_lock);
                    res->user_marks_with_faces[i]++;
                    pthread_mutex_unlock(&results_lock);
                }
        next_marked_photo:
                free(photoPath);
            }
            pthread_mutex_lock(&results_lock);
            if(res->user_marks_with_faces[i] > 1023)    res->user_marks_with_faces[i] = 1023;
            res->marks_with_faces[res->user_marks_with_faces[i]]++;
            pthread_mutex_unlock(&results_lock);

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
