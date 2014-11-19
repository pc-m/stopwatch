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
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#include "stopwatch.h"

#define NSEC_PER_MSEC 1000000L
#define NSEC_PER_DECISEC 100000000L

#define CLOCKID CLOCK_MONOTONIC
#define TIMER_SIGNAL SIGRTMIN

struct config {
	struct timespec refresh_interval;
	int quiet;
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
	fprintf(stderr, "usage: %s [-d delay] [-q]\n", prog_name);

	exit(1);
}

static void config_set_default(struct config *config)
{
	config->refresh_interval.tv_sec = 0;
	config->refresh_interval.tv_nsec = 100L * NSEC_PER_MSEC;
	config->quiet = 0;
}

static void config_parse_args(struct config *config, int argc, char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, ":d:hq")) != -1)
	{
		switch (opt) {
		case 'd':
			if (parse_time_interval(optarg, &config->refresh_interval)) {
				fprintf(stderr, "invalid value for option -%c: %s\n", opt, optarg);
				usage(argv[0]);
			}
			break;
		case 'h':
			usage(argv[0]);
			break;
		case 'q':
			config->quiet = 1;
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

static void output_init(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
}

static void output_update(const struct timespec *ts)
{
	int ms = ts->tv_nsec / NSEC_PER_MSEC;
	int s = ts->tv_sec % 60;
	int m = ts->tv_sec / 60 % 60;
	int h = ts->tv_sec / 3600;

	printf("%d:%02d:%02d.%03d\r", h, m, s, ms);
}

int main(int argc, char *argv[])
{
	struct stopwatch watch;
	struct config config;
	struct timespec curr_time;
	struct itimerspec timer_spec;
	struct sigevent sevp;
	sigset_t sigmask;
	timer_t refresh_timer;
	int curr_signal;

	output_init();

	config_set_default(&config);
	config_parse_args(&config, argc, argv);

	if (stopwatch_init(&watch, CLOCKID)) {
		fprintf(stderr, "couldn't init stopwatch\n");
		return -1;
	}

	stopwatch_start(&watch);

	/* block SIGINT and the signal used for the refresh timer */
	sigemptyset(&sigmask);
	sigaddset(&sigmask, SIGINT);
	sigaddset(&sigmask, TIMER_SIGNAL);
	sigprocmask(SIG_BLOCK, &sigmask, NULL);

	/* create the refresh timer */
	sevp.sigev_notify = SIGEV_SIGNAL;
	sevp.sigev_signo = TIMER_SIGNAL;
	if (timer_create(CLOCKID, &sevp, &refresh_timer) == -1) {
		perror("error while calling timer_create");
		return -1;
	}

	timer_spec.it_interval = config.refresh_interval;
	timer_spec.it_value = config.refresh_interval;
	if (!config.quiet) {
		if (timer_settime(refresh_timer, 0, &timer_spec, NULL) == -1) {
			perror("error while calling timer_settime");
			return -1;
		}
	}

	do {
		curr_signal = sigwaitinfo(&sigmask, NULL);
		if (curr_signal == -1) {
			perror("error while calling sigwaitinfo");
			return -1;
		}

		stopwatch_get_time(&watch, &curr_time);
		output_update(&curr_time);
	} while (curr_signal != SIGINT);

	putchar('\n');

	/* XXX could delete the timer, but process is terminating anyway */

	return 0;
}
