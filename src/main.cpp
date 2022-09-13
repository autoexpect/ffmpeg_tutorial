#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "ffmpeg.h"

bool g_flag_run = 1;

long long current_timestamp()
{
	struct timeval te;
	gettimeofday(&te, NULL);
	long long milliseconds = te.tv_sec * 1000000LL + te.tv_usec;
	return milliseconds;
}

static void signal_process(int signo)
{
	g_flag_run = false;
	sleep(1);

	exit(0);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, signal_process);
	signal(SIGPIPE, SIG_IGN);

	FFmpegStreamChannel *channel = new FFmpegStreamChannel();
	channel->decode(argv[1]);
}