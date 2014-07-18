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

#include "FaceAgent.h"

pthread_mutex_t             faceAgent_trace_cs;
pthread_mutex_t             faceAgent_cs;
FaceAgent::FaceAgent(unsigned short localPort, const char *photoBasePath, const char *targetsFolderPath)
{
    this->photoBasePath = new char[strlen(photoBasePath) + 1];
    this->targetsFolderPath = new char[strlen(targetsFolderPath) + 1];
    strcpy(this->photoBasePath, photoBasePath);
    strcpy(this->photoBasePath, photoBasePath);

    pthread_mutexattr_t mAttr;
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&faceAgent_trace_cs, &mAttr);
    pthread_mutex_init(&faceAgent_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);

    threadServerListener = new CommonThread;
    localSock = INVALID_SOCKET;

    this->localPortNumber = localPort;
    FACE_AGENT_TRACE(FaceAgent, "new FaceAgent");
}

FaceAgent::~FaceAgent()
{
    delete []photoBasePath;
    delete []targetsFolderPath;

    if(localSock != INVALID_SOCKET)
    {
        shutdown(localSock, 2);
        close(localSock);
    }
    threadServerListener->stopThread();
    delete threadServerListener;

    pthread_mutex_destroy(&faceAgent_cs);
    pthread_mutex_destroy(&faceAgent_trace_cs);
    FACE_AGENT_TRACE(~FaceAgent, "~FaceAgent");
}

bool FaceAgent::StartFaceAgent()
{
    FACE_AGENT_TRACE(StartFaceAgent, "Calling InitFaceCommonLib");
    if(!InitFaceCommonLib("FaceAgent.log"))
    {
        FACE_AGENT_TRACE(StartFaceAgent, "InitFaceCommonLib failed");
        return false;
    }
    FACE_AGENT_TRACE(StartFaceAgent, "InitFaceCommonLib succeed");
    FACE_AGENT_TRACE(StartFaceAgent, "Calling StartNetworkServer");
    NetResult res;
    if(NET_SUCCESS != (res = StartNetworkServer()))
    {
        FACE_AGENT_TRACE(StartFaceAgent, "StartNetworkServer failed on error %x", res);
        return false;
    }
    FACE_AGENT_TRACE(StartFaceAgent, "StartNetworkServer succeed");

    return true;
}

NetResult FaceAgent::StartNetworkServer()
{
    sockaddr_in             server;
    memset(&server, 0, sizeof(server));

    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(localPortNumber);

    if ((localSock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == INVALID_SOCKET)
    {
        FACE_AGENT_TRACE(StartNetworkServer, "socket failed on error = %s", strerror(errno));
        return NET_SOCKET_ERROR;
    }

    //[TBD] think about SO_KEEPALIVE option

    int option = 1;
    if(0 != setsockopt(localSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        FACE_AGENT_TRACE(StartNetworkServer, "setsockopt failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    if(0 != setsockopt(localSock, SOL_SOCKET, SO_KEEPALIVE, &option, sizeof(option)))
    {
        FACE_AGENT_TRACE(StartNetworkServer, "setsockopt (SO_KEEPALIVE) failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    int         optval;
    socklen_t   optlen = sizeof(int);

    if(0 != getsockopt(localSock, SOL_SOCKET,  SO_REUSEADDR, &optval, &optlen))
    {
        FACE_AGENT_TRACE(StartNetworkServer, "getsockopt failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    if(optval == 0)
    {
        FACE_AGENT_TRACE(StartNetworkServer, "setsockopt failed to enabled SO_REUSEADDR option");
        //return NET_SOCKET_ERROR;
    }

    if(0 != bind(localSock, (struct sockaddr*)&server, sizeof(server)))
    {
        FACE_AGENT_TRACE(StartNetworkServer, "bind failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        close(localSock);
        return NET_SOCKET_ERROR;
    }

    if (0 != listen(localSock, SOMAXCONN))
    {
        FACE_AGENT_TRACE(StartNetworkServer, "listen failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        close(localSock);
        return NET_UNSPECIFIED_ERROR;
    }

    FaceAgent *pThis = this;

    FACE_AGENT_TRACE(StartNetworkServer, "Starting ServerListener");

    if(!threadServerListener->startThread((void*(*)(void*))FaceAgent::ServerListener, &pThis, sizeof(FaceAgent*)))
    {
        FACE_AGENT_TRACE(StartNetworkServer, "Failed to start ServerListener thread. See CommonThread logs for information");
        return NET_COMMON_THREAD_ERROR;
    }

    FACE_AGENT_TRACE(StartNetworkServer, "Succeed, socket = %d, port = %d", localSock, localPortNumber);

    return NET_SUCCESS;
}

bool FaceAgent::StopFaceAgent()
{
    bool success = true;

    NetResult res;

    FACE_AGENT_TRACE(StopFaceAgent, "Calling StopNetworkServer");
    if(NET_SUCCESS != (res = StopNetworkServer()))
    {
        FACE_AGENT_TRACE(StopFaceAgent, "StopNetworkServer failed on result %x. Continue...", res);
        success = false;
    }
    FACE_AGENT_TRACE(StopFaceAgent, "StopNetworkServer succeed");
    FACE_AGENT_TRACE(StopFaceAgent, "Calling DeinitFaceCommonLib");
    if(!DeinitFaceCommonLib())
    {
        FACE_AGENT_TRACE(StopFaceAgent, "DeinitFaceCommonLib failed");
        success = false;
    }
    FACE_AGENT_TRACE(StopFaceAgent, "DeinitFaceCommonLib succeed");

    return success;
}

void FaceAgent::ServerListener(void* param)
{
    FaceAgent                  *pThis                       = NULL;
    SOCKET                      accepted_socket             = INVALID_SOCKET;
    int                         dataLength                  = 0;
    char                        data[MAX_SOCKET_BUFF_SIZE]  = {0};
    std::vector<std::string>    mandatoryKeys;

    memcpy(&pThis, param, sizeof(FaceAgent*));

    mandatoryKeys.push_back("cmd");
    mandatoryKeys.push_back("req_id");
    mandatoryKeys.push_back("reply_sock");

    FACE_AGENT_TRACE(ServerListener, "Accepting single incoming connections for socket %i", pThis->localSock);

    for(;;)
    {
        close(accepted_socket);
        if ((accepted_socket = accept(pThis->localSock, NULL, NULL)) == SOCKET_ERROR)
        {
            if(pThis->threadServerListener->isStopThreadReceived())
            {
                FACE_AGENT_TRACE(ServerListener, "terminate thread sema received");
                shutdown(pThis->localSock, 2);
                close(pThis->localSock);
                FACE_AGENT_TRACE(ServerListener, "AcceptConnection finished");
                return;
            }
            continue;
        }

        FACE_AGENT_TRACE(AcceptConnection, "Incoming connection accepted. Accepted socket = %u", accepted_socket);

        int flag = 1;   //TRUE
        if(0 != setsockopt(accepted_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)))
        {
            FACE_AGENT_TRACE(AcceptConnection, "setsockopt (TCP_NODELAY) failed on error %s", strerror(errno));
            FACE_AGENT_TRACE(ServerListener, "Accepting single incoming connections for socket %i", pThis->localSock);
            continue;
        }

        if(0 != setsockopt(accepted_socket, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(int)))
        {
            FACE_AGENT_TRACE(AcceptConnection, "setsockopt (SO_KEEPALIVE) failed on error %s", strerror(errno));
            FACE_AGENT_TRACE(ServerListener, "Accepting single incoming connections for socket %i", pThis->localSock);
            continue;
        }

        if(0 != setsockopt(accepted_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)))
        {
            FACE_AGENT_TRACE(AcceptConnection, "setsockopt (SO_REUSEADDR) failed on error %s", strerror(errno));
            FACE_AGENT_TRACE(ServerListener, "Accepting single incoming connections for socket %i", pThis->localSock);
            continue;
        }

        FACE_AGENT_TRACE(ServerListener, "Socket listener cycle started. The agent will process one request and then connection would be terminated.");
        for(;;)
        {
            if(pThis->threadServerListener->isStopThreadReceived())
            {
                FACE_AGENT_TRACE(ServerListener, "terminate thread sema received");
                FACE_AGENT_TRACE(ServerListener, "Accepting single incoming connections for socket %i", pThis->localSock);
                shutdown(pThis->localSock, 2);
                close(pThis->localSock);
                FACE_AGENT_TRACE(ServerListener, "AcceptConnection finished");
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
                FACE_AGENT_TRACE(ServerListener, "recv failed on error %s", strerror(errno));
                FACE_AGENT_TRACE(ServerListener, "SocketListener shutdown");
                shutdown(accepted_socket, 2);
                close(accepted_socket);
                FACE_AGENT_TRACE(ServerListener, "Accepting single incoming connections for socket %i", pThis->localSock);
                break;
            }

            if(dataLength == 0)
            {
                // TCP keep alive
                continue;
            }

            FACE_AGENT_TRACE(ServerListener, "Received %d bytes from the server(%u)", dataLength, accepted_socket);

            // Response from agent received
            json::Object requestJson;
            try
            {
                requestJson = ((json::Value)json::Deserialize((std::string)data)).ToObject();
            }
            catch (...)
            {
                FACE_AGENT_TRACE(ServerListener, "Failed to parse incoming JSON: %s", data);
                FACE_AGENT_TRACE(ServerListener, "Accepting single incoming connections for socket %i", pThis->localSock);
                shutdown(accepted_socket, 2);
                close(accepted_socket);
                break;
            }

            if (!(requestJson.HasKeys(mandatoryKeys)))
            {
                FACE_AGENT_TRACE(ServerListener, "Invalid input JSON: no cmd field (%s)", data);
                FACE_AGENT_TRACE(ServerListener, "Accepting single incoming connections for socket %i", pThis->localSock);
                shutdown(accepted_socket, 2);
                close(accepted_socket);
                break;
            }

            requestJson["result"] = "success";

            std::string outJson = json::Serialize(requestJson);

            FACE_AGENT_TRACE(ServerListener, "Simply echo with success result %s", outJson.c_str());

            if(NET_SUCCESS != pThis->SendData(accepted_socket, outJson.c_str(), outJson.size()))
            {
                FACE_AGENT_TRACE(ServerListener, "Failed to send response to server (%u). Reponse = %s", accepted_socket, outJson.c_str());
                shutdown(accepted_socket, 2);
                close(accepted_socket);
                FACE_AGENT_TRACE(ServerListener, "Accepting single incoming connections for socket %i", pThis->localSock);
                break;
            }
            shutdown(accepted_socket, 2);
            close(accepted_socket);
            FACE_AGENT_TRACE(ServerListener, "Accepting single incoming connections for socket %i", pThis->localSock);
            break;
        }
    }
    shutdown(pThis->localSock, 2);
    close(pThis->localSock);
    FACE_AGENT_TRACE(ServerListener, "AcceptConnection finished");
    return;
}

NetResult FaceAgent::SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize)
{
    int sendlen = 0;

    if(sock != INVALID_SOCKET)
    {
        if (-1 == (sendlen = send(sock, pBuffer, uBufferSize, 0)))
        {
            FACE_AGENT_TRACE(SendData, "Failed to send outgoing bytes to the remote peer %u with error %s", sock, strerror(errno));
            return NET_SOCKET_ERROR;
        }

        usleep(50000);      // sleep for 1 system monitor tact
        FACE_AGENT_TRACE(SendData, "%d bytes were sent to the remote peer %u", sendlen, sock);
    }
    else
    {
        FACE_AGENT_TRACE(SendData, "Invalid socket");
        return NET_SOCKET_ERROR;
    }

    return NET_SUCCESS;
}

NetResult FaceAgent::StopNetworkServer()
{
    NetResult res = NET_SUCCESS;

    if(localSock != INVALID_SOCKET)
    {
        FACE_AGENT_TRACE(StopNetworkServer, "Calling socket shutdown. localSock = %i", localSock);
        if(-1 == shutdown(localSock, 2))      // 2 = Both reception and transmission
        {
            FACE_AGENT_TRACE(StopNetworkServer, "Failed to shutdown local socket with error = %s", strerror(errno));
            res = NET_SOCKET_ERROR;
        }
        FACE_AGENT_TRACE(StopNetworkServer, "socket shutdown succeed");

        FACE_AGENT_TRACE(StopNetworkServer, "Closing socket descriptor. localSock = %i", localSock);
        if(-1 == close(localSock))
        {
            FACE_AGENT_TRACE(StopNetworkServer, "Failed to close socket descriptor on error", strerror(errno));
            res = NET_SOCKET_ERROR;
        }
        FACE_AGENT_TRACE(StopNetworkServer, "Descriptor successfully closed");
    }

    FACE_AGENT_TRACE(StopNetworkServer, "Calling threadServerListener->stopThread");
    if(!threadServerListener->stopThread())
    {
        FACE_AGENT_TRACE(StopNetworkServer, "threadServerListener->stopThread failed");
        res = NET_COMMON_THREAD_ERROR;
    }
    FACE_AGENT_TRACE(StopNetworkServer, "threadServerListener->stopThread succeed");

    return res;
}
