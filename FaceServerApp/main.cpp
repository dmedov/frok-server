#include "FaceServer.h"

// Add SHOW_IMAGE define to preprocessor defines in FaceDetectionApp and FaceDetectionLib projects to see resulting image

#define     NET_CMD_RECOGNIZE    "recognize"
#define     NET_CMD_TRAIN        "train"
#define     NET_CMD_GET_FACES    "get_faces"
#define     NET_CMD_SAVE_FACE    "save_face"

/*void callback(unsigned evt, SOCKET sock, unsigned length, void *param);

Network net(callback, PORT);

void callback(unsigned evt, SOCKET sock, unsigned length, void *param)
{
    if (sock == INVALID_SOCKET)
    {
        FilePrintMessage(_FAIL("Something went wrong. Msg received from invalid socket."));
        return;
    }
    switch (evt)
    {
        case
        NET_SERVER_CONNECTED:
        {
            printf("Received connection. Socket = 0x%x\n", sock);
            break;
        }
        case
        NET_SERVER_DISCONNECTED:
        {
            printf("Received disconnection. Socket = 0x%x\n", sock);
            break;
        }
        case
        NET_RECEIVED_REMOTE_DATA:
        {
            json::Object objInputJson;
            try
            {
                objInputJson = ((json::Value)json::Deserialize((string)((char*)param))).ToObject();
            }
            catch (...)
            {
                FilePrintMessage(_FAIL("Failed to parse incoming JSON: %s"), (char*)param);
                net.SendData(sock, "{ \"error\":\"bad command\" }\n\0", strlen("{ \"error\":\"bad command\" }\n\0"));
                return;
            }
            
            if (!objInputJson.HasKey("cmd"))
            {
                FilePrintMessage(_FAIL("Invalid input JSON: no cmd field (%s)"), (char*)param);
                net.SendData(sock, "{ \"error\":\"no cmd field\" }\n\0", strlen("{ \"error\":\"no cmd field\" }\n\0"));
                return;
            }

            // Parse cmd
            if (objInputJson["cmd"].ToString() == NET_CMD_RECOGNIZE)
            {
                if (!objInputJson.HasKey("friends"))
                {
                    FilePrintMessage(_FAIL("Invalid input JSON: no friends field (%s)"), (char*)param);
                    net.SendData(sock, "{ \"error\":\"no friends field\" }\n\0", strlen("{ \"error\":\"no friends field\" }\n\0"));
                    return;
                }
                
                if (!objInputJson.HasKey("photo_id"))
                {
                    FilePrintMessage(_FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
                    net.SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
                    return;
                }

                ContextForRecognize *psContext = new ContextForRecognize;
                psContext->arrFrinedsList = objInputJson["friends"].ToArray();
                psContext->targetImg = objInputJson["photo_id"].ToString();
                psContext->sock = sock;

                CommonThread *threadRecongnize = new CommonThread;
                threadRecongnize->startThread((void*(*)(void*))recognizeFromModel, psContext, sizeof(ContextForRecognize));
                FilePrintMessage(_SUCC("Recognizing started..."));
                // Notice that psContext should be deleted in recognizeFromModel function!
            }
            else if (objInputJson["cmd"].ToString() == NET_CMD_GET_FACES)
            {
                if (!objInputJson.HasKey("user_id"))
                {
                    FilePrintMessage(_FAIL("Invalid input JSON: no user_id field (%s)"), (char*)param);
                    net.SendData(sock, "{ \"error\":\"no user_id field\" }\n\0", strlen("{ \"error\":\"no user_id field\" }\n\0"));
                    return;
                }

                if (!objInputJson.HasKey("photo_id"))
                {
                    FilePrintMessage(_FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
                    net.SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
                    return;
                }

                ContextForGetFaces *psContext = new ContextForGetFaces;
                psContext->userId = objInputJson["user_id"].ToString();
                psContext->photoName = objInputJson["photo_id"].ToString();
                psContext->sock = sock;

                FilePrintMessage(_SUCC("Getting faces started..."));
                CommonThread *threadGetFaces = new CommonThread;
                threadGetFaces->startThread((void*(*)(void*))getFacesFromPhoto, psContext, sizeof(ContextForGetFaces));
                // Notice that psContext should be deleted in recognizeFromModel function!
            }
            else if (objInputJson["cmd"].ToString() == NET_CMD_SAVE_FACE)
            {
                if (!objInputJson.HasKey("user_id"))
                {
                    FilePrintMessage(_FAIL("Invalid input JSON: no user_id field (%s)"), (char*)param);
                    net.SendData(sock, "{ \"error\":\"no user_id field\" }\n\0", strlen("{ \"error\":\"no user_id field\" }\n\0"));
                    return;
                }

                if (!objInputJson.HasKey("photo_id"))
                {
                    FilePrintMessage(_FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
                    net.SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
                    return;
                }

                if (!objInputJson.HasKey("face_number"))
                {
                    FilePrintMessage(_FAIL("Invalid input JSON: no face_number field (%s)"), (char*)param);
                    net.SendData(sock, "{ \"error\":\"no face_points field\" }\n\0", strlen("{ \"error\":\"no face_number field\" }\n\0"));
                    return;
                }

                ContextForSaveFaces *psContext = new ContextForSaveFaces;
                psContext->userId = objInputJson["user_id"].ToString();
                psContext->photoName = objInputJson["photo_id"].ToString();
                psContext->faceNumber = atoi(objInputJson["face_number"].ToString().c_str());
                psContext->sock = sock;

                FilePrintMessage(_SUCC("Cut face started..."));
                CommonThread *threadSaveFaces = new CommonThread;
                threadSaveFaces->startThread((void*(*)(void*))saveFaceFromPhoto, psContext, sizeof(ContextForSaveFaces));
                // Notice that psContext should be deleted in recognizeFromModel function!
            }
            else if (objInputJson["cmd"].ToString() == NET_CMD_TRAIN)
            {
                if (!objInputJson.HasKey("ids"))
                {
                    FilePrintMessage(_FAIL("Invalid input JSON: no ids field (%s)"), (char*)param);
                    net.SendData(sock, "{ \"error\":\"no ids field\" }\n\0", strlen("{ \"error\":\"no ids field\" }\n\0"));
                    return;
                }

                ContextForTrain *psContext = new ContextForTrain;
                psContext->arrIds = objInputJson["ids"].ToArray();
                psContext->sock = sock;

                FilePrintMessage(_SUCC("Training started..."));
                CommonThread *threadTrain = new CommonThread;
                threadTrain->startThread((void*(*)(void*))generateAndTrainBase, (void*)psContext, sizeof(ContextForTrain));

                // Notice that psContext should be deleted in recognizeFromModel function!
            }
            else
            {
                FilePrintMessage(_FAIL("Invalid input JSON: invalid cmd received (%s)"), (char*)param);
                net.SendData(sock, "{ \"error\":\"invalid cmd\" }\n\0", strlen("{ \"error\":\"invalid cmd\" }\n\0"));
                return;
            }
            break;
        }
        default:
        {
            FilePrintMessage(_FAIL("Unknown event 0x%x"), evt);
            break;
        }
    }
    return;
}*/

void usage()
{
    FilePrintMessage(_SUCC("FaceDetectionApp <Local port number>"));
    return;
}

int main(void)
{
    std::vector<AgentInfo*> agentsInfo;
    agentsInfo.push_back(new AgentInfo(0, 0));
    FaceServer server(agentsInfo);
    /*InitFaceDetectionLib();

    FilePrintMessage(_SUCC("Starting network server with port = %d"), PORT);
    if (NET_SUCCESS != net.StartNetworkServer())
    {
        FilePrintMessage(_FAIL("Failed to start network. For additional info build project with NET_DEBUG_PRINT flag enabled"));
        DeinitFaceDetectionLib();
        return -1;
    }
    FilePrintMessage(_SUCC("Network server started!"));

    char train[] = "{\"cmd\":\"train\", \"ids\":[\"1\"]}\0";    // cut faces and train base
    callback(NET_RECEIVED_REMOTE_DATA, 1, strlen(train), train);

    //char save_face[] = "{\"cmd\":\"save_face\", \"user_id\":\"5\", \"photo_id\":\"1\", \"face_number\":\"0\"}\0";    // cut faces and train base
    //callback(NET_RECEIVED_REMOTE_DATA, 1, strlen(save_face), save_face);

    //char recognize[] = "{\"cmd\":\"recognize\", \"friends\":[\"1\"], \"photo_id\": \"2\"}\0";    // recognize name = 1.jpg
    //callback(NET_RECEIVED_REMOTE_DATA, 1, strlen(recognize), recognize);

    getchar();

    DeinitFaceDetectionLib();

    printf("success\n");*/
    return 0;
}
