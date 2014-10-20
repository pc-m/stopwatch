#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <time.h>

struct stopwatch {
	struct timespec start_time;
	struct timespec total_time;
	int running;
};

/*
 * Initialize a stopwatch.
 *
 * The stopwatch starts in the non-running state.
 *
 * Return 0 on success, non-zero on failure.
 */
int stopwatch_init(struct stopwatch *watch);

/*
 * Start the stopwatch.
 *
 * Is is an error to start an already running stopwatch.
 *
 * Return 0 on sucess, non-zero on error.
 */
int stopwatch_start(struct stopwatch *watch);

/*
 * Stop the stopwatch.
 *
 * Is is an error to stop a non-running stopwatch.
 *
 * Return 0 on sucess, non-zero on error.
 */
int stopwatch_stop(struct stopwatch *watch);

/*
 * Get the total running time of the stopwatch.
 *
 * The result is stored in *res.
 *
 * Return 0 on success, non-zero on error.
 */
int stopwatch_get_time(const struct stopwatch *watch, struct timespec *res);

#endif
