#include "stdafx.h"
#include "LibInclude.h"
// Add SHOW_IMAGE define to preprocessor defines in FaceDetectionApp and FaceDetectionLib projects to see resulting image

#define	NET_CMD_RECOGNIZE	"recognize"
#define NET_CMD_TRAIN		"train"
#define	NET_CMD_GET_FACES	"get_faces"
#define NET_CMD_SAVE_FACE	"save_face"

Network net;

void callback(SOCKET sock, unsigned evt, unsigned length, void *param)
{
	if (sock == INVALID_SOCKET)
	{
		FilePrintMessage(NULL, _FAIL("Something went wrong. Msg received from invalid socket."));
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
				FilePrintMessage(NULL, _FAIL("Failed to parse incoming JSON: %s"), (char*)param);
				net.SendData(sock, "{ \"error\":\"bad command\" }\n\0", strlen("{ \"error\":\"bad command\" }\n\0"));
				return;
			}
			
			if (!objInputJson.HasKey("cmd"))
			{
				FilePrintMessage(NULL, _FAIL("Invalid input JSON: no cmd field (%s)"), (char*)param);
				net.SendData(sock, "{ \"error\":\"no cmd field\" }\n\0", strlen("{ \"error\":\"no cmd field\" }\n\0"));
				return;
			}

			// Parse cmd
			if (objInputJson["cmd"].ToString() == NET_CMD_RECOGNIZE)
			{
				if (!objInputJson.HasKey("friends"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no friends field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no friends field\" }\n\0", strlen("{ \"error\":\"no friends field\" }\n\0"));
					return;
				}
				
				if (!objInputJson.HasKey("photo_id"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
					return;
				}

				ContextForRecognize *psContext = new ContextForRecognize;
				memset(psContext, 0, sizeof(ContextForRecognize));
				psContext->arrFrinedsList = objInputJson["friends"].ToArray();
				psContext->targetImg = objInputJson["photo_id"];
				psContext->sock = sock;

				FilePrintMessage(NULL, _SUCC("Recognizing started..."));
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recognizeFromModel, psContext, 0, NULL);
				// Notice that psContext should be deleted in recognizeFromModel function!
			}
			else if (objInputJson["cmd"].ToString() == NET_CMD_GET_FACES)
			{
				if (!objInputJson.HasKey("user_id"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no user_id field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no user_id field\" }\n\0", strlen("{ \"error\":\"no user_id field\" }\n\0"));
					return;
				}

				if (!objInputJson.HasKey("photo_id"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
					return;
				}

				ContextForGetFaces *psContext = new ContextForGetFaces;
				memset(psContext, 0, sizeof(ContextForGetFaces));
				psContext->userId = objInputJson["user_id"].ToString();
				psContext->photoName = objInputJson["photo_id"].ToString();
				psContext->sock = sock;

				FilePrintMessage(NULL, _SUCC("Getting faces started..."));
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)getFacesFromPhoto, psContext, 0, NULL);
				// Notice that psContext should be deleted in recognizeFromModel function!
			}
			else if (objInputJson["cmd"].ToString() == NET_CMD_SAVE_FACE)
			{
				if (!objInputJson.HasKey("user_id"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no user_id field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no user_id field\" }\n\0", strlen("{ \"error\":\"no user_id field\" }\n\0"));
					return;
				}

				if (!objInputJson.HasKey("photo_id"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no photo_id field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no photo_id field\" }\n\0", strlen("{ \"error\":\"no photo_id field\" }\n\0"));
					return;
				}

				if (!objInputJson.HasKey("face_number"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no face_number field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no face_points field\" }\n\0", strlen("{ \"error\":\"no face_number field\" }\n\0"));
					return;
				}

				ContextForSaveFaces *psContext = new ContextForSaveFaces;
				psContext->userId = objInputJson["user_id"].ToString();
				psContext->photoName = objInputJson["photo_id"].ToString();
				psContext->faceNumber = atoi(objInputJson["face_number"].ToString().c_str());
				psContext->sock = sock;

				FilePrintMessage(NULL, _SUCC("Cut face started..."));
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)saveFaceFromPhoto, psContext, 0, NULL);
				// Notice that psContext should be deleted in recognizeFromModel function!
			}
			else if (objInputJson["cmd"].ToString() == NET_CMD_TRAIN)
			{
				if (!objInputJson.HasKey("ids"))
				{
					FilePrintMessage(NULL, _FAIL("Invalid input JSON: no ids field (%s)"), (char*)param);
					net.SendData(sock, "{ \"error\":\"no ids field\" }\n\0", strlen("{ \"error\":\"no ids field\" }\n\0"));
					return;
				}

				ContextForTrain *psContext = new ContextForTrain;
				memset(psContext, 0, sizeof(ContextForTrain));
				psContext->arrIds = objInputJson["ids"].ToArray();
				psContext->sock = sock;

				FilePrintMessage(NULL, _SUCC("Training started..."));
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)generateAndTrainBase, psContext, 0, NULL);
				// Notice that psContext should be deleted in recognizeFromModel function!
			}
			else
			{
				FilePrintMessage(NULL, _FAIL("Invalid input JSON: invalid cmd received (%s)"), (char*)param);
				net.SendData(sock, "{ \"error\":\"invalid cmd\" }\n\0", strlen("{ \"error\":\"invalid cmd\" }\n\0"));
				return;
			}
			break;
		}
		default
			:
		{
			FilePrintMessage(NULL, _FAIL("Unknown event 0x%x"), evt);
			break;
		}
	}
	return;
}

void usage()
{
	FilePrintMessage(NULL, _SUCC("FaceDetectionApp <Local port number>"));
	return;
}

int main(int argc, char *argv[]) 
{
	InitFaceDetectionLib();
	if (argc != 2)
	{
		FilePrintMessage(NULL, _FAIL("Invalid input patemeters."));
		usage();
		DeleteCriticalSection(&faceDetectionCS);
		DeleteCriticalSection(&fileCS);
		return -1;
	}
	
	unsigned uPort = atoi(argv[1]);

	FilePrintMessage(NULL, _SUCC("Starting network server with port = %d"), uPort);
	if (NET_SUCCESS != net.StartNetworkServer(callback, uPort))
	{
		FilePrintMessage(NULL, _FAIL("Failed to start network. For additional info build project with NET_DEBUG_PRINT flag enabled"));
		DeleteCriticalSection(&faceDetectionCS);
		DeleteCriticalSection(&fileCS);
		return -1;
	}
	FilePrintMessage(NULL, _SUCC("Network server started!"));
	
	//char train[] = "{\"cmd\":\"train\", \"ids\":[\"5\"]}\0";	// cut faces and train base
	//callback(1, NET_RECEIVED_REMOTE_DATA, strlen(train), train);

	//char save_face[] = "{\"cmd\":\"save_face\", \"user_id\":\"5\", \"photo_id\":\"1\", \"face_number\":\"0\"}\0";	// cut faces and train base
	//callback(1, NET_RECEIVED_REMOTE_DATA, strlen(save_face), save_face);

	//char recognize[] = "{\"cmd\":\"recognize\", \"friends\":[\"5\", \"1\", \"99\"], \"photo_id\": \"1\"}\0";	// recognize name = 1.jpg
	//callback(1, NET_RECEIVED_REMOTE_DATA, strlen(recognize), recognize);
	
	getchar();

	DeinitFaceDetectionLib();

	return 0;
}