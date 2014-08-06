#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <list>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#define MODULE_NAME     "AGENT"

#include "FrokAgent.h"

pthread_mutex_t             frokAgent_trace_cs;
pthread_mutex_t             frokAgent_cs;
FrokAgent::FrokAgent(std::map<std::string, FrokAPIFunction*> enabledFucntions, unsigned short localPort, const char *photoBasePath, const char *targetsFolderPath)
{
    this->photoBasePath = new char[strlen(photoBasePath) + 1];
    this->targetsFolderPath = new char[strlen(targetsFolderPath) + 1];
    strcpy(this->photoBasePath, photoBasePath);
    strcpy(this->photoBasePath, photoBasePath);

    int res = 0;
    pthread_mutexattr_t mAttr;
    if(0 != (res = pthread_mutexattr_init(&mAttr)))
    {
        TRACE_F("pthread_mutexattr_init failed on error %s", strerror(res));
        throw NET_MEM_ALLOCATION_FAIL;
    }
    if(0 != (res = pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP)))
    {
        TRACE_F("pthread_mutexattr_settype failed on error %s", strerror(res));
        throw NET_MEM_ALLOCATION_FAIL;
    }
    if(0 != (res = pthread_mutex_init(&frokAgent_trace_cs, &mAttr)))
    {
        TRACE_F("pthread_mutex_init failed on error %s", strerror(res));
        throw NET_MEM_ALLOCATION_FAIL;
    }
    if(0 != (res = pthread_mutex_init(&frokAgent_cs, &mAttr)))
    {
        TRACE_F("pthread_mutex_init failed on error %s", strerror(res));
        throw NET_MEM_ALLOCATION_FAIL;
    }
    if(0 != (res = pthread_mutexattr_destroy(&mAttr)))
    {
        TRACE_F("pthread_mutexattr_destroy failed on error %s", strerror(res));
        throw NET_MEM_ALLOCATION_FAIL;
    }

    localSock = INVALID_SOCKET;

    this->localPortNumber = localPort;

    try
    {
        recognizer = new FaceRecognizerEigenfaces;
        detector = new FrokFaceDetector;
    }
    catch(...)
    {
        TRACE_F("Failed to create recognizer and detector object");
        throw NET_MEM_ALLOCATION_FAIL;
    }

    try
    {
        fapi = new FrokAPI(this->photoBasePath, this->targetsFolderPath, detector, recognizer);
    }
    catch(...)
    {
        TRACE_F("Failed to create FrokAPI object");
        throw NET_MEM_ALLOCATION_FAIL;
    }

    TRACE("List of included functions:");
    for(std::map<std::string, FrokAPIFunction*>::iterator it = enabledFucntions.begin(); it != enabledFucntions.end(); ++it)
    {
        TRACE("\t%s", it->first.c_str());
        fapi->AddAPIFunction(it->first, it->second);
    }
    TRACE("new FrokAgent");
}

FrokAgent::~FrokAgent()
{
    delete recognizer;
    delete detector;

    delete []photoBasePath;
    delete []targetsFolderPath;

    if(localSock != INVALID_SOCKET)
    {
        shutdown(localSock, 2);
        close(localSock);
    }

    pthread_mutex_destroy(&frokAgent_cs);
    pthread_mutex_destroy(&frokAgent_trace_cs);
    TRACE("~FrokAgent");
}

bool FrokAgent::StartFrokAgent()
{
    TRACE("Calling InitFaceCommonLib");
    if(!InitFaceCommonLib("FrokAgent.log"))
    {
        TRACE_F("InitFaceCommonLib failed");
        return false;
    }
    TRACE_S("InitFaceCommonLib succeed");
    TRACE("Calling StartNetworkServer");
    NetResult res;
    if(NET_SUCCESS != (res = StartNetworkServer()))
    {
        TRACE_F("StartNetworkServer failed on error %x", res);
        return false;
    }
    TRACE_S("StartNetworkServer succeed");

    ServerListener();

    return true;
}

NetResult FrokAgent::StartNetworkServer()
{
    sockaddr_in             server;
    memset(&server, 0, sizeof(server));

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(localPortNumber);

    if ((localSock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET)
    {
        TRACE_F("socket failed on error = %s", strerror(errno));
        return NET_SOCKET_ERROR;
    }

    //[TBD] think about SO_KEEPALIVE option

    int option = 1;
    if(0 != setsockopt(localSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        TRACE_F("setsockopt failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    if(0 != setsockopt(localSock, SOL_SOCKET, SO_KEEPALIVE, &option, sizeof(option)))
    {
        TRACE_F("setsockopt (SO_KEEPALIVE) failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    int         optval;
    socklen_t   optlen = sizeof(int);

    if(0 != getsockopt(localSock, SOL_SOCKET,  SO_REUSEADDR, &optval, &optlen))
    {
        TRACE_F("getsockopt failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    if(optval == 0)
    {
        TRACE_F("setsockopt failed to enabled SO_REUSEADDR option");
        //return NET_SOCKET_ERROR;
    }

    if(0 != bind(localSock, (struct sockaddr*)&server, sizeof(server)))
    {
        TRACE_F("bind failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    if (0 != listen(localSock, SOMAXCONN))
    {
        TRACE_F("listen failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        close(localSock);
        return NET_UNSPECIFIED_ERROR;
    }

    TRACE_S("Succeed, socket = %d, port = %d", localSock, localPortNumber);

    return NET_SUCCESS;
}

bool FrokAgent::StopFrokAgent()
{
    bool success = true;

    NetResult res;

    TRACE("Calling StopNetworkServer");
    if(NET_SUCCESS != (res = StopNetworkServer()))
    {
        TRACE_F("StopNetworkServer failed on result %x. Continue...", res);
        success = false;
    }
    TRACE_S("StopNetworkServer succeed");
    TRACE("Calling DeinitFaceCommonLib");
    if(!DeinitFaceCommonLib())
    {
        TRACE_F("DeinitFaceCommonLib failed");
        success = false;
    }
    TRACE_S("DeinitFaceCommonLib succeed");

    return success;
}

void FrokAgent::ServerListener()
{
    SOCKET                      accepted_socket             = INVALID_SOCKET;
    int                         dataLength                  = 0;
    char                        data[MAX_SOCKET_BUFF_SIZE]  = {0};
    std::vector<std::string>    mandatoryKeys;
    std::vector<std::string>    includedFunctions;

    fapi->GetAvailableFunctions(includedFunctions);

    mandatoryKeys.push_back("cmd");
    mandatoryKeys.push_back("req_id");
    mandatoryKeys.push_back("reply_sock");

    TRACE("Accepting single incoming connections for socket %i", localSock);

    for(;;)
    {
        if(accepted_socket != INVALID_SOCKET)
        {
            shutdown(accepted_socket, 2);
            close(accepted_socket);
        }

        if ((accepted_socket = accept(localSock, NULL, NULL)) == SOCKET_ERROR)
        {
            break;
        }

        TRACE("Incoming connection accepted. Accepted socket = %u", accepted_socket);

        int flag = 1;   //TRUE
        if(0 != setsockopt(accepted_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)))
        {
            TRACE_F("setsockopt (TCP_NODELAY) failed on error %s", strerror(errno));
            TRACE_F("Accepting single incoming connections for socket %i", localSock);
            continue;
        }

        if(0 != setsockopt(accepted_socket, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(int)))
        {
            TRACE_F("setsockopt (SO_KEEPALIVE) failed on error %s", strerror(errno));
            TRACE_F("Accepting single incoming connections for socket %i", localSock);
            continue;
        }

        if(0 != setsockopt(accepted_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)))
        {
            TRACE_F("setsockopt (SO_REUSEADDR) failed on error %s", strerror(errno));
            TRACE_F("Accepting single incoming connections for socket %i", localSock);
            continue;
        }

        TRACE_S("Socket listener cycle started. The agent will process one request and then connection would be terminated.");
        for(;;)
        {
            if(localSock == INVALID_SOCKET)
            {
                TRACE("AcceptConnection finished");
                return;
            }

            if( -1 == (dataLength = recv(accepted_socket, data, sizeof(data), MSG_DONTWAIT)))
            {
                if((errno == EAGAIN) || (errno == EWOULDBLOCK))
                {
                    // Valid errors - continue
                    continue;
                }
                // unspecified error occured
                TRACE_F("recv failed on error %s", strerror(errno));
                TRACE_W("Listening to socket %u stopped", accepted_socket);
                shutdown(accepted_socket, 2);
                close(accepted_socket);
                TRACE("Accepting single incoming connections for socket %i", localSock);
                break;
            }

            if(dataLength == 0)
            {
                TRACE("Half disconnection received. Terminating...");
                TRACE("Accepting single incoming connections for socket %i", localSock);
                break;
            }

            TRACE("Received %d bytes from the server(%u)", dataLength, accepted_socket);

            // Response from agent received
            json::Object requestJson;
            json::Object resultJson;
            try
            {
                requestJson = ((json::Value)json::Deserialize((std::string)data)).ToObject();
            }
            catch (...)
            {
                TRACE_F("Failed to parse incoming JSON: %s", data);
                TRACE("Accepting single incoming connections for socket %i", localSock);
                shutdown(accepted_socket, 2);
                close(accepted_socket);
                break;
            }

            if (!(requestJson.HasKeys(mandatoryKeys)))
            {
                TRACE_F("Invalid input JSON: no cmd field (%s)", data);
                TRACE("Accepting single incoming connections for socket %i", localSock);
                shutdown(accepted_socket, 2);
                close(accepted_socket);
                break;
            }

            std::string cmd = requestJson["cmd"].ToString();
            bool noSuchFunction = true;
            for(std::vector<std::string>::iterator it = includedFunctions.begin(); it != includedFunctions.end(); ++it)
            {
                if(*it == cmd)
                {
                    noSuchFunction = false;
                    break;
                }
            }
            if(noSuchFunction == true)
            {
                resultJson["result"] = "fail";
                resultJson["reason"] = "cmd not supported";
                continue;
            }

            std::string responseString;
            FrokResult res = fapi->ExecuteFunction(cmd, (std::string)data, responseString);
            if(res != FROK_RESULT_SUCCESS)
            {
                resultJson["result"] = "fail";
                resultJson["reason"] = FrokResultToString(res);
                goto sendResult;
            }
            try
            {
                resultJson = json::Deserialize(responseString);
            }
            catch(...)
            {
                resultJson["result"] = "fail";
                resultJson["reason"] = "internal error";
                goto sendResult;
            }

            resultJson["result"] = "success";

sendResult:
            resultJson["reply_sock"] = requestJson["reply_sock"];
            resultJson["req_id"] = requestJson["req_id"];
            std::string outJson = json::Serialize(resultJson);

            TRACE_S("Response json: %s", outJson.c_str());

            if(NET_SUCCESS != SendData(accepted_socket, outJson.c_str(), outJson.size()))
            {
                TRACE_F("Failed to send response to server (%u). Reponse = %s", accepted_socket, outJson.c_str());
                shutdown(accepted_socket, 2);
                close(accepted_socket);
                TRACE("Accepting single incoming connections for socket %i", localSock);
                break;
            }
            shutdown(accepted_socket, 2);
            close(accepted_socket);
            TRACE("Accepting single incoming connections for socket %i", localSock);
            break;
        }
    }

    if(accepted_socket != INVALID_SOCKET)
    {
        shutdown(accepted_socket, 2);
        close(accepted_socket);
    }

    shutdown(localSock, 2);
    close(localSock);
    TRACE("AcceptConnection finished");
    return;
}

NetResult FrokAgent::SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize)
{
    int sendlen = 0;

    if(sock != INVALID_SOCKET)
    {
        if (-1 == (sendlen = send(sock, pBuffer, uBufferSize, 0)))
        {
            TRACE_F("Failed to send outgoing bytes to the remote peer %u with error %s", sock, strerror(errno));
            return NET_SOCKET_ERROR;
        }

        usleep(50000);      // sleep for 1 system monitor tact
        TRACE("%d bytes were sent to the remote peer %u", sendlen, sock);
    }
    else
    {
        TRACE_F("Invalid socket");
        return NET_SOCKET_ERROR;
    }

    return NET_SUCCESS;
}

NetResult FrokAgent::StopNetworkServer()
{
    NetResult res = NET_SUCCESS;

    if(localSock != INVALID_SOCKET)
    {
        TRACE("Calling socket shutdown. localSock = %i", localSock);
        if(-1 == shutdown(localSock, 2))      // 2 = Both reception and transmission
        {
            TRACE_F("Failed to shutdown local socket with error = %s", strerror(errno));
            res = NET_SOCKET_ERROR;
        }
        TRACE_S("socket shutdown succeed");

        TRACE("Closing socket descriptor. localSock = %i", localSock);
        if(-1 == close(localSock))
        {
            TRACE_F("Failed to close socket descriptor on error %s", strerror(errno));
            res = NET_SOCKET_ERROR;
        }
        TRACE_S("Descriptor successfully closed");
    }

    localSock = INVALID_SOCKET;

    return res;
}
