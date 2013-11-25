#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <stdbool.h>

#include "utils.h"
#include "logging.h"

/****************************************************************************************************************************************/
/*  Video is recorded with following settings:																							*/
/*  Resolution: 1920x1080																												*/
/*  Fps: 30																																*/
/*  Bitrate: 8000000																													*/
/*  One minute of a video recording with those settings requires 56 MB of free disk space. This is a basic unit used to calculate		*/
/*  maximum time of recording.																											*/
/****************************************************************************************************************************************/

#define ONE_MINUTE_VIDEO_LENGTH 56 
#define MILLISECONDS 60000

static unsigned long GetFreeSpace(void);
static pid_t FindRunningProcess(const char *procName);
static char *FileExist(const char *path);
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

	unsigned long minutes_of_recording = (free_space / (ONE_MINUTE_VIDEO_LENGTH *2));

	return (minutes_of_recording * MILLISECONDS);
}

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

	long long total_free_MB = total_free_space/(long long)(1024*1024);

	return (unsigned long)total_free_MB;
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

