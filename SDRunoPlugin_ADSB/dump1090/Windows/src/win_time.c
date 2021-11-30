//
// win_time.c:  Windows time functions
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

#include <Windows.h>
#include <errno.h>
#include "win_time.h"

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag = 0;

  if (tv != NULL) {
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    tmpres /= 10;  // convert to microsecs

    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }

  if (tz != NULL) {
    if (!tzflag) {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }

  return 0;
}

int clock_gettime(clock_t id, struct timespec *ts)
{
    if (id == CLOCK_MONOTONIC) {
        static LARGE_INTEGER freq;
        LARGE_INTEGER count;
        long long int ns;

        if (!freq.QuadPart) {
            // Number of counts per second
            QueryPerformanceFrequency(&freq);
        }
        // Total number of counts from a starting point
        QueryPerformanceCounter(&count);

        // Total nano seconds from a starting point
        ns = count.QuadPart / freq.QuadPart * 1000000000;

        ts->tv_sec = count.QuadPart / freq.QuadPart;
        ts->tv_nsec = ns % 1000000000;
    } else if (id == CLOCK_REALTIME) {
        ULARGE_INTEGER unix_epoch, current_time;
        SYSTEMTIME unix_epoch_st = { 1970, 1, 0, 1, 0, 0, 0, 0};
        FILETIME unix_epoch_ft, current_time_ft;

        // Set current_time in UTC as a 64-bit value representing the number
        // of 100-nanosecond intervals since 01/01/1601 (Windows epoch)
		// Precise function not available on Windows 7 and below
        //GetSystemTimePreciseAsFileTime(&current_time_ft);
		GetSystemTimeAsFileTime(&current_time_ft);
        current_time.LowPart = current_time_ft.dwLowDateTime;
        current_time.HighPart = current_time_ft.dwHighDateTime;
        
        // Set unix_epoch as a 64-bit value representing 01/01/1970 (Unix epoch)
        SystemTimeToFileTime(&unix_epoch_st, &unix_epoch_ft);
        unix_epoch.LowPart = unix_epoch_ft.dwLowDateTime;
        unix_epoch.HighPart = unix_epoch_ft.dwHighDateTime;

        // Now calculate time from epoch to now
        ts->tv_sec = (current_time.QuadPart - unix_epoch.QuadPart) / 10000000;
        ts->tv_nsec = ((current_time.QuadPart - unix_epoch.QuadPart) % 10000000) * 100;
    } 
    else {
        return -1;
    }

    return 0;
}

int clock_nanosleep(clockid_t id, int flags, const struct timespec *ts,
            struct timespec *ots)
{
    HANDLE htimer;
    FILETIME *ft = NULL;
    ULARGE_INTEGER bint;

    if (ts == NULL)
        return EFAULT;
    
    if ((ts->tv_nsec < 0) || (ts->tv_nsec >= 1000000000)
             || ((id != CLOCK_REALTIME) && (id != CLOCK_MONOTONIC)))
        return EINVAL;

    bint.QuadPart = (LONGLONG)ts->tv_sec * 10000000
            + ((LONGLONG)ts->tv_nsec + 50) / 100;

    if (flags == TIMER_ABSTIME)
        bint.QuadPart += FT_EPOCH;	
    else
        bint.QuadPart *= -1;

//	ft->dwLowDateTime = bint.LowPart;
//	ft->dwHighDateTime = bint.HighPart;

    htimer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(htimer, (LARGE_INTEGER *)&ft, 0, NULL, NULL, FALSE);
    WaitForSingleObject(htimer, INFINITE);
    CloseHandle(htimer);

    return 0;
}

struct tm *localtime_r(const time_t *timep, struct tm *result)
{
    if (!timep || !result)
        return NULL;
    memcpy(result, localtime(timep), sizeof(*result));
    return result;
}

int nsleep(long long ns)
{
    HANDLE timer;
    LARGE_INTEGER lin;

    if (!(timer = CreateWaitableTimer(NULL, TRUE, NULL)))
        return 1;

    lin.QuadPart = -ns;
    if (!SetWaitableTimer(timer, &lin, 0, NULL, NULL, FALSE)) {
        CloseHandle(timer);
        return 1;
    }

    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);

    return 0;
}

// usleep() is in microseconds, Windows Sleep() is in milliseconds
void usleep(UINT32 microsecs) {
    Sleep(microsecs / 1000);
}
// sleep() is in seconds, Windows Sleep() is in milliseconds
void sleep(UINT32 secs) {
    Sleep(secs * 1000);
}
