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

#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <time.h>

struct stopwatch {
	struct timespec start_time;
	struct timespec total_time;
	clockid_t clock_id;
	int running;
};

/*
 * Initialize a stopwatch.
 *
 * Use the clock clock_id to mesure the elapsed time. It is recommended to use a
 * monotonic clock. The CLOCK_MONOTONIC clock is a good default.
 *
 * The stopwatch starts in the non-running state.
 *
 * Return 0 on success, non-zero on failure.
 */
int stopwatch_init(struct stopwatch *watch, clockid_t clock_id);

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
