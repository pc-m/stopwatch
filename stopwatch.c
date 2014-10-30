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

static int is_clock_supported(clockid_t clock_id)
{
	struct timespec test;

	return clock_gettime(clock_id, &test) == 0;
}

int stopwatch_init(struct stopwatch *watch, clockid_t clock_id)
{
	if (!is_clock_supported(clock_id)) {
		return -1;
	}

	watch->total_time.tv_sec = 0;
	watch->total_time.tv_nsec = 0L;
	watch->clock_id = clock_id;
	watch->running = 0;

	return 0;
}

int stopwatch_start(struct stopwatch *watch)
{
	if (watch->running) {
		return -1;
	}

	clock_gettime(watch->clock_id, &watch->start_time);
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

	clock_gettime(watch->clock_id, &now);

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
