#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "FrokAPI.h"
#include "frokLibCommon.h"

#define MODULE_NAME     "GRUB"

#define GRUBBER_PATH    "/home/zda/grubber/"

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

        //TRACE_N("Parsed json: %s", json::Serialize(json).c_str());
        sleep(1);
next_user_0:
        free(jsonFilePath);
        jsonFilePath = NULL;
        TRACE_S_T("[%02d%%] Processing user %s finished", pcdone, users[i]);
    }

    return 0;
}

BOOL getJsonFromFile(const char *filePath, json::Object &json)
{
    int fd = open(filePath, O_RDWR);
    if(fd == -1)
    {
        TRACE_F("Failed to open file %s on error %s", filePath, strerror(errno));
        return FALSE;
    }

    FILE *fs = fdopen(fd, "r+");
    if(!fs)
    {
        TRACE_F("Failed to open fs on error %s", strerror(errno));
        close(fd);
        return FALSE;
    }

    if(-1 == fseek(fs, 0, SEEK_END))
    {
        TRACE_F("fseek failed on error %s", strerror(errno));
        fclose(fs);
        return FALSE;
    }

    long fileSize = ftell(fs);
    if(-1 == fileSize)
    {
        TRACE_F("ftell failed on error %s", strerror(errno));
        fclose(fs);
        return FALSE;
    }

    if(-1 == fseek(fs, 0, SEEK_SET))
    {
        TRACE_F("fseek failed on error %s", strerror(errno));
        fclose(fs);
        return FALSE;
    }

    char *buffer = (char*)calloc(fileSize + 1, 1);
    if(buffer == NULL)
    {
        TRACE_F("calloc failed on error %s", strerror(errno));
        fclose(fs);
        return FALSE;
    }

    size_t nr;
    nr = fread(buffer, fileSize, 1, fs);
    if(nr == 0)
    {
        TRACE_F("Failed to read file with fread");
        fclose(fs);
        return FALSE;
    }

    try {
        json = json::Deserialize(buffer);
    } catch(...) {
        TRACE_F("Failed to deserialize jsonFile");
        fclose(fs);
        return FALSE;
    }

    fclose(fs);
    return TRUE;
}

