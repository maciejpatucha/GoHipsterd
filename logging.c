#include "logging.h"

void WriteToLog(int level, char *message)
{
	openlog("GoHipsterd", LOG_PID, LOG_USER);

	switch (level)
	{
		case 0:
			syslog(LOG_INFO, "%s", message);
			break;
		case 1:
			syslog(LOG_ERR, "%s", message);
			break;
	};
}
