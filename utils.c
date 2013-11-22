#include "utils.h"
#include "logging.h"

#define ONE_MINUTE_VIDEO_LENGTH 56
#define MILLISECONDS 1000

static unsigned long GetFreeSpace();

int CheckLink(char *interface)
{
	int state = 1;
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock < 0)
	{
		fprintf(stderr, "Socket failed: %d\n", errno);
		return -1;
	}

	struct ifreq if_req;
	strncpy(if_req.ifr_name, interface, sizeof(if_req.ifr_name));
	int rv = ioctl(sock, SIOCGIFFLAGS, &if_req);
	close(sock);

	if (rv == -1)
	{
		fprintf(stderr, "Ioctl failed: %d\n", errno);
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

	return (minutes_of_recording);
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

int main()
{
	printf("Maximum video length in minutes: %lu\n", GetMaxRecordingTime()); 
	return 0;
}
