#include "commonSched.h"
#include "faceCommonLib.h"

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define MODULE_NAME     "SCHED"

BOOL frokBecomeADaemon()
{
    pid_t pid;
    int i;

    TRACE_N("Try to become a daemon");

    TRACE_N("Create faemon process");
    pid = fork();
    if(pid == -1)
    {
        TRACE_F("fork failed on error %s", strerror(errno));
        return FALSE;
    }
    else if(pid != 0)
    {
        TRACE_S("Old process finished");
        exit(EXIT_SUCCESS);
    }

    TRACE_S("Daemon process created");

    TRACE_N("Set new session for process");
    if(-1 == setsid())
    {
        TRACE_F("setsid failed on error %s", strerror(errno));
        return FALSE;
    }
    TRACE_S("New session for process created");

    TRACE_N("Set \"/\" as a working directory");
    if(-1 == chdir("/"))
    {
        TRACE_F("chdir failed on error %s", strerror(errno));
        return FALSE;
    }
    TRACE_S("chdir succeed");

    TRACE_S("Daemon started");

    TRACE_S("finished");
}
