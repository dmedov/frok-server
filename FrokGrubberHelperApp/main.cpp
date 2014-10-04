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

#define MODULE_NAME     "GRUB"

const char GRUBBER_PATH[] = "/home/zda/grubber/";
const char RESULT_FILE_PATH[] ="/home/zda/grubber/result.txt";

struct results
{
    unsigned marks[1024];
    unsigned friends[10240];
    unsigned photos[10240];
    unsigned marks_with_faces[1024];
};

BOOL getJsonFromFile(const char *filePath, json::Object &json);

int main(void)
{
    if(FROK_RESULT_SUCCESS != frokLibCommonInit(FROK_LIB_COMMON_DEFAULT_CONFIG_FILENAME))
    {
        TRACE_F("frokLibCommonInit");
        return -1;
    }

    //FaceDetectorAbstract *detector = new FrokFaceDetector;
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

    for(int i = 0; i < usersNum; i++)
    {
        int pcdone = 100 * i / usersNum;
        TRACE_S_T("[%02d%%] Processing user %s started", pcdone, users[i]);

        char *userPath = NULL;
        char *jsonFilePath = NULL;
        json::Object json;
        json::Array markedPhotos;
        json::Array friends;
        size_t numOfMarks = 0;
        size_t numOfFriends = 0;
        unsigned numOfPhotos = 0;
        char **photos = NULL;

        userPath = (char*)calloc(strlen(GRUBBER_PATH) + strlen(users[i]) + strlen("/") + 100, 1);
        if(userPath == NULL)
        {
            TRACE_F("calloc failed on error %s", strerror(errno));
            goto next_user_0;
        }

        strcpy(userPath, GRUBBER_PATH);
        strcat(userPath, users[i]);
        strcat(userPath, "/");

        jsonFilePath = (char*)calloc(strlen(userPath) + strlen(users[i]) + strlen(".json") + 100, 1);
        if(jsonFilePath == NULL)
        {
            TRACE_F("calloc failed on error %s", strerror(errno));
            goto next_user_0;
        }

        strcpy(jsonFilePath, userPath);
        strcat(jsonFilePath, users[i]);
        strcat(jsonFilePath, ".json");

        if(FALSE == getJsonFromFile(jsonFilePath, json))
        {
            TRACE_F("getJsonFromFile failed");
            goto next_user_0;
        }

        markedPhotos = json[(std::string)"markedPhotos"].ToArray();
        friends = json[(std::string)"friends"].ToArray();

        numOfMarks = markedPhotos.size();
        if(numOfMarks > 1023) {
            numOfMarks = 1023;
        }
        numOfFriends = friends.size();
        if(numOfFriends > 10239) numOfFriends = 10239;

        res.marks[numOfMarks]++;
        res.friends[numOfFriends]++;

        if(TRUE == getFilesFromDir(userPath, &photos, &numOfPhotos))
        {
            if(numOfPhotos > 10240) numOfPhotos = 10240;
            res.photos[numOfPhotos]++;
        }

        /*for(int cnt = 0; cnt < markedPhotos.size(); cnt++)
        {

        }*/

next_user_0:
        for(int cnt = 0;cnt < numOfPhotos; cnt++)
        {
            free(photos[cnt]);
        }
        free(photos);
        free(jsonFilePath);
        free(userPath);
        userPath = NULL;
        TRACE_S_T("[%02d%%] Processing user %s finished", pcdone, users[i]);
        free(users[i]);
    }

    free(users);

    // Print results
    unsigned numOfPeopleWith5Marks = 0;
    unsigned numOfPeopleWith10Marks = 0;
    unsigned numOfPeopleWith20Marks = 0;
    fprintf(resfs, "makrs:\n");
    for(int i = 0; i < 1024; i++)
    {
        if(i >= 5) numOfPeopleWith5Marks += res.marks[i];
        if(i >= 10) numOfPeopleWith10Marks += res.marks[i];
        if(i >= 20) numOfPeopleWith20Marks += res.marks[i];
        fprintf(resfs, "%i\n", res.marks[i]);
    }

    TRACE_R("%02d - people with 20 marks or higher", (100 * numOfPeopleWith20Marks / usersNum));
    TRACE_R("%02d - people with 10 marks or higher", (100 * numOfPeopleWith10Marks / usersNum));
    TRACE_R("%02d - people with 5 marks or higher", (100 * numOfPeopleWith5Marks / usersNum));

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

