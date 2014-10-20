#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "stopwatch.h"

#define NSEC_PER_MSEC 1000000L

struct output {
	void (*init)(void);
	void (*update)(const struct timespec *ts);
};

static void tty_output_init(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
}

static void tty_output_update(const struct timespec *ts)
{
	int ms = ts->tv_nsec / NSEC_PER_MSEC;
	int s = ts->tv_sec % 60;
	int m = ts->tv_sec / 60 % 60;
	int h = ts->tv_sec / 3600;

	printf("%d:%02d'%02d\"%03d\r", h, m, s, ms);
}

static void notty_output_init(void)
{
	setvbuf(stdout, NULL, _IOLBF, 0);
}

static void notty_output_update(const struct timespec *ts)
{
	int ms = ts->tv_nsec / NSEC_PER_MSEC;
	int s = ts->tv_sec % 60;
	int m = ts->tv_sec / 60 % 60;
	int h = ts->tv_sec / 3600;

	printf("%d:%02d'%02d\"%03d\n", h, m, s, ms);
}

static const struct output tty_output = {
	.init = tty_output_init,
	.update = tty_output_update,
};

static const struct output notty_output = {
	.init = notty_output_init,
	.update = notty_output_update,
};

static void parse_args(int argc, char *argv[], struct timespec *refresh_interval)
{
	/* TODO implement more seriously */
	refresh_interval->tv_sec = 0;
	refresh_interval->tv_nsec = 100L * NSEC_PER_MSEC;
}

int main(int argc, char *argv[])
{
	struct stopwatch watch;
	struct timespec curr_time;
	struct timespec refresh_interval;
	struct output output;

	if (isatty(fileno(stdout))) {
		output = tty_output;
	} else {
		output = notty_output;
	}

	output.init();

	parse_args(argc, argv, &refresh_interval);

	if (stopwatch_init(&watch)) {
		fprintf(stderr, "couldn't init stopwatch\n");
		return -1;
	}

	stopwatch_start(&watch);
	for (;;) {
		nanosleep(&refresh_interval, NULL);

		stopwatch_get_time(&watch, &curr_time);

		output.update(&curr_time);
	}

	return 0;
}
