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

#define MODULE_NAME     "GRUB"

const char GRUBBER_PATH[] = "/home/zda/grubber/";
const char RESULT_FILE_PATH[] ="/home/zda/grubber/result.txt";
#define MAX_USERS_NUM 5000

pthread_mutex_t results_lock = PTHREAD_MUTEX_INITIALIZER;

struct results
{
    unsigned photos_with_faces[1024];
    unsigned photos_with_solo_faces[1024];
    unsigned marks_without_faces[1024];
    unsigned marks_with_faces[1024];
    unsigned friends[10240];
    unsigned photos[10240];
    unsigned user_photos_with_faces[MAX_USERS_NUM];
    unsigned user_photos_with_solo_faces[MAX_USERS_NUM];
    unsigned user_marks[MAX_USERS_NUM];
    unsigned user_marks_with_faces[MAX_USERS_NUM];
};

struct thread_params
{
    int i;
    struct results *res;
    char *userName;
};

BOOL getJsonFromFile(const char *filePath, json::Object &json);

int main(void)
{
    if(FROK_RESULT_SUCCESS != frokLibCommonInit(FROK_LIB_COMMON_DEFAULT_CONFIG_FILENAME))
    {
        TRACE_F("frokLibCommonInit");
        return -1;
    }

    FaceDetectorAbstract *detector = new FrokFaceDetector;
    char **users = NULL;
    unsigned usersNum = 0;
    struct results res;
    memset(&res, 0, sizeof(res));

    FILE *resfs = fopen(RESULT_FILE_PATH, "w");
    if(!resfs)
    {
        TRACE_F("Failed to open file %s for writing", RESULT_FILE_PATH);
        return -1;
    }

    if(TRUE != getSubdirsFromDir(GRUBBER_PATH, &users, &usersNum))
    {
        TRACE_F("Failed to get users");
        return -1;
    }

    if(usersNum > MAX_USERS_NUM)
    {
        TRACE("Too much users\n");
        return -1;
    }

    for(int i = 0; i < usersNum; i++)
    {
        int pcdone = 100 * i / usersNum;
        TRACE_TIMESTAMP("[%02d%%] Processing user %s started\n", pcdone, users[i]);

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

        userPath = (char*)calloc(strlen(GRUBBER_PATH) + strlen(users[i]) + strlen("/") + 100, 1);
        if(userPath == NULL)
        {
            TRACE("calloc failed on error %s\n", strerror(errno));
            goto next_user_0;
        }

        strcpy(userPath, GRUBBER_PATH);
        strcat(userPath, users[i]);
        strcat(userPath, "/");

        jsonFilePath = (char*)calloc(strlen(userPath) + strlen(users[i]) + strlen(".json") + 100, 1);
        if(jsonFilePath == NULL)
        {
            TRACE("calloc failed on error %s\n", strerror(errno));
            goto next_user_0;
        }

        strcpy(jsonFilePath, userPath);
        strcat(jsonFilePath, users[i]);
        strcat(jsonFilePath, ".json");

        if(FALSE == getJsonFromFile(jsonFilePath, json))
        {
            TRACE("getJsonFromFile failed\n");
            goto next_user_0;
        }

        try {
            markedPhotos = json[(std::string)"markedPhotos"].ToArray();
            friends = json[(std::string)"friends"].ToArray();
        } catch(...) {
            TRACE("Invalid json\n");
            goto next_user_0;
        }

        numOfMarks = markedPhotos.size();
        if(numOfMarks > 1023) {
            numOfMarks = 1023;
        }
        numOfFriends = friends.size();
        if(numOfFriends > 10239) numOfFriends = 10239;

        res.marks_without_faces[numOfMarks]++;
        res.friends[numOfFriends]++;

        if(TRUE == getFilesFromDir(userPath, &photos, &numOfPhotos))
        {
            if(numOfPhotos > 10240) numOfPhotos = 10240;
            res.photos[numOfPhotos]++;
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
                res.user_photos_with_faces[i]++;
                if(normFaces.size() == 1)
                {
                    res.user_photos_with_solo_faces[i]++;
                }
            }
next_photo:
            free(photoPath);
        }

        if(res.user_photos_with_faces[i] > 1023)    res.user_photos_with_faces[i] = 1023;
        res.photos_with_faces[res.user_photos_with_faces[i]]++;
        if(res.user_photos_with_solo_faces[i] > 1023)    res.user_photos_with_solo_faces[i] = 1023;
        res.photos_with_solo_faces[res.user_photos_with_solo_faces[i]]++;

        res.user_marks[i] = markedPhotos.size();
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
                res.user_marks_with_faces[i]++;
            }
next_marked_photo:
            free(photoPath);
        }
        if(res.user_marks_with_faces[i] > 1023)    res.user_marks_with_faces[i] = 1023;
        res.marks_with_faces[res.user_marks_with_faces[i]]++;

next_user_0:
        for(int cnt = 0;cnt < numOfPhotos; cnt++)
        {
            free(photos[cnt]);
        }
        free(photos);
        free(jsonFilePath);
        free(userPath);
        userPath = NULL;
        TRACE_TIMESTAMP("[%02d%%] Processing user %s finished\n", pcdone, users[i]);
        free(users[i]);
    }

    free(users);

    // Print results
    unsigned numOfPeopleWith5Marks = 0;
    unsigned numOfPeopleWith10Marks = 0;
    unsigned numOfPeopleWith20Marks = 0;

    fprintf(resfs, "makrs_with_faces:\n");
    for(int i = 0; i < 5000; i++)
    {
        fprintf(resfs, "%i %i\n", res.user_marks[i], res.user_marks_with_faces[i]);
    }

    fprintf(resfs, "makrs without faces:\n");
    for(int i = 0; i < 1024; i++)
    {
        if(i >= 5) numOfPeopleWith5Marks += res.marks_without_faces[i];
        if(i >= 10) numOfPeopleWith10Marks += res.marks_without_faces[i];
        if(i >= 20) numOfPeopleWith20Marks += res.marks_without_faces[i];
        fprintf(resfs, "%i\n", res.marks_without_faces[i]);
    }

    TRACE("%02d - people with 20 marks or higher\n", (100 * numOfPeopleWith20Marks / usersNum));
    TRACE("%02d - people with 10 marks or higher\n", (100 * numOfPeopleWith10Marks / usersNum));
    TRACE("%02d - people with 5 marks or higher\n", (100 * numOfPeopleWith5Marks / usersNum));

    numOfPeopleWith20Marks = 0;
    numOfPeopleWith5Marks = 0;
    numOfPeopleWith10Marks = 0;
    fprintf(resfs, "makrs with faces:\n");
    for(int i = 0; i < 1024; i++)
    {
        if(i >= 5) numOfPeopleWith5Marks += res.marks_with_faces[i];
        if(i >= 10) numOfPeopleWith10Marks += res.marks_with_faces[i];
        if(i >= 20) numOfPeopleWith20Marks += res.marks_with_faces[i];
        fprintf(resfs, "%i\n", res.marks_with_faces[i]);
    }

    TRACE("%02d - people with 20 marks with faces or higher\n", (100 * numOfPeopleWith20Marks / usersNum));
    TRACE("%02d - people with 10 marks with faces or higher\n", (100 * numOfPeopleWith10Marks / usersNum));
    TRACE("%02d - people with 5 marks with faces or higher\n", (100 * numOfPeopleWith5Marks / usersNum));

    int numOfPeopleWith5Faces = 0;
    int numOfPeopleWith5SoloFaces = 0;
    int numOfPeopleWith10Faces = 0;
    int numOfPeopleWith10SoloFaces = 0;
    int numOfPeopleWith20Faces = 0;
    int numOfPeopleWith20SoloFaces = 0;

    for(int i = 0; i < 1024; i++)
    {
        if(i >= 5) numOfPeopleWith5Faces += res.photos_with_faces[i];
        if(i >= 10) numOfPeopleWith10Faces += res.photos_with_faces[i];
        if(i >= 20) numOfPeopleWith20Faces += res.photos_with_faces[i];
        if(i >= 5) numOfPeopleWith5SoloFaces += res.photos_with_solo_faces[i];
        if(i >= 10) numOfPeopleWith10SoloFaces += res.photos_with_solo_faces[i];
        if(i >= 20) numOfPeopleWith20SoloFaces += res.photos_with_solo_faces[i];
    }

    TRACE("%02d - people with 20 faces or higher\n", (100 * numOfPeopleWith20Faces / usersNum));
    TRACE("%02d - people with 10 faces or higher\n", (100 * numOfPeopleWith10Faces / usersNum));
    TRACE("%02d - people with 5 faces or higher\n", (100 * numOfPeopleWith5Faces / usersNum));

    TRACE("%02d - people with 20 solo faces or higher\n", (100 * numOfPeopleWith20SoloFaces / usersNum));
    TRACE("%02d - people with 10 solo faces or higher\n", (100 * numOfPeopleWith10SoloFaces / usersNum));
    TRACE("%02d - people with 5 solo faces or higher\n", (100 * numOfPeopleWith5SoloFaces / usersNum));

    fprintf(resfs, "friends:\n");
    for(int i = 0; i < 10240; i++)
    {
        fprintf(resfs, "%i\n", res.friends[i]);
    }

    fprintf(resfs, "photos:\n");
    for(int i = 0; i < 10240; i++)
    {
        fprintf(resfs, "%i\n", res.photos[i]);
    }

    fclose(resfs);

    delete detector;
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

void getUserStats(void *params)
{
    struct results *res = ((struct thread_params *)params)->res;
    char *userName = ((struct thread_params *)params)->userName;
    int i = ((struct thread_params *)params)->i;
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
    pthread_mutex_lock(&results_lock);

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
    free(params);
}
