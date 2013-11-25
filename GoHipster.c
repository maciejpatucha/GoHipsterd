#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "logging.h"
#include "utils.h"

void *RecordingThread(void *param);
void *NetworkThread(void *param);
bool recordingOn;
/****************************************************************************************************************************************/
/*  Main loop of the app.																												*/
/****************************************************************************************************************************************/

void mainloop()
{
	pthread_t networkThread;

	recordingOn = false;
	pthread_t recordingThread;

	if (pthread_create(&recordingThread, NULL, RecordingThread, NULL) == -1)
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

	if (pthread_join(recordingThread, &result) == -1)
	{
		WriteToLog(1,"Error while waiting for Recording Thread to finish");
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
	struct timespec req, rem;
	
	req.tv_sec = 0;
	req.tv_nsec = 100000000;

	while(1)
	{
		int retval = CheckLink("eth0");
		WriteToLog(0, "Still running...");
		switch(retval)
		{
			case -1:
				WriteToLog(0, "Error while checking for connection state");
				break;
			case 0:
				recordingOn = true;
				break;
			case 1:
				if (recordingOn)
				{
					TerminateRecording();
					recordingOn = false;
				}
				break;
		};
		
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
	struct timespec req, rem;
	
	req.tv_sec = 0;
	req.tv_nsec = 10000000;
	
	while (1)
	{
		nanosleep(&req, &rem);
		
		if (!recordingOn)
		{
			continue;
		}

		pid_t raspividPid = fork();

		if (raspividPid == -1)
		{
			WriteToLog(1, "Unable to fork child process");
			return 0;
		}
		else if (raspividPid == 0)
		{
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

			execl("/usr/bin/raspivid","raspivid", "-o", fileName, "-t", durationStr, "-vs", "-rot", rotation, "-ex", "antishake", "-awb", "auto", "-ifx", "none", "-b", "8000000", NULL);
		}
		else if (raspividPid > 0)
		{
			int status;
			waitpid(raspividPid, &status, 0);
		}
	}

	return 0;
}
