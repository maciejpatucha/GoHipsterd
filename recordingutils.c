#include <sys/statvfs.h>
#include <time.h>

#include "common_headers.h"
#include "recordingutils.h"

/****************************************************************************************************************************************/
/*  Video is recorded with following settings:																							*/
/*  Resolution: 1920x1080																												*/
/*  Fps: 30																																*/
/*  Bitrate: 8000000																													*/
/*  One minute of a video recording with those settings requires 56 MB of free disk space. This is a basic unit used to calculate		*/
/*  maximum time of recording.																											*/
/****************************************************************************************************************************************/

#define ONE_MINUTE_VIDEO_LENGTH_8MBS 57
#define ONE_MINUTE_VIDEO_LENGTH_4MBS 29
#define ONE_MINUTE_VIDEO_LENGTH_17MBS 122
#define ONE_MINUTE_LOW_QUALITY 12
#define MILLISECONDS 60000

static unsigned long GetFreeSpace(void);

/****************************************************************************************************************************************/
/*  Function to determine maximum time of recording based on available space.															*/
/*  We'll use only half of the available space for recording to be able to compress video later into mp4								*/
/*  On success recording time in milliseconds is returend. On failure 0 is returned.													*/
/****************************************************************************************************************************************/

unsigned long GetMaxRecordingTime(void)
{
	unsigned long free_space;
	if ((free_space = GetFreeSpace()) == 0)
	{
		return 0;
	}

	unsigned long minutes_of_recording = (free_space / (ONE_MINUTE_LOW_QUALITY));

	return (minutes_of_recording * MILLISECONDS);
}

/****************************************************************************************************************************************/
/*  Generate output file name in format: VID_date_time.h264																				*/
/*  On success function returns pointer to char array containing file name. On failure function returns NULL.							*/
/****************************************************************************************************************************************/

char *OutputFileName(void)
{
	time_t rawtime;
	struct tm *ltime;
	
	time(&rawtime);
	ltime = localtime(&rawtime);

	char *dateTimeString = calloc(PATH_MAX, sizeof(char));
	
	if (dateTimeString == NULL)
	{
		WriteToLog(1, "Unable to allocate string for filename");
		return NULL;
	}

	snprintf(dateTimeString, PATH_MAX, "/home/pi/Recordings/VID_%d-%d-%d_%d-%d-%d.h264", ltime->tm_year, ltime->tm_mon, ltime->tm_mday, ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

	return dateTimeString;
}

/****************************************************************************************************************************************/
/*  Get available space.																												*/
/*  On success total free space in MB is returned. On failure 0 is returned.															*/
/****************************************************************************************************************************************/

static unsigned long GetFreeSpace(void)
{
	struct statvfs vfsbuffer;

	if (statvfs("/", &vfsbuffer) == -1)
	{
		WriteToLog(1, "Cannot get information about file system");
		return 0;
	}

	long long total_free_space = (long long)vfsbuffer.f_bsize * (long long)vfsbuffer.f_bavail;

	long long total_free_MB = total_free_space/(long long)(1023*1024);

	return (unsigned long)total_free_MB;
}
