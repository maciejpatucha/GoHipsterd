#include <pthread.h>
#include <unistd.h>

#include "logging.h"
#include "utils.h"

void *RecordingThread(void *param);

/****************************************************************************************************************************************/
/*  Main loop of the app.																												*/
/****************************************************************************************************************************************/

void mainloop()
{
	pthread_t recTh;

	if (pthread_create(&recTh, NULL, RecordingThread, NULL) == -1)
	{
		WriteToLog(1, "Cannot create Recording Thread");
		_exit(1);
	}

	void *result;

	if (pthread_join(recTh, &result) == -1)
	{
		WriteToLog(1, "Error while waiting for Recording Thread to finish");
		_exit(1);
	}
}

/****************************************************************************************************************************************/
/*  Thread function responsible for starting recording.																					*/
/*  Recording will not start until network cable is plugged.																			*/
/****************************************************************************************************************************************/

void *RecordingThread(void *param)
{
	pid_t pid = fork();

	if (pid == -1)
	{
		WriteToLog(1, "Cannot fork process");
		_exit(1);
	}
	else if (!pid)
	{
		while(!CheckLink("eth0"));
		char *rotation = "270";
		
		unsigned long maxTime = GetMaxRecordingTime();
		
		if (maxTime == 0)
		{
			WriteToLog(1, "Not enough space to start recording");
			_exit(1);
		}

		char *fileName = OutputFileName();

		if (filename == NULL)
		{
			WriteToLog(1, "Cannot get output file name.");
			_exit(1);
		}

		char *durationStr = calloc(64, sizeof(char));

		if (durationStr == NULL)
		{
			free(filename);
			WriteToLog(1, "Cannot convert duration to string");
			_exit(1);
		}
		snprintf(durationStr, 64, "%lu", maxTime);

		execl("/usr/bin/raspivid","raspivid", "-o", fileName, "-t", durationStr, "-vs", "-rot", rotation, "-ex", "antishake", "-awb", "auto", "-ifx", "none", NULL);
			
	}
	else if (pid > 0)
	{
		while(1)
		{
			if (CheckLink("eth0"))
			{
				TerminateRecording();	
			}
		}
	}
}
