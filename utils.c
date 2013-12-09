#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/statvfs.h>
#include <time.h>

#include "common_headers.h"
#include "utils.h"

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
static bool IsRawVideoFile(char *file);

/****************************************************************************************************************************************/
/*	Function to check if there's working internet connection.																			*/
/*  If IFF_UP is set than it means that network interface is up.																		*/
/*	IFF_RUNNING indicates that network cable is plugged in.																				*/
/*  If there's working connection 1 will be returned. 0 will be returned if there's no working connection. On failure -1 is returned.   */
/****************************************************************************************************************************************/

int CheckLink(char *interface)
{
	int state = 1;
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock < 0)
	{
		WriteToLog(1,"Failed to create socket");
		return -1;
	}

	struct ifreq if_req;
	strncpy(if_req.ifr_name, interface, sizeof(if_req.ifr_name));
	int rv = ioctl(sock, SIOCGIFFLAGS, &if_req);
	close(sock);

	if (rv == -1)
	{
		WriteToLog(1,"Ioctl failed");
		return -1;
	}

	return (if_req.ifr_flags & IFF_UP) && (if_req.ifr_flags & IFF_RUNNING);
}

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

char *GetFileToConvert(void)
{
	DIR *dir = opendir("/home/pi/Recordings");

	if (dir == NULL)
	{
		WriteToLog(1, "Cannot open directory");
		return NULL;
	}

	struct dirent *currdir;

	while ((currdir = readdir(dir)) != NULL)
	{
		if ((currdir->d_type == DT_DIR) || (!strncmp(currdir->d_name, ".", 1)) || (!strncmp(currdir->d_name, "..", 2)))
		{
			continue;
		}

		if (!IsRawVideoFile(currdir->d_name))
		{
			continue;
		}

		char *fileToConvert = calloc(PATH_MAX, sizeof(char));

		if (fileToConvert == NULL)
		{
			WriteToLog(1, "Cannot allocate memory for fileToConvert");
			closedir(dir);
			return NULL;
		}

		snprintf(fileToConvert, PATH_MAX, "/home/pi/Recordings/%s", currdir->d_name);
		closedir(dir);
		return fileToConvert;
	}
	closedir(dir);

	return NULL;
}

char *ConvertOutputFileName(char *filename)
{
	int length = strlen(filename);
	
	char *convFileName = calloc(length, sizeof(char));

	if (convFileName == NULL)
	{
		WriteToLog(0, "Unable to allocate memory for convFileName");
		return NULL;
	}

	
	int amount = strstr(filename, "h264") - filename;

	strncpy(convFileName, filename, amount);
	strncat(convFileName, "mp4", 3);

	return convFileName;
}

static bool IsRawVideoFile(char *file)
{
	if (strstr(file, "h264") == NULL)
	{
		return false;
	}

	return true;
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
