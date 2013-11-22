#include "utils.h"
#include "logging.h"

#define ONE_MINUTE_VIDEO_LENGTH 56
#define MILLISECONDS 60000

static unsigned long GetFreeSpace();
static pid_t FindRunningProcess(const char *procName);
static char *FileExist(const char *path);
static char *OutputFileName();

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

unsigned long GetMaxRecordingTime()
{
	unsigned long free_space;
	if ((free_space = GetFreeSpace()) == 0)
	{
		return 0;
	}

	unsigned long minutes_of_recording = (free_space / (ONE_MINUTE_VIDEO_LENGTH *2));

	return (minutes_of_recording * MILLISECONDS);
}

int TerminateRecording()
{
	pid_t recordingPid = FindRunningProcess("GoHipsterd");

	if (recordingPid == 0)
	{
		return -1;
	}

	if (kill(recordingPid, SIGTERM) == -1)
	{
		WriteToLog(1, "Error sending SIGTERM to recording process");
		return -1;
	}

	return 0;
}

char *PrepareCommand(unsigned int duration)
{
	char *command = calloc(256, sizeof(char));
	
	if (command == NULL)
	{
		WriteToLog(1, "Unable to allocate memory for command string. Backing off");
		return NULL;
	}

	char *filename = OutputFileName();
	
	if (filename == NULL)
	{
		return NULL;
	}

	snprintf(command, 256, "raspivid -o %s -t %lu -rot 270 -vs -ex antishake -awb auto -ifx none -n", filename, duration);	

	free(filename);

	return command;
}

static unsigned long GetFreeSpace()
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
		char *processName = FileExist(currdir->d_name);
		
		if (processName == NULL)
		{
			continue;
		}

		if (strncmp(processName, procName, 10) == 0)
		{
			free(processName);
			unsigned int pid = (unsigned int) atoi(currdir->d_name);
			return pid;
		}

		free(processName);
	}

	return 0;
}

static char *FileExist(const char *path)
{
	char buffer[PATH_MAX] = { 0 };
	struct stat file;

	snprintf(buffer, PATH_MAX, "%s/comm", path);

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

static char *OutputFileName()
{
	time_t rawtime;
	struct tm *ltime;
	
	time(&rawtime);
	ltime = localtime(&rawtime);

	char *dateTimeString = calloc(NAME_MAX, sizeof(char));
	
	if (dateTimeString == NULL)
	{
		WriteToLog(1, "Unable to allocate string for filename");
		return NULL;
	}

	snprintf(dateTimeString, NAME_MAX, "VID_%s-%s-%s_%s:%s:%s.h264", ltime->tm_year, ltime->tm_mon, ltime->tm_mday, ltime->hour, ltime->min, ltime->sec);

	return dateTimeString;
}
