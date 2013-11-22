#include "logging.h"

/****************************************************************************************************************************************/
/*  Function to write to syslog. level is used to indicate whether it's just information or an error.									*/
/****************************************************************************************************************************************/

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

	closelog();
}
