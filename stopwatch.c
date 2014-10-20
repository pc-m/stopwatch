#include "stopwatch.h"

#define NSEC_PER_SEC 1000000000L

static inline struct timespec timespec_sub(struct timespec lhs, struct timespec rhs)
{
	struct timespec res;

	res.tv_sec = lhs.tv_sec - rhs.tv_sec;
	res.tv_nsec = lhs.tv_nsec - rhs.tv_nsec;
	if (res.tv_nsec < 0L) {
		res.tv_sec -= 1;
		res.tv_nsec += NSEC_PER_SEC;
	}

	return res;
}

static inline struct timespec timespec_add(struct timespec lhs, struct timespec rhs)
{
	struct timespec res;

	res.tv_sec = lhs.tv_sec + rhs.tv_sec;
	res.tv_nsec = lhs.tv_nsec + rhs.tv_nsec;
	if (res.tv_nsec >= NSEC_PER_SEC) {
		res.tv_sec += 1;
		res.tv_nsec -= NSEC_PER_SEC;
	}

	return res;
}

int stopwatch_init(struct stopwatch *watch)
{
	struct timespec test;

	/* check if clock is supported on this system */
	if (clock_gettime(CLOCK_MONOTONIC, &test) == -1) {
		return -1;
	}

	watch->total_time.tv_sec = 0;
	watch->total_time.tv_nsec = 0L;
	watch->running = 0;

	return 0;
}

int stopwatch_start(struct stopwatch *watch)
{
	if (watch->running) {
		return -1;
	}

	clock_gettime(CLOCK_MONOTONIC, &watch->start_time);
	watch->running = 1;

	return 0;
}

/*
 * Get the elapsed time of the current run.
 *
 * stopwatch must be running, otherwise its undefined behaviour.
 */
static inline struct timespec stopwatch_get_run_time(const struct stopwatch *watch)
{
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);

	return timespec_sub(now, watch->start_time);
}

int stopwatch_stop(struct stopwatch *watch)
{
	if (!watch->running) {
		return -1;
	}

	watch->total_time = timespec_add(watch->total_time, stopwatch_get_run_time(watch));
	watch->running = 0;

	return 0;
}

int stopwatch_get_time(const struct stopwatch *watch, struct timespec *res)
{
	*res = watch->total_time;
	if (watch->running) {
		*res = timespec_add(*res, stopwatch_get_run_time(watch));
	}

	return 0;
}
