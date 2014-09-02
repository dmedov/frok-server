#ifndef FACECOMMONLIB_H
#define FACECOMMONLIB_H

// Common defines
#define CASE_RET_STR(x)                     case x: return #x

// Lib includes
#include "frokCommonDefaults.h"
#include "frokCommonTypes.h"

#include "io.h"                 // File system - depend operations, input - output operations
#include "commonMath.h"         // Calculating ChiSquare percantage
#include "linux/commonSched.h"

// externals
extern frokCommonContext *commonContext;

#ifdef __cplusplus
extern "C" {
#endif

// Library init functions
const char *FrokResultToString(FrokResult res);
FrokResult frokLibCommonInit(const char *configFilePath);
void frokLibCommonDeinit();
void set_time_stamp(unsigned *sec, unsigned *usec);
void set_trace_prefix(const char *prefix);
#ifdef __cplusplus
}
#endif

#endif // FACECOMMONLIB_H
