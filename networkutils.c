#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#include "networkutils.h"
#include "common_headers.h"

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
