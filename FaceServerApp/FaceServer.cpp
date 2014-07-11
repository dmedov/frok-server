#include "stdafx.h"

#include "FaceServer.h"

FaceServer::FaceServer(char *photoBasePath, char*targetsFolderPath)
{
    this->photoBasePath = new char[strlen(photoBasePath) + 1];
    this->targetsFolderPath = new char[strlen(targetsFolderPath) + 1];
    strcpy(this->photoBasePath, photoBasePath);
    strcpy(this->photoBasePath, photoBasePath);
}

FaceServer::~FaceServer()
{
    delete []photoBasePath;
    delete []targetsFolderPath;
}

bool FaceServer::StartFaceServer()
{
    return true;
}

bool FaceServer::StopFaceServer()
{
    return true;
}
