//
// win_time.h:  Windows time functions header file
//
// This file is free software: you may copy, redistribute and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 2 of the License, or (at your
// option) any later version.
//
// This file is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef WIN_TIME_UTILS_H
#define WIN_TIME_UTILS_H

#include "time.h"

#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64

/* timespec time are expressed since Epoch i.e. since January, 1, 1970
whereas windows FILETIME since  January 1, 1601 (UTC)*/
#define FT_EPOCH (((LONGLONG)27111902 << 32) + (LONGLONG)3577643008)

#define TIMER_ABSTIME 1

struct timezone
{
	int  tz_minuteswest; // minutes W of Greenwich
	int  tz_dsttime;     // type of dst correction
};

#if 0
struct timespec {
	time_t tv_sec;  // seconds
	long tv_nsec;   //nanoseconds
};
#endif

typedef enum
{
	CLOCK_REALTIME,
	CLOCK_MONOTONIC,
	CLOCK_PROCESS_CPUTIME_ID,
	CLOCK_THREAD_CPUTIME_ID
} clockid_t;

#ifdef __cplusplus
extern "C" {
#endif

int gettimeofday(struct timeval *tv, struct timezone *tz);
int clock_gettime(clockid_t id, struct timespec *ts);
int clock_nanosleep(clockid_t id, int flags, const struct timespec *ts, struct timespec *ots);
int nsleep(long long ns);
void usleep(unsigned int ulSleep);
void sleep(unsigned int secs);
struct tm *localtime_r(const time_t *timep, struct tm *result);

#ifdef __cplusplus
}
#endif

#endif  // WIN_TIME_UTILS_H
