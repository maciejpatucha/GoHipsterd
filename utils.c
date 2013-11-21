#include "utils.h"

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
