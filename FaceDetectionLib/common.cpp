#include "stdafx.h"
#include "LibInclude.h"
#include "io.h"

HANDLE    hStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
CRITICAL_SECTION fileCS;
CRITICAL_SECTION faceDetectionCS;

void InitFaceDetectionLib()
{
    InitializeCriticalSection(&faceDetectionCS);
    InitializeCriticalSection(&fileCS);
    
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
    }
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
    
    DeleteCriticalSection(&faceDetectionCS);
    DeleteCriticalSection(&fileCS);
}

void FilePrintMessage(char* file, char* expr...)
{
    EnterCriticalSection(&fileCS);
    char message[LOG_MESSAGE_MAX_LENGTH];
    va_list args;

    errno_t err = 0;

    if (expr)
    {
        va_start(args, expr);

        vsprintf(message, expr, args);

        ChooseTextColor(message);

        printf("%s", message);

        RestoreTextColor();

        if (file)
        {
            FILE* fl = NULL;

            if ((fl = fopen(file, "a+")) != NULL)
            {
                if (fl)
                {
                    fprintf(fl, "%s", message);
                }
            }
        }
        va_end(args);
    }
    LeaveCriticalSection(&fileCS);
}

void ChooseTextColor(char* msg)
{
    if (memcmp(&msg[0], "[", sizeof(char)) != 0)
        return;

    for (int i = 0; i < 20; i++) // (18 + 2) - max prefix length
    {
        if (memcmp(&msg[i], "]", sizeof(char)) == 0)
        {
            if (memcmp(msg, "[PASS]", i + 1) == 0)
                SetConsoleTextAttribute(hStdHandle, 0x0002 | FOREGROUND_INTENSITY);
            else if (memcmp(msg, "[FAIL]", i + 1) == 0)
                SetConsoleTextAttribute(hStdHandle, 0x0004 | FOREGROUND_INTENSITY);
            else if (memcmp(msg, "[SUCC]", i + 1) == 0)
                SetConsoleTextAttribute(hStdHandle, 0x0007 | FOREGROUND_INTENSITY);
            else if (memcmp(msg, "[WARN]", i + 1) == 0)
                SetConsoleTextAttribute(hStdHandle, 0x0006 | FOREGROUND_INTENSITY);
            else if (memcmp(msg, "[RES]", i + 1) == 0)
                SetConsoleTextAttribute(hStdHandle, 0x000b | FOREGROUND_INTENSITY);
            else
                SetConsoleTextAttribute(hStdHandle, 0x0002 | FOREGROUND_INTENSITY);

            break;
        }
    }
}

void RestoreTextColor()
{
    SetConsoleTextAttribute(hStdHandle, 0x0007);
}
