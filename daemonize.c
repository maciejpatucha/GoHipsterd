#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "logging.h"
#include "daemonize.h"
#include "GoHipster.h"

/****************************************************************************************************************************************/
/*  Function to exit in case of error while deamonising.																				*/
/****************************************************************************************************************************************/

static void die(char *msg)
{
	WriteToLog(1, msg);
	_exit(1);
}

/****************************************************************************************************************************************/
/*  Function to daemonise.																												*/
/****************************************************************************************************************************************/

void daemonise(void)
{
	pid_t pid = fork();
	if (pid == -1)
	{
		die("failed to fork while daemonising");
	}
	else if (pid != 0)
	{
		_exit(0);
	}

	if (setsid() == -1)
	{
		die("failed to become a session leader while daemonising");
	}

	signal(SIGHUP,SIG_IGN);

	pid = fork();

	if (pid == -1)
	{
		die("failed to fork while daemonising");
	}
	else if (pid != 0)
	{
		_exit(0);
	}

	if (chdir("/") == -1)
	{
		die("failed to change working directory while daemonising");
	}

	umask(0);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	if (open("/dev/null", O_RDONLY) == -1)
	{
		die("failed to reopen stdin while daemonising");
	}

	if (open("/dev/null", O_WRONLY) == -1)
	{
		die("failed to reopen stdout while daemonising");
	}

	if (open("/dev/null", O_RDWR) == -1)
	{
		die("failed to reopen stderr while daemonising");
	}

	mainloop();
}
