#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <list>

#include "FaceServer.h"

#define MODULE_NAME     "SERVER"

pthread_mutex_t             faceServer_trace_cs;
pthread_mutex_t             faceServer_cs;
FaceServer::FaceServer(std::vector<AgentInfo*> &agentsInfo, unsigned short localPort)
{
    for(std::vector<AgentInfo*>::const_iterator it = agentsInfo.begin(); it != agentsInfo.end(); ++it)
    {
        agents.push_back(new FaceAgentConnector(*(AgentInfo*)*it));
    }

    localPortNumber = localPort;

    pthread_mutexattr_t mAttr;
    pthread_mutexattr_init(&mAttr);
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&faceServer_cs, &mAttr);
    pthread_mutex_init(&faceServer_trace_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);

    threadAcceptConnection = new CommonThread;
    localSock = INVALID_SOCKET;
    TRACE("new FaceServer");
}

FaceServer::~FaceServer()
{
    shutdown(localSock, 2);
    close(localSock);

    for(std::vector<CommonThread*>::iterator it = threadVecSocketListener.begin(); it != threadVecSocketListener.end(); ++it)
    {
        ((CommonThread*)(*it))->stopThread();
        delete ((CommonThread*)(*it));
    }
    threadVecSocketListener.clear();

    threadAcceptConnection->stopThread();
    delete threadAcceptConnection;

    pthread_mutex_destroy(&faceServer_cs);
    pthread_mutex_destroy(&faceServer_trace_cs);

    TRACE("~FaceServer");
}

bool FaceServer::StartFaceServer()
{
    if(!InitFaceCommonLib())
    {
        return false;
    }

    for(std::vector<FaceAgentConnector*>::const_iterator it = agents.begin(); it != agents.end(); ++it)
    {
        FaceAgentConnector *agent = (FaceAgentConnector*)*it;
        TRACE("Calling ConnectToAgent");
        if(!agent->ConnectToAgent())
        {
            TRACE_W("Failed to connect to agent");
            continue;
        }
        TRACE_S("Agent connected!");
    }
    TRACE("Calling StartNetworkServer");
    if(NET_SUCCESS != StartNetworkServer())
    {
        TRACE_F("StartNetworkServer Failed");
        return false;
    }
    TRACE_S("StartNetworkServer succeed!");
    return true;
}

bool FaceServer::StopFaceServer()
{
    bool success = true;

    for(std::vector<FaceAgentConnector*>::const_iterator it = agents.begin(); it != agents.end(); ++it)
    {
        FaceAgentConnector *agent = (FaceAgentConnector*)*it;
        if(!agent->DisconnectFromAgent())
        {
            success = false;
        }
    }

    if(NET_SUCCESS != StopNetworkServer())
    {
        success = false;
    }

    if(!DeinitFaceCommonLib())
    {
        success = false;
    }

    return success;
}

NetResult FaceServer::StopNetworkServer()
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

    for(std::vector<CommonThread*>::iterator it = threadVecSocketListener.begin(); it != threadVecSocketListener.end(); ++it)
    {
        ((CommonThread*)(*it))->stopThread();
        delete ((CommonThread*)(*it));
    }
    threadVecSocketListener.clear();

    TRACE("Calling threadServerListener->stopThread");
    if(!threadAcceptConnection->stopThread())
    {
        TRACE_F("threadServerListener->stopThread failed");
        res = NET_COMMON_THREAD_ERROR;
    }
    TRACE_S("threadServerListener->stopThread succeed");

    return res;
}

void FaceServer::AcceptConnection(void* param)
{
    FaceServer             *pThis = NULL;
    memcpy(&pThis, param, sizeof(FaceServer*));

    SOCKET                  accepted_socket;

    StructSocketListenerData socketListenerData;
    socketListenerData.pThis = &pThis;
    unsigned socketListenerDataLength = sizeof(FaceServer*) + sizeof(SOCKET) + sizeof(CommonThread*);

    TRACE("Accepting all incoming connections for socket %i", pThis->localSock);

    for(;;)
    {
        if(accepted_socket != INVALID_SOCKET)
        {
            shutdown(accepted_socket, 2);
            close(accepted_socket);
        }
        if ((accepted_socket = accept(pThis->localSock, NULL, NULL)) == SOCKET_ERROR)
        {
            if(pThis->threadAcceptConnection->isStopThreadReceived())
            {
                TRACE("terminate thread sema received");
                break;
            }
            continue;
        }

        TRACE_S("Incoming connection accepted. Accepted socket = %u", accepted_socket);

        int flag = 1;   //TRUE
        if(0 != setsockopt(accepted_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)))
        {
            TRACE_F("setsockopt (TCP_NODELAY) failed on error %s", strerror(errno));
            continue;
        }

        if(0 != setsockopt(accepted_socket, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(int)))
        {
            TRACE_F("setsockopt (SO_REUSEADDR) failed on error %s", strerror(errno));
            continue;
        }

        if(0 != setsockopt(accepted_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)))
        {
            TRACE_F("setsockopt (SO_REUSEADDR) failed on error %s", strerror(errno));
            continue;
        }

        socketListenerData.listenedSocket = accepted_socket;

        CommonThread *thread = new CommonThread;
        if(!thread->startThread((void*(*)(void*))FaceServer::SocketListener, &socketListenerData, socketListenerDataLength))
        {
            TRACE_F("Failed to start SocketListener thread. See CommonThread logs for information");
            continue;
        }

        pThis->threadVecSocketListener.push_back(thread);
    }

    shutdown(accepted_socket, 2);
    close(accepted_socket);

    shutdown(pThis->localSock, 2);
    close(pThis->localSock);
    TRACE("AcceptConnection finished");
    return;
}

void FaceServer::SocketListener(void* param)
{
    SocketListenerData     *psParam = (SocketListenerData*)param;
    FaceServer *pThis = NULL;
    memcpy(&pThis, psParam->pThis, sizeof(FaceServer*));

    int         dataLength;
    char        data[MAX_SOCKET_BUFF_SIZE];

    TRACE("start listening to socket %i", psParam->listenedSocket);

    for(;;)
    {
        if(psParam->thread->isStopThreadReceived())
        {
            TRACE("terminate thread sema received");
            break;
        }

        if( -1 == (dataLength = recv(psParam->listenedSocket, data, sizeof(data), MSG_DONTWAIT)))
        {
            if((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                // Valid errors - continue
                continue;
            }
            // unspecified error occured
            shutdown(psParam->listenedSocket, 2);
            close(psParam->listenedSocket);
            TRACE_F("recv failed on error %s", strerror(errno));
            TRACE_W("SocketListener shutdown");
            return;
        }

        if(dataLength == 0)
        {
            TRACE("Half disconnection observed. Terminating...");
            // TCP keep alivelocalSock
            break;
        }

        TRACE("Received %d bytes from the socket %u", dataLength, psParam->listenedSocket);
    }

    if(psParam->listenedSocket != INVALID_SOCKET)
    {
        shutdown(psParam->listenedSocket, 2);
        close(psParam->listenedSocket);
    }

    TRACE("SocketListener finished");
    return;
}

NetResult FaceServer::SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize)
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
        TRACE("Invalid socket");
        return NET_SOCKET_ERROR;
    }

    return NET_SUCCESS;
}

NetResult FaceServer::StartNetworkServer()
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

    FaceServer *pThis = this;

    TRACE("Starting ServerListener");

    if(!threadAcceptConnection->startThread((void*(*)(void*))FaceServer::AcceptConnection, &pThis, sizeof(FaceServer*)))
    {
        TRACE_F("Failed to start ServerListener thread. See CommonThread logs for information");
        shutdown(localSock, 2);
        close(localSock);
        return NET_COMMON_THREAD_ERROR;
    }

    TRACE_S("Succeed, socket = %d, port = %d", localSock, localPortNumber);

    return NET_SUCCESS;
}
