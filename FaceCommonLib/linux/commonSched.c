#include "commonSched.h"
#include "faceCommonLib.h"

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>

#define MODULE_NAME     "SCHED"

BOOL frokBecomeADaemon()
{
    pid_t pid;
    int i;

    if(!commonContext)
    {
        TRACE_F("frokLibCommon not inited");
        return FALSE;
    }

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

BOOL obtainCPU(short cpu_number)
{
    cpu_set_t set;
    int i;

    CPU_ZERO(&set);

    TRACE_N("Get available CPUs");

    if(-1 == sched_getaffinity(0, sizeof(cpu_set_t), &set))
    {
        TRACE_F("sched_getaffinity failed on error %s", strerror(errno));
        return FALSE;
    }

    for(i = 0; i < CPU_SETSIZE; i++)
    {
        CPU_CLR(i, &set);
    }
    CPU_SET(cpu_number, &set);

    TRACE_N("Obtain CPU #%d", cpu_number);

    if(-1 == sched_setaffinity(0, sizeof(cpu_set_t), &set))
    {
        TRACE_F("sched_setaffinity failed on error %s", strerror(errno));
        return FALSE;
    }

    TRACE_N("Set nice value to -15");

    if(-1 == setpriority(PRIO_PROCESS, 0, -15))
    {
        TRACE_F("setpriority failed on error %s", strerror(errno));
        return FALSE;
    }

    TRACE_S("finished");
    return TRUE;
}
