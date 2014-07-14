#include "FaceAgent.h"
#include <stdio.h>

void usage()
{
    FilePrintMessage(_SUCC("FaceDetectionApp <Local port number>"));
    return;
}

int main(void)
{
    FaceAgent agent(DEFAULT_PORT);
    agent.StartFaceAgent();
    getchar();

    return 0;
}
