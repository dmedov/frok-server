#include "FrokAgent.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>

#define MODULE_NAME     "AGENT"

static FrokAgentContext *context = NULL;
static pthread_mutex_t frokAgentMutex = PTHREAD_MUTEX_INITIALIZER;

// private functions declaration
FrokResult frokAgentSocketListener();
FrokResult frokAgentSendData(SOCKET sock, const char* pBuffer, size_t uBufferSize);
void frokAgentProcessJson(SOCKET outSock, const char *json, size_t UNUSED(jsonLength));
BOOL isFullPacketReceived(char *packet, size_t packetSize, size_t *realPacketLength);
BOOL isPacketValid(char *packet, size_t packetSize);

// API implementation
FrokResult frokAgentInit(unsigned short port, void *detector, void *recognizer, const char *photoBaseFolderPath, const char *targetsFolderPath)
{
    int error, option = TRUE;

    TRACE_S("started");

    if(context != NULL)
    {
        TRACE_F("Already inited");
        return FROK_RESULT_INVALID_STATE;
    }

    context = malloc(sizeof(FrokAgentContext));
    if(!context)
    {
        TRACE_F("malloc failed on error %s", strerror(errno));
        return FROK_RESULT_MEMORY_FAULT;
    }

    TRACE_N("lock frokAgentMutex");
    if(-1 == (error = pthread_mutex_lock(&frokAgentMutex)))
    {
        TRACE_F("pthread_mutex_lock failed on error %s", strerror(error));
        free(context);
        context = NULL;
        return FROK_RESULT_LINUX_ERROR;
    }

    // Create local socket
    TRACE_N("Create new local TCP socket");
    if (-1 == (context->localSock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)))
    {
        TRACE_F("socket failed on error = %s", strerror(errno));
        free(context);
        context = NULL;
        pthread_mutex_unlock(&frokAgentMutex);
        return FROK_RESULT_SOCKET_ERROR;
    }

    // Enable SO_REUSEADDR option - if socket was already binded to requested address - it will be deleted and new socket will bind.
    TRACE_N("Set SO_REUSEADDR to local socket");
    if(0 != setsockopt(context->localSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)))
    {
        TRACE_F("setsockopt (SO_REUSEADDR) failed on error %s", strerror(errno));
        close(context->localSock);
        free(context);
        context = NULL;
        pthread_mutex_unlock(&frokAgentMutex);
        return FROK_RESULT_SOCKET_ERROR;
    }

    // Google SO_KEEPALIVE
    // I'm not sure do we need it here. It may cause lack of EOF in socket fd stream
    /*TRACE_N("Set SO_KEEPALIVE to local socket");
    if(0 != setsockopt(context->localSock, SOL_SOCKET, SO_KEEPALIVE, &option, sizeof(option)))
    {
        TRACE_F("setsockopt (SO_KEEPALIVE) failed on error %s", strerror(errno));
        close(context->localSock);
        free(context);
        context = NULL;
        pthread_mutex_unlock(&frokAgentMutex);
        return FROK_RESULT_SOCKET_ERROR;
    }*/

    context->localPortNumber = port;

    // Create semaphore FD

    TRACE_N("Create semaphore fd");
    if(-1 == (context->terminateAgentEvent = eventfd(0, EFD_SEMAPHORE)))
    {
        TRACE_F("eventfd failed on error %s", strerror(errno));
        close(context->localSock);
        free(context);
        context = NULL;
        pthread_mutex_unlock(&frokAgentMutex);
        return FROK_RESULT_LINUX_ERROR;
    }

    TRACE_N("Alloc frokAPI instance");
    context->api = frokAPIAlloc(photoBaseFolderPath, targetsFolderPath, detector, recognizer);
    frokAPIInit(context->api);
    if(context->api == NULL)
    {
        TRACE_F("frokAPIAlloc failed");
        return FROK_RESULT_UNSPECIFIED_ERROR;
    }
    TRACE_N("unlock frokAgentMutex");
    if(-1 == (error = pthread_mutex_unlock(&frokAgentMutex)))
    {
        TRACE_F("pthread_mutex_unlock failed on error %s", strerror(error));
        close(context->localSock);
        free(context);
        context = NULL;
        pthread_mutex_unlock(&frokAgentMutex);
        return FROK_RESULT_LINUX_ERROR;
    }

    TRACE_S("finished");

    return FROK_RESULT_SUCCESS;
}

FrokResult frokAgentStart()
{
    int error;
    struct sockaddr_in server;

    TRACE_S("started");

    if(context == NULL)
    {
        TRACE_F("Not inited");
        return FROK_RESULT_INVALID_STATE;
    }

    TRACE_N("lock frokAgentMutex");
    if(-1 == (error = pthread_mutex_lock(&frokAgentMutex)))
    {
        TRACE_F("pthread_mutex_lock failed on error %s", strerror(error));
        return FROK_RESULT_LINUX_ERROR;
    }

    if(context->agentStarted == TRUE)
    {
        TRACE_F("Agent already started");
        pthread_mutex_unlock(&frokAgentMutex);
        return FROK_RESULT_INVALID_STATE;
    }

    context->agentStarted = TRUE;

    memset(&server, 0, sizeof(struct sockaddr_in));

    // Listen to all available IPv4 interfaces. Port = localPortNumber
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(context->localPortNumber);

    TRACE_N("bind local socket %d to all available ipV4 itnerfaces. Port is %d", context->localSock, context->localPortNumber);
    if(-1 == bind(context->localSock, (struct sockaddr*)&server, sizeof(struct sockaddr_in)))
    {
        TRACE_F("bind failed on error %s", strerror(errno));
        context->agentStarted = FALSE;
        pthread_mutex_unlock(&frokAgentMutex);
        return FROK_RESULT_SOCKET_ERROR;
    }

    TRACE_N("Listen to socket %d. Max pending connections is %d", context->localSock, SOMAXCONN);
    if (-1 == listen(context->localSock, SOMAXCONN))
    {
        TRACE_F("listen failed on error %s", strerror(errno));
        shutdown(context->localSock, 2);
        context->agentStarted = FALSE;
        pthread_mutex_unlock(&frokAgentMutex);
        return FROK_RESULT_SOCKET_ERROR;
    }

    TRACE_N("unlock frokAgentMutex");
    if(-1 == (error = pthread_mutex_unlock(&frokAgentMutex)))
    {
        TRACE_F("pthread_mutex_unlock failed on error %s", strerror(error));
        shutdown(context->localSock, 2);
        context->agentStarted = FALSE;
        pthread_mutex_unlock(&frokAgentMutex);
        return FROK_RESULT_LINUX_ERROR;
    }

    TRACE_S("finished");

    return frokAgentSocketListener();
}

FrokResult frokAgentSocketListener()
{
    SOCKET acceptedSocket;
    int result;
    struct pollfd fds[2];               // Network data received and terminate event fd
    char *incomingRequest = NULL;       // Request sent via network
    char *tmp_reallocPointer;
    size_t receivedDataSize = 0;
    size_t tmp_dataSize;
    size_t receivedPacketSize;
    BOOL invalidPacket = FALSE;

    TRACE_S("started");

    for(;;)
    {
        // Check that we are not in deinit or stop functions
        TRACE_N("lock-unlock frokAgentMutex");
        pthread_mutex_lock(&frokAgentMutex);
        pthread_mutex_unlock(&frokAgentMutex);
        TRACE_S("Accepting connections from socket %d, port is %d", context->localPortNumber, context->localSock);
        if(-1 == (acceptedSocket = accept(context->localSock, NULL, NULL)))
        {
            if((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                continue;
            }

            if(errno == EINVAL)
            {
                TRACE_S("Finished");
                return FROK_RESULT_SUCCESS;
            }
            TRACE_F("accept failed on error %s", strerror(errno));
            return FROK_RESULT_SOCKET_ERROR;
        }
        TRACE_S("Incoming connection received and accepted. Accepted socket = %d", acceptedSocket);

        // Waiting for any data from connected dude. If none data received for FROK_AGENT_CLIENT_DATA_TIMEOUT_MS milliseconds - disconnect
        TRACE_N("lock frokAgentMutex");
        if(-1 == (result = pthread_mutex_lock(&frokAgentMutex)))
        {
            TRACE_F("pthread_mutex_lock failed on error %s", strerror(result));
            shutdown(acceptedSocket, 2);
            close(acceptedSocket);
            return FROK_RESULT_LINUX_ERROR;
        }

        fds[0].fd = acceptedSocket;
        fds[0].events = POLLIN;

        fds[1].fd = context->terminateAgentEvent;
        fds[1].events = POLLIN;

        TRACE_N("unlock frokAgentMutex");
        if(-1 == (result = pthread_mutex_unlock(&frokAgentMutex)))
        {
            TRACE_F("pthread_mutex_unlock failed on error %s", strerror(result));
            shutdown(acceptedSocket, 2);
            close(acceptedSocket);
            pthread_mutex_unlock(&frokAgentMutex);
            return FROK_RESULT_LINUX_ERROR;
        }

        TRACE_N("Waiting for incoming data from socket %d", acceptedSocket);
        for(;;)
        {
            result = poll(fds, 2, FROK_AGENT_CLIENT_DATA_TIMEOUT_MS);
            if(-1 == result)
            {
                TRACE_F("poll failed on error %s", strerror(errno));
                return FROK_RESULT_LINUX_ERROR;
            }
            else if (0 == result)
            {
                TRACE_S("Timeout reached for reading socket %d. Disconnect", acceptedSocket);
                break;
            }

            // Check terminate event
            if(fds[1].revents & POLLIN)
            {
                TRACE_S("Terminate event received - terminating");
                TRACE_N("Shutdown client socket %d", acceptedSocket);
                if(-1 == shutdown(acceptedSocket, 2))
                {
                    TRACE_F("shutdown failed on error %s", strerror(errno));
                    return FROK_RESULT_SOCKET_ERROR;
                }

                TRACE_N("Closing client socket %d", acceptedSocket);
                if(-1 == close(acceptedSocket))
                {
                    TRACE_F("close failed on error %s", strerror(errno));
                    return FROK_RESULT_SOCKET_ERROR;
                }

                TRACE_S("finished");
                return FROK_RESULT_SUCCESS;
            }
            else if(fds[0].revents & POLLIN)
            {
                if(-1 == ioctl(acceptedSocket, FIONREAD, &tmp_dataSize))
                {
                    TRACE_F("ioctl failed on error %s", strerror(errno));
                    break;
                }

                if(tmp_dataSize == 0)
                {
                    TRACE_W("Half disconnection received. Disconnect");
                    break;
                }
                // Alloc memory for new data
                if(incomingRequest == NULL)
                {
                    incomingRequest = calloc(tmp_dataSize, 1);  // sizeof(char) == 1 due to c specification
                    if(!incomingRequest)
                    {
                        // new message
                        TRACE_F("calloc failed on error %s", strerror(errno));
                        shutdown(acceptedSocket, 2);
                        close(acceptedSocket);
                        return FROK_RESULT_MEMORY_FAULT;
                    }
                }
                else
                {
                    // append to existing message
                    tmp_reallocPointer = incomingRequest;
                    incomingRequest = realloc(tmp_reallocPointer, receivedDataSize + tmp_dataSize);
                    if(!incomingRequest)
                    {
                        TRACE_F("realloc failed on error %s", strerror(errno));
                        free(tmp_reallocPointer);
                        shutdown(acceptedSocket, 2);
                        close(acceptedSocket);
                        return FROK_RESULT_MEMORY_FAULT;
                    }
                    memset(incomingRequest + receivedDataSize, 0, tmp_dataSize);
                }

                // Read all available data
                while(tmp_dataSize != 0)
                {
                    if(-1 == (result = recv(acceptedSocket, incomingRequest + receivedDataSize, tmp_dataSize, 0)))
                    {
                        if((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
                        {
                            continue;
                        }
                        TRACE_F("recv failed on error %s", strerror(errno));
                        break;
                    }
                    tmp_dataSize -= result;
                    receivedDataSize += result;
                }

                // Verify that packet is valid. Valid packet starts with "{
                if(FALSE == isPacketValid(incomingRequest, receivedDataSize))
                {
                    TRACE_F("Invalid packet received: %s", incomingRequest);
                    break;
                }

                // Seems like packet is valid. If it isn't - we should wait for timeout

                // Verify that full packet received
                while(TRUE == isFullPacketReceived(incomingRequest, receivedDataSize, &receivedPacketSize))
                {
                    // Call functions

                    frokAgentProcessJson(acceptedSocket, incomingRequest, receivedPacketSize);
                    // Function call finished
                    // Copy tail from incoming request
                    if(receivedDataSize != receivedPacketSize)
                    {
                        tmp_reallocPointer = malloc(receivedDataSize - receivedPacketSize);
                        if(!tmp_reallocPointer)
                        {
                            TRACE_F("malloc failed on error %s", strerror(errno));
                            return FROK_RESULT_MEMORY_FAULT;
                        }
                        memcpy(tmp_reallocPointer, incomingRequest + receivedPacketSize, receivedDataSize - receivedPacketSize);
                        free(incomingRequest);
                        incomingRequest = tmp_reallocPointer;

                        receivedDataSize -= receivedPacketSize;

                        if(FALSE == isPacketValid(incomingRequest, receivedDataSize))
                        {
                            TRACE_F("Invalid packet received: %s", incomingRequest);
                            invalidPacket = TRUE;
                            break;
                        }
                    }
                    else
                    {
                        free(incomingRequest);
                        receivedDataSize = 0;
                    }

                }

                if(invalidPacket == TRUE)   // [TBD] This can be replaced with break label, or goto label
                {
                    break;
                }
            }
            else
            {
                // Something went wrong
                TRACE_F("Poll returned success while no events captured - report problem to D.Zotov");
                return FROK_RESULT_UNSPECIFIED_ERROR;
            }
        }

        TRACE_N("Shutdown client socket %d", acceptedSocket);
        if(-1 == shutdown(acceptedSocket, 2))
        {
            TRACE_W("shutdown socket %d failed on error %s. Continue...", acceptedSocket, strerror(errno));
        }

        TRACE_N("Close client socket %d", acceptedSocket);
        if(-1 == close(acceptedSocket))
        {
            TRACE_W("close socket %d failed on error %s. Continue...", acceptedSocket, strerror(errno));
        }

        free(incomingRequest);
        incomingRequest = NULL;

        receivedDataSize = 0;
    }
    TRACE_S("finished");
}

void frokAgentProcessJson(SOCKET outSock, const char *json, size_t UNUSED(jsonLength))
{
    char *functionName, *outJson;
    FrokResult res;

    TRACE_N("Processing json %s", json);
    TRACE_N("Calling getFunctionFromJson");
    functionName = getFunctionFromJson(json);
    if(functionName == NULL)
    {
        TRACE_F("getFunctionFromJson failed");
        return;
    }

    TRACE_N("Calling frokAPIExecuteFunction, functionName = %s", functionName);
    res = frokAPIExecuteFunction(context->api, functionName, json, &outJson);

    TRACE_N("Send response json %s to remote peer %d", outJson, outSock);

    if(FROK_RESULT_SUCCESS != (res = frokAgentSendData(outSock, outJson, strlen(outJson))))
    {
        TRACE_F("frokAgentSendData failed on error %s", FrokResultToString(res));
    }

    free(outJson);
    free(functionName);
}

// Full packet begins with "{ and ends with }". Number of { and } symbols has to be equal
BOOL isFullPacketReceived(char *packet, size_t packetSize, size_t *realPacketLength)
{
    size_t i;
    size_t entries = 0;

    if(realPacketLength == NULL)
    {
        TRACE_F("Invalid input data. realPacketLength = %p", realPacketLength);
        return FALSE;
    }

    // Not ehough data for any json packet
    if(packetSize < 2)
    {
        return FALSE;
    }

    for(i = 0; i < packetSize; i++)
    {
        if(packet[i] == '{')
        {
            entries++;
        }
        else if(packet[i] == '}')
        {
            entries--;
            if(entries == 0)
            {
                // Full packet is received;
                *realPacketLength = i + 1;
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL isPacketValid(char *packet, size_t packetSize)
{
    if(packetSize < 1)
    {
        return FALSE;
    }
    if(packet[0] != '{')
    {
        return FALSE;
    }
    return TRUE;
}

FrokResult frokAgentStop()
{
    int error;
    u_int64_t evt = 1;

    TRACE_S("started");

    if(context == NULL)
    {
        TRACE_F("Not inited");
        return FROK_RESULT_INVALID_STATE;
    }

    TRACE_N("lock frokAgentMutex");
    if(-1 == (error = pthread_mutex_lock(&frokAgentMutex)))
    {
        TRACE_F("pthread_mutex_lock failed on error %s", strerror(error));
        return FROK_RESULT_LINUX_ERROR;
    }

    // Send shutdown event
    TRACE_N("send shutdown event via terminateEvent fd");
    if(-1 == write(context->terminateAgentEvent, &evt, sizeof(u_int64_t)))
    {
        TRACE_F("write failed on error %s", strerror(errno));
        return FROK_RESULT_LINUX_ERROR;
    }

    TRACE_N("shutdown local socket %d from reading and writing", context->localSock);
    if(-1 == shutdown(context->localSock, 2))
    {
        TRACE_F("shutdown failed on error %s", strerror(errno));
        return FROK_RESULT_SOCKET_ERROR;
    }

    context->agentStarted = FALSE;

    TRACE_N("unlock frokAgentMutex");
    if(-1 == (error = pthread_mutex_unlock(&frokAgentMutex)))
    {
        TRACE_F("pthread_mutex_unlock failed on error %s", strerror(error));
        pthread_mutex_unlock(&frokAgentMutex);
        return FROK_RESULT_LINUX_ERROR;
    }

    TRACE_S("finished");

    return FROK_RESULT_SUCCESS;
}

FrokResult frokAgentDeinit()
{
    int error;
    FrokResult res;

    TRACE_S("started");

    if(context == NULL)
    {
        TRACE_F_T("Already deinited");
        return FROK_RESULT_INVALID_STATE;
    }

    TRACE_N("lock frokAgentMutex");
    if(-1 == (error = pthread_mutex_lock(&frokAgentMutex)))
    {
        TRACE_F("pthread_mutex_lock failed on error %s", strerror(error));
        return FROK_RESULT_LINUX_ERROR;
    }

    if(context->agentStarted == TRUE)
    {
        if(FROK_RESULT_SUCCESS != (res = frokAgentStop()))
        {
            TRACE_W("frokAgentStop failed on error %s", FrokResultToString(res));
        }
    }

    // Close socket if necessary
    TRACE_N("local socket is %d", context->localSock);
    if(INVALID_SOCKET != context->localSock)
    {
        TRACE_N("Close local socket %d", context->localSock);
        if(-1 == close(context->localSock))
        {
            TRACE_F("close failed on error %s", strerror(errno));
            pthread_mutex_unlock(&frokAgentMutex);
            return FROK_RESULT_SOCKET_ERROR;
        }

        context->localSock = INVALID_SOCKET;
    }

    TRACE_N("Close terminate semaphore fd");
    if(-1 == close(context->terminateAgentEvent))
    {
        TRACE_F("close failed on error %s", strerror(errno));
        pthread_mutex_unlock(&frokAgentMutex);
        return FROK_RESULT_SOCKET_ERROR;
    }

    frokAPIDealloc(context->api);
    free(context);
    context = NULL;

    TRACE_N("unlock frokAgentMutex");
    if(-1 == (error = pthread_mutex_unlock(&frokAgentMutex)))
    {
        TRACE_F("pthread_mutex_unlock failed on error %s", strerror(error));
        pthread_mutex_unlock(&frokAgentMutex);
        return FROK_RESULT_LINUX_ERROR;
    }

    TRACE_S("finished");

    return FROK_RESULT_SUCCESS;
}

FrokResult frokAgentSendData(SOCKET sock, const char* pBuffer, size_t uBufferSize)
{
    ssize_t result;
    size_t offset = 0;
    if(sock != INVALID_SOCKET)
    {
        while(offset != uBufferSize)
        {
            if(-1 == (result = send(sock, pBuffer + offset, uBufferSize - offset, 0)))
            {
                if((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
                {
                    continue;
                }
                TRACE_F("failed to send data to socket %d on error %s", sock, strerror(errno));
                return FROK_RESULT_SOCKET_ERROR;
            }
            offset += result;
        }

        TRACE_N("%zu bytes were sent to the remote peer %u", uBufferSize, sock);
    }
    else
    {
        TRACE_F("Invalid socket");
        return FROK_RESULT_INVALID_PARAMETER;
    }

    return FROK_RESULT_SUCCESS;
}
