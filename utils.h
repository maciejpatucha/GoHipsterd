#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>
#include <string.h>
#include <unistd.h>

int CheckLink(char *interface);
