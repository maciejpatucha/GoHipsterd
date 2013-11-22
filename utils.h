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

int CheckLink(char *interface);
unsigned long GetMaxRecordingTime();
int TerminateRecording();
char *PrepareCommand(unsigned long duration);
