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

#define GRUBBER_PATH    "/home/zda/grubber/"
#define RESULT_FILE_PATH    "/home/zda/grubber/result.txt"

struct results
{
    unsigned marks[1024];
    unsigned friends[10240];
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

    for(int i = 0; i < usersNum; i++)
    {
        int pcdone = 100 * i / usersNum;
        TRACE_S_T("[%02d%%] Processing user %s started", pcdone, users[i]);

        json::Object json;
        json::Array markedPhotos;
        json::Array friends;
        size_t numOfMarks;
        size_t numOfFriends;

        char *jsonFilePath = (char*)calloc(strlen(GRUBBER_PATH) + strlen(users[i]) + strlen("/")
                                           + strlen(users[i]) + strlen(".json") + 1, 1);
        if(jsonFilePath == NULL)
        {
            TRACE_F("calloc failed on error %s", strerror(errno));
            goto next_user_0;
        }

        strcpy(jsonFilePath, GRUBBER_PATH);
        strcat(jsonFilePath, users[i]);
        strcat(jsonFilePath, "/");
        strcat(jsonFilePath, users[i]);
        strcat(jsonFilePath, ".json");

        if(FALSE == getJsonFromFile(jsonFilePath, json))
        {
            TRACE_F("getJsonFromFile failed");
            goto next_user_0;
        }

        markedPhotos = json["markedPhotos"].ToArray();
        friends = json["friends"].ToArray();

        numOfMarks = markedPhotos.size();;
        if(numOfMarks > 1023) {
            numOfMarks = 1023;
        }
        numOfFriends = friends.size();;
        if(numOfFriends > 10239) numOfFriends = 10239;

        res.marks[numOfMarks]++;
        res.friends[numOfFriends]++;

next_user_0:
        free(jsonFilePath);
        jsonFilePath = NULL;
        TRACE_S_T("[%02d%%] Processing user %s finished", pcdone, users[i]);
    }

    // Print results
    fprintf(resfs, "makrs:\n");
    for(int i = 0; i < 1024; i++)
    {
        fprintf(resfs, "%i\n", res.marks[i]);
    }

    fprintf(resfs, "friends:\n");
    for(int i = 0; i < 10240; i++)
    {
        fprintf(resfs, "%i\n", res.friends[i]);
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

    char *buffer = (char*)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if(buffer == MAP_FAILED)
    {
        TRACE_F("mmap failed on error %s", strerror(errno));
        close(fd);
        return FALSE;
    }

    try {
        json = json::Deserialize(buffer);
    } catch(...) {
        TRACE_F("Failed to deserialize jsonFile");
        munmap(buffer, sb.st_size);
        close(fd);
        return FALSE;
    }

    munmap(buffer, sb.st_size);
    close(fd);
    return TRUE;
}

