#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common_headers.h"
#include "logging.h"
#include "recordingutils.h"
#include "networkutils.h"
#include "convertutils.h"
#include "processutils.h"

void *RecordingThread(void *param);
void *NetworkThread(void *param);
void *ConvertThread(void *param);

bool recordingOn;
bool convert;
bool convertInProgress;

/****************************************************************************************************************************************/
/*  Main loop of the app.																												*/
/****************************************************************************************************************************************/

void mainloop(void)
{
	pthread_t recordingThread;
	pthread_t networkThread;
	pthread_t convertingThread;

	recordingOn = false;
	convert = false;
	convertInProgress = false;

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

	if (pthread_create(&convertingThread, NULL, ConvertThread, NULL) == -1)
	{
		WriteToLog(1, "Cannot create Converting Thread");
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

	if (pthread_join(convertingThread, &result) == -1)
	{
		WriteToLog(1, "Error while waiting for Converting Thread to finish");
		_exit(1);
	}
}

/****************************************************************************************************************************************/
/*  Thread responsible for converting all h264 files recorded by the camera to mp4 format.												*/
/****************************************************************************************************************************************/

void *ConvertThread(void *param)
{
	struct timespec req, rem;

	req.tv_sec = 0;
	req.tv_nsec = 10000000;

	while(1)
	{
		nanosleep(&req, &rem);

		if (!convert || recordingOn || convertInProgress)
		{
			continue;
		}
		
		convertInProgress = true;

		char *inputFile = GetFileToConvert();
		
		if (inputFile == NULL)
		{
			convert = false;
			convertInProgress = false;
			continue;
		}

		char input[PATH_MAX];

		snprintf(input, PATH_MAX, "%s", inputFile);
		free(inputFile);

		char *outputFile = ConvertOutputFileName(input);

		if (outputFile == NULL)
		{
			convertInProgress = false;
			free(inputFile);
			continue;
		}

		char output[PATH_MAX];

		snprintf(output, PATH_MAX, "%s", outputFile);
		free(outputFile);

		pid_t convertPid = fork();

		if (convertPid == -1)
		{
			WriteToLog(1, "Unable to fork child process");
			continue;
		}
		else if (convertPid == 0)
		{
			execl("/usr/bin/MP4Box", "MP4Box", "-fps", "25", "-add", input, output);
		}
		else if (convertPid > 0)
		{
			int status;
			waitpid(convertPid, &status, 0);

			if (WEXITSTATUS(status) == 0)
			{
				convertInProgress = false;
				unlink(input);
			}
		}
	}
}

/****************************************************************************************************************************************/
/*  Thread responsible for checking on connection state.																				*/
/*  If network is disconnected than start recording. If network is connected thread will check if recording was on and terminate it.	*/
/****************************************************************************************************************************************/

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
			    if (!convert)
				{
					recordingOn = true;
				}
				break;
			case 1:
				if (recordingOn)
				{
					TerminateRecording();
					recordingOn = false;
					convert = true;
				}
				else if (!convertInProgress)
				{
					convert=true;
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
		
		if ((!recordingOn) || (convert))
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

			execl("/usr/bin/raspivid","raspivid", "-o", fileName, "-t", durationStr, "-vs", "-rot", rotation, "-ex", "antishake", "-awb", "auto", "-ifx", "none", "-b", "2000000", "-fps", "25", NULL);
		}
		else if (raspividPid > 0)
		{
			int status;
			waitpid(raspividPid, &status, 0);
		}
	}

	return 0;
}
