#include <fcntl.h>
#include <time.h>

#include "locking.h"
#include "common_headers.h"

void RecordingLock(void)
{
    struct timespec req;

    req.tv_sec = 0;
    req.tv_nsec = 1000000;

    while (TryRecordingLock())
    {
        nanosleep(&req, NULL);
    }

    int fd = open("/var/lock/recordings.lck", O_WRONLY | O_CREAT | O_EXCL, 0666);

    if (fd == -1)
    {
        WriteToLog(1, "Failed to create recording lock!!!");
        return;
    }
}

void ConversionLock(void)
{
    struct timespec req;

    req.tv_sec = 0;
    req.tv_nsec = 1000000;

    while(TryConversionLock())
    {
        nanosleep(&req, NULL);
    }

    int fd = open("/var/lock/conversion.lck", O_WRONLY | O_CREAT | O_EXCL, 0666);

    if (fd == -1)
    {
        WriteToLog(1, "Failed to create conversion lock!!!");
        return;
    }
}

void RecordingUnlock(void)
{
    unlink("/var/lock/recording.lck");
}

void ConversionUnlock(void)
{
    unlink("/var/lock/conversion.lck");
}

bool TryRecordingLock(void)
{
    struct stat buf;

    if (stat("/var/lock/recording.lck", &buf) == 0)
    {
        return true;
    }

    return false;
}

bool TryConversionLock(void)
{
    struct stat buf;

    if (stat("/var/lock/conversion.lck", &buf) == 0)
    {
        return true;
    }

    return false;
}
