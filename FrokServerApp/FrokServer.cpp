/*#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <list>
#include "FrokServer.h"

#define MODULE_NAME     "SERVER"

pthread_mutex_t             frokServer_trace_cs;
pthread_mutex_t             frokServer_cs;
FrokServer::FrokServer(std::vector<AgentInfo*> &agentsInfo, unsigned short localPort)
{
    for(std::vector<AgentInfo*>::const_iterator it = agentsInfo.begin(); it != agentsInfo.end(); ++it)
    {
        agents.push_back(new FrokAgentConnector(*(AgentInfo*)*it));
    }

    localPortNumber = localPort;

    int res = 0;
    pthread_mutexattr_t mAttr;
    if (0 != (res = pthread_mutexattr_init(&mAttr)))
    {
        TRACE_F("pthread_mutexattr_init failed on error %s", strerror(res));
        throw NET_MEM_ALLOCATION_FAIL;
    }
    if (0 != (res = pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP)))
    {
        TRACE_F("pthread_mutexattr_settype failed on error %s", strerror(res));
        throw NET_MEM_ALLOCATION_FAIL;
    }
    if (0 != (res = pthread_mutex_init(&frokServer_cs, &mAttr)))
    {
        TRACE_F("pthread_mutex_init failed on error %s", strerror(res));
        throw NET_MEM_ALLOCATION_FAIL;
    }
    if (0 != (res = pthread_mutex_init(&frokServer_trace_cs, &mAttr)))
    {
        TRACE_F("pthread_mutex_init failed on error %s", strerror(res));
        throw NET_MEM_ALLOCATION_FAIL;
    }
    if (0 != (res = pthread_mutexattr_destroy(&mAttr)))
    {
        TRACE_F("pthread_mutexattr_destroy failed on error %s", strerror(res));
        throw NET_MEM_ALLOCATION_FAIL;
    }

    localSock = INVALID_SOCKET;
    TRACE("new FrokServer");
}

FrokServer::~FrokServer()
{
    shutdown(localSock, 2);
    close(localSock);

    for(std::vector<CommonThread*>::iterator it = threadVecSocketListener.begin(); it != threadVecSocketListener.end(); ++it)
    {
        ((CommonThread*)(*it))->stopThread();
        delete ((CommonThread*)(*it));
    }
    threadVecSocketListener.clear();

    pthread_mutex_destroy(&frokServer_cs);
    pthread_mutex_destroy(&frokServer_trace_cs);

    TRACE("~FrokServer");
}

bool FrokServer::StartFrokServer()
{
    if(!InitFaceCommonLib())
    {
        return false;
    }

    for(std::vector<FrokAgentConnector*>::const_iterator it = agents.begin(); it != agents.end(); ++it)
    {
        FrokAgentConnector *agent = (FrokAgentConnector*)*it;
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

    AcceptConnection();
    return true;
}

bool FrokServer::StopFrokServer()
{
    bool success = true;

    for(std::vector<FrokAgentConnector*>::const_iterator it = agents.begin(); it != agents.end(); ++it)
    {
        FrokAgentConnector *agent = (FrokAgentConnector*)*it;
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

NetResult FrokServer::StopNetworkServer()
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

        localSock = INVALID_SOCKET;
    }

    for(std::vector<CommonThread*>::iterator it = threadVecSocketListener.begin(); it != threadVecSocketListener.end(); ++it)
    {
        ((CommonThread*)(*it))->stopThread();
        delete ((CommonThread*)(*it));
    }
    threadVecSocketListener.clear();

    return res;
}

void FrokServer::AcceptConnection()
{
    SOCKET                  accepted_socket;

    TRACE("Accepting all incoming connections for socket %i", localSock);

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

        FrokServer *pThis = this;
        CommonThread *thread = new CommonThread;

        if(!thread->startThread((void*(*)(void*))FrokServer::SocketListener, &accepted_socket, sizeof(accepted_socket), &pThis))
        {
            TRACE_F("Failed to start SocketListener thread. See CommonThread logs for information");
            delete thread;
            continue;
        }

        threadVecSocketListener.push_back(thread);
    }

    shutdown(accepted_socket, 2);
    close(accepted_socket);

    shutdown(localSock, 2);
    close(localSock);
    TRACE("AcceptConnection finished");
    return;
}

void FrokServer::SocketListener(void* param)
{
    ThreadFunctionParameters *thParams = (ThreadFunctionParameters*)param;
    int listenedSocket = *(int*)thParams->params;
    FrokServer *pThis = (FrokServer*)thParams->object;

    UNREFERENCED_PARAMETER(pThis);

    int         dataLength;
    char        data[MAX_SOCKET_BUFF_SIZE];

    TRACE("start listening to socket %i", listenedSocket);

    for(;;)
    {
        if(thParams->thread->isStopThreadReceived())
        {
            TRACE("terminate thread sema received");
            break;
        }

        if( -1 == (dataLength = recv(listenedSocket, data, sizeof(data), MSG_DONTWAIT)))
        {
            if((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                // Valid errors - continue
                continue;
            }
            // unspecified error occured
            shutdown(listenedSocket, 2);
            close(listenedSocket);
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

        TRACE("Received %d bytes from the socket %u", dataLength, listenedSocket);

        json::Object requestJson;
        try
        {
            requestJson = json::Deserialize(data);
        }
        catch(...)
        {
            TRACE_F("Invalid input json %s", (char*)data);
            continue;
        }
        requestJson["reply_sock"] = listenedSocket;

        std::string requestString;
        try
        {
            requestString = json::Serialize(requestJson);
        }
        catch(...)
        {
            TRACE_F("Failed to serialize request json");
            continue;
        }

        while(1)
        {
            bool success = false;
            TRACE("Scanning registered agents");
            for(std::vector<FrokAgentConnector*>::iterator agent = pThis->agents.begin(); agent != pThis->agents.end(); ++agent)
            {
                AgentState state = ((FrokAgentConnector*)*agent)->GetAgentState();
                TRACE("\t state = %s", AgentStateToString(state));
                if((state == FROK_AGENT_FREE) && (success == false))
                {

                    if(((FrokAgentConnector*)*agent)->SendCommand(requestString))
                    {
                        success = true;
                        break;
                    }
                }
            }

            if(success == false)
            {
                for(std::vector<FrokAgentConnector*>::iterator agent = pThis->agents.begin(); agent != pThis->agents.end(); ++agent)
                {
                    if(((FrokAgentConnector*)*agent)->GetAgentState() != FROK_AGENT_BUSY)
                    {
                        TRACE_W("Reconnecting to agent");
                        ((FrokAgentConnector*)*agent)->DisconnectFromAgent();
                        ((FrokAgentConnector*)*agent)->ConnectToAgent();
                    }
                }
            }
            break;
        }
    }

    if(listenedSocket != INVALID_SOCKET)
    {
        shutdown(listenedSocket, 2);
        close(listenedSocket);
    }

    TRACE("SocketListener finished");
    return;
}

NetResult FrokServer::SendData(SOCKET sock, const char* pBuffer, unsigned uBufferSize)
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

NetResult FrokServer::StartNetworkServer()
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
*/
