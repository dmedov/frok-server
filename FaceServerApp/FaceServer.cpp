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
    pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&faceServer_cs, &mAttr);
    pthread_mutex_init(&faceServer_trace_cs, &mAttr);
    pthread_mutexattr_destroy(&mAttr);

    threadAcceptConnection = new CommonThread;
    localSock = INVALID_SOCKET;
}

FaceServer::~FaceServer()
{
    shutdown(localSock, 2);

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

    close(localSock);
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
        FACE_SERVER_TRACE(StartFaceServer, "Calling ConnectToAgent");
        if(!agent->ConnectToAgent())
        {
            FACE_SERVER_TRACE(StartFaceServer, "Failed to connect to agent");
            continue;
        }
        FACE_SERVER_TRACE(StartFaceServer, "Agent connected!");
    }
    FACE_SERVER_TRACE(StartFaceServer, "Calling StartNetworkServer");
    if(NET_SUCCESS != StartNetworkServer())
    {
        FACE_SERVER_TRACE(StartFaceServer, "StartNetworkServer succeed");
        return false;
    }
    FACE_SERVER_TRACE(StartFaceServer, "StartNetworkServer failed");
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

    FACE_SERVER_TRACE(StopNetworkServer, "Calling socket shutdown. localSock = %i", localSock);
    if(-1 == shutdown(localSock, 2))      // 2 = Both reception and transmission
    {
        FACE_SERVER_TRACE(StopNetworkServer, "Failed to shutdown local socket with error = %s", strerror(errno));
        res = NET_SOCKET_ERROR;
    }
    FACE_SERVER_TRACE(StopNetworkServer, "socket shutdown succeed");

    for(std::vector<CommonThread*>::iterator it = threadVecSocketListener.begin(); it != threadVecSocketListener.end(); ++it)
    {
        ((CommonThread*)(*it))->stopThread();
        delete ((CommonThread*)(*it));
    }
    threadVecSocketListener.clear();

    FACE_SERVER_TRACE(StopNetworkServer, "Calling threadServerListener->stopThread");
    if(!threadAcceptConnection->stopThread())
    {
        FACE_SERVER_TRACE(StopNetworkServer, "threadServerListener->stopThread failed");
        res = NET_COMMON_THREAD_ERROR;
    }
    FACE_SERVER_TRACE(StopNetworkServer, "threadServerListener->stopThread succeed");

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

    FACE_SERVER_TRACE(AcceptConnection, "Accepting all incoming connections for socket %i", pThis->localSock);

    for(;;)
    {
        if ((accepted_socket = accept(pThis->localSock, NULL, NULL)) == SOCKET_ERROR)
        {
            if(pThis->threadAcceptConnection->isStopThreadReceived())
            {
                FACE_SERVER_TRACE(AcceptConnection, "terminate thread sema received");
                break;
            }
            continue;
        }

        FACE_SERVER_TRACE(AcceptConnection, "Incoming connection accepted. Accepted socket = %u", accepted_socket);

        int flag = 1;   //TRUE
        if(0 != setsockopt(accepted_socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int)))
        {
            FACE_SERVER_TRACE(AcceptConnection, "setsockopt (TCP_NODELAY) failed on error %s", strerror(errno));
            continue;
        }

        if(0 != setsockopt(accepted_socket, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(int)))
        {
            FACE_SERVER_TRACE(AcceptConnection, "setsockopt (SO_REUSEADDR) failed on error %s", strerror(errno));
            continue;
        }

        if(0 != setsockopt(accepted_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)))
        {
            FACE_SERVER_TRACE(AcceptConnection, "setsockopt (SO_REUSEADDR) failed on error %s", strerror(errno));
            continue;
        }

        socketListenerData.listenedSocket = accepted_socket;

        CommonThread *thread = new CommonThread;
        if(!thread->startThread((void*(*)(void*))FaceServer::SocketListener, &socketListenerData, socketListenerDataLength))
        {
            FACE_SERVER_TRACE(AcceptConnection, "Failed to start SocketListener thread. See CommonThread logs for information");
            continue;
        }

        pThis->threadVecSocketListener.push_back(thread);
    }

    shutdown(pThis->localSock, 2);
    FACE_SERVER_TRACE(AcceptConnection, "AcceptConnection finished");
    return;
}

void FaceServer::SocketListener(void* param)
{
    SocketListenerData     *psParam = (SocketListenerData*)param;
    FaceServer *pThis = NULL;
    memcpy(&pThis, psParam->pThis, sizeof(FaceServer*));

    int         dataLength;
    char        data[MAX_SOCKET_BUFF_SIZE];

    FACE_SERVER_TRACE(SocketListener, "start listening to socket %i", psParam->listenedSocket);

    for(;;)
    {
        if(psParam->thread->isStopThreadReceived())
        {
            FACE_SERVER_TRACE(SocketListener, "terminate thread sema received");
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
            FACE_SERVER_TRACE(SocketListener, "recv failed on error %s", strerror(errno));
            FACE_SERVER_TRACE(SocketListener, "SocketListener shutdown");
            shutdown(psParam->listenedSocket, 2);
            close(psParam->listenedSocket);
            return;
        }

        if(dataLength == 0)
        {
            // TCP keep alive
            continue;
        }

        FACE_SERVER_TRACE(SocketListener, "Received %d bytes from the socket %u", sCallbackData.dataLength, psParam->listenedSocket);
    }

    FACE_SERVER_TRACE(SocketListener, "SocketListener finished");
    return;
}

NetResult FaceServer::SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize)
{
    int sendlen = 0;

    if(sock != INVALID_SOCKET)
    {
        if (-1 == (sendlen = send(sock, pBuffer, uBufferSize, 0)))
        {
            FACE_SERVER_TRACE(SendData, "Failed to send outgoing bytes to the remote peer %u with error %s", sock, strerror(errno));
            return NET_SOCKET_ERROR;
        }

        usleep(50000);      // sleep for 1 system monitor tact
        FACE_SERVER_TRACE(SendData, "%d bytes were sent to the remote peer %u", sendlen, sock);
    }
    else
    {
        FACE_SERVER_TRACE(SendData, "Invalid socket");
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
        FACE_SERVER_TRACE(StartNetworkServer, "socket failed on error = %s", strerror(errno));
        return NET_SOCKET_ERROR;
    }

    //[TBD] think about SO_KEEPALIVE option

    int option = 1;
    if(0 != setsockopt(localSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        FACE_SERVER_TRACE(StartNetworkServer, "setsockopt failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        return NET_SOCKET_ERROR;
    }

    if(0 != setsockopt(localSock, SOL_SOCKET, SO_KEEPALIVE, &option, sizeof(option)))
    {
        FACE_SERVER_TRACE(StartNetworkServer, "setsockopt (SO_KEEPALIVE) failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        return NET_SOCKET_ERROR;
    }

    int         optval;
    socklen_t   optlen = sizeof(int);

    if(0 != getsockopt(localSock, SOL_SOCKET,  SO_REUSEADDR, &optval, &optlen))
    {
        FACE_SERVER_TRACE(StartNetworkServer, "getsockopt failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        return NET_SOCKET_ERROR;
    }

    if(optval == 0)
    {
        FACE_SERVER_TRACE(StartNetworkServer, "setsockopt failed to enabled SO_REUSEADDR option");
        //return NET_SOCKET_ERROR;
    }

    if(0 != bind(localSock, (struct sockaddr*)&server, sizeof(server)))
    {
        FACE_SERVER_TRACE(StartNetworkServer, "bind failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        return NET_SOCKET_ERROR;
    }

    if (0 != listen(localSock, SOMAXCONN))
    {
        FACE_SERVER_TRACE(StartNetworkServer, "listen failed on error %s", strerror(errno));
        shutdown(localSock, 2);
        return NET_UNSPECIFIED_ERROR;
    }

    FaceServer *pThis = this;

    FACE_SERVER_TRACE(StartNetworkServer, "Starting ServerListener", strerror(errno));

    if(!threadAcceptConnection->startThread((void*(*)(void*))FaceServer::AcceptConnection, &pThis, sizeof(FaceServer*)))
    {
        FACE_SERVER_TRACE(StartNetworkServer, "Failed to start ServerListener thread. See CommonThread logs for information");
        return NET_COMMON_THREAD_ERROR;
    }

    FACE_SERVER_TRACE(StartNetworkServer, "Succeed, socket = %d, port = %d", localSock, localPortNumber);

    return NET_SUCCESS;
}
