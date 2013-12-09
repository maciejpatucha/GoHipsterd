#include <sys/types.h>

#include "common_headers.h"
#include "processutils.h"

static pid_t FindRunningProcess(const char *procName);
static char *FileExist(const char *path);

/****************************************************************************************************************************************/
/*  Terminate currently running recording process.																						*/
/*  On success 0 is returned. On failure -1 is returned.																				*/
/****************************************************************************************************************************************/

int TerminateRecording(void)
{
    pid_t recordingPid = FindRunningProcess("raspivid");

    if (recordingPid == 0)
    {
        return -1;
    }

    if (kill(recordingPid, SIGKILL) == -1)
    {
        WriteToLog(1, "Error sending SIGTERM to recording process");
        return -1;
    }

    WriteToLog(0, "raspivid killed successfully");

    return 0;
}

/****************************************************************************************************************************************/
/*  Scan /proc directory looking for process specified in procName.																		*/
/*  On success function returns pid of raspivid process. On failure 0 is returned.														*/
/****************************************************************************************************************************************/

static pid_t FindRunningProcess(const char *procName)
{
    DIR *dir = opendir("/proc");

    if (dir == NULL)
    {
        WriteToLog(1, "Cannot open proc directory to read");
        return 0;
    }

    struct dirent *currdir;

    while ((currdir = readdir(dir)) != NULL)
    {
        if ((currdir->d_type !=DT_DIR) || (!strncmp(currdir->d_name, ".", 1)) || (!strncmp(currdir->d_name, "..", 2)))
        {
            continue;
        }

        char *processName = FileExist(currdir->d_name);

        if (processName == NULL)
        {
            continue;
        }

        if (strncmp(processName, procName, 10) == 0)
        {
            free(processName);
            unsigned int pid = (unsigned int) atoi(currdir->d_name);
            closedir(dir);
            return pid;
        }

        free(processName);
    }

    closedir(dir);

    return 0;
}

/****************************************************************************************************************************************/
/*  Check if file comm exists in path specified in path parameter.																		*/
/*  On success function returns process name read from comm file. On failure function returns NULL.										*/
/****************************************************************************************************************************************/

static char *FileExist(const char *path)
{
    char buffer[PATH_MAX] = { 0 };
    struct stat file;

    snprintf(buffer, PATH_MAX, "/proc/%s/comm", path);

    if (stat(buffer, &file) == -1)
    {
        return NULL;
    }

    char *processName = calloc(256, sizeof(char));

    if (processName == NULL)
    {
        return NULL;
    }

    FILE *fd = fopen(buffer, "r+");

    if (fd == NULL)
    {
        return NULL;
    }

    fscanf(fd, "%s", processName);
    fclose(fd);

    return processName;
}
