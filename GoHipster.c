#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "logging.h"
#include "utils.h"

void *RecordingThread(void *param);
void *NetworkThread(void *param);
/****************************************************************************************************************************************/
/*  Main loop of the app.																												*/
/****************************************************************************************************************************************/

void mainloop()
{
	pthread_t recThread;
	pthread_t networkThread;

	if (pthread_create(&recThread, NULL, RecordingThread, NULL) == -1)
	{
		WriteToLog(1, "Cannot create Recording Thread");
		_exit(1);
	}

	if (pthread_create(&networkThread, NULL, NetworkThread, NULL) == -1)
	{
		WriteToLog(1, "Cannot create Networking Thread");
		_exit(1);
	}

	void *result;

	if (pthread_join(recThread, &result) == -1)
	{
		WriteToLog(1, "Error while waiting for Recording Thread to finish");
		_exit(1);
	}

	if (pthread_join(networkThread, &result) == -1)
	{
		WriteToLog(1, "Error while waiting for Networking Thread to finish");
		_exit(1);
	}
}

void *NetworkThread(void *param)
{
	while(1)
	{
		int retval = CheckLink("eth0");

		switch(retval)
		{
			case -1:
				WriteToLog(0, "Error while checking for connection state");
				break;
			case 0:
				WriteToLog(0, "No connection available");
				break;
			case 1:
				TerminateRecording();
				break;
		};
		
		struct timespec req, rem;
		
		req.tv_sec = 0;
		req.tv_nsec = 10000000;

		nanosleep(&req, &rem);
	}

	return 0;
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
	else if (pid == 0)
	{
		while(CheckLink("eth0") == 1)
		{
			struct timespec req, rem;

			req.tv_sec=1;
			req.tv_nsec = 10000000;
			
			nanosleep(&req, &rem);
		}
		
		char *rotation = "270";
		
		unsigned long maxTime = GetMaxRecordingTime();
		
		if (maxTime == 0)
		{
			WriteToLog(1, "Not enough space to start recording");
			_exit(1);
		}

		char *fileName = OutputFileName();

		if (fileName == NULL)
		{
			WriteToLog(1, "Cannot get output file name.");
			_exit(1);
		}

		char *durationStr = calloc(64, sizeof(char));

		if (durationStr == NULL)
		{
			free(fileName);
			WriteToLog(1, "Cannot convert duration to string");
			_exit(1);
		}
		snprintf(durationStr, 64, "%lu", maxTime);

		execl("/usr/bin/raspivid","raspivid", "-o", fileName, "-t", durationStr, "-vs", "-rot", rotation, "-ex", "antishake", "-awb", "auto", "-ifx", "none", NULL);
			
	}

	return 0;
}
