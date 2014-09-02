#ifndef COMMONSCHED_H
#define COMMONSCHED_H

#include "frokCommonTypes.h"

// Return type is BOOL
BOOL frokBecomeADaemon();
BOOL obtainCPU(short cpu_number);

#endif // COMMONSCHED_H
