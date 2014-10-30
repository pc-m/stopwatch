/*
 * Copyright 2014 Etienne Lessard
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#include "stopwatch.h"

#define NSEC_PER_MSEC 1000000L
#define NSEC_PER_DECISEC 100000000L

struct output {
	void (*init)(void);
	void (*update)(const struct timespec *ts);
};

static volatile sig_atomic_t sigint_delivered = 0;

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

static int parse_time_interval(const char *value, struct timespec *result)
{
	long integer_part = 0;
	long fractional_part = 0;
	long weight;
	int i = 0;

	/* parse integer part */
	for (; isdigit(value[i]); i++) {
		integer_part = integer_part * 10 + value[i] - '0';
	}

	switch (value[i]) {
	case '.':
		i++;
		break;
	case '\0':
		if (i == 0) {
			return -1;
		}

		goto success;
	default:
		return -1;
	}

	/* parse fractional part */
	weight = NSEC_PER_DECISEC;
	for (; isdigit(value[i]); i++) {
		fractional_part = fractional_part + (value[i] - '0') * weight;

		weight = weight / 10;
		if (weight == 0) {
			goto success;
		}
	}

	switch (value[i]) {
	case '\0':
		if (i == 1) {
			return -1;
		}

		break;
	default:
		return -1;
	}

success:
	result->tv_sec = integer_part;
	result->tv_nsec = fractional_part;

	return 0;
}

static void usage(const char *prog_name)
{
	fprintf(stderr, "usage: %s [-d delay]\n", prog_name);

	exit(1);
}

static void parse_args(int argc, char *argv[], struct timespec *refresh_interval)
{
	int opt;

	/* set default value */
	refresh_interval->tv_sec = 0;
	refresh_interval->tv_nsec = 100L * NSEC_PER_MSEC;

	while ((opt = getopt(argc, argv, ":d:h")) != -1)
	{
		switch (opt) {
		case 'd':
			if (parse_time_interval(optarg, refresh_interval)) {
				fprintf(stderr, "invalid value for option -%c: %s\n", opt, optarg);
				usage(argv[0]);
			}
			break;
		case 'h':
			usage(argv[0]);
			break;
		case '?':
			fprintf(stderr, "unrecognized option -%c\n", optopt);
			usage(argv[0]);
			break;
		case ':':
			fprintf(stderr, "option -%c requires an argument\n", optopt);
			usage(argv[0]);
			break;
		}
	}

	if (optind != argc) {
		fprintf(stderr, "unexpected argument\n");
		usage(argv[0]);
	}
}

static void sigint_handler(int signum)
{
	sigint_delivered = 1;
}

int main(int argc, char *argv[])
{
	struct stopwatch watch;
	struct timespec curr_time;
	struct timespec refresh_interval;
	struct output output;
	struct sigaction sa;
	sigset_t sigmask;
	sigset_t sigmask_orig;

	if (isatty(fileno(stdout))) {
		output = tty_output;
	} else {
		output = notty_output;
	}

	output.init();

	parse_args(argc, argv, &refresh_interval);

	if (stopwatch_init(&watch, CLOCK_MONOTONIC)) {
		fprintf(stderr, "couldn't init stopwatch\n");
		return -1;
	}

	stopwatch_start(&watch);

	sigemptyset(&sa.sa_mask);
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		fprintf(stderr, "couldn't install SIGINT handler\n");
		return -1;
	}

	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGINT);
	sigprocmask(SIG_BLOCK, &sigmask, &sigmask_orig);

	while (!sigint_delivered) {
		pselect(0, NULL, NULL, NULL, &refresh_interval, &sigmask_orig);

		stopwatch_get_time(&watch, &curr_time);

		output.update(&curr_time);
	}

	putchar('\n');

	return 0;
}
