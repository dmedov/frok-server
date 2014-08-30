#ifndef LINUXINCLUDES_H
#define LINUXINCLUDES_H

#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#define SOCKET                              int
#define SOCKET_ERROR                        (-1)
#define INVALID_SOCKET                      SOCKET_ERROR
#define UNUSED(P)                           __attribute__ ((unused)) P

#endif // LINUXINCLUDES_H
