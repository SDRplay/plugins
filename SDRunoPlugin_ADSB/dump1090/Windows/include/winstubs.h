// dump1090, a Mode S messages decoder for RTLSDR devices.
//
// Copyright (C) 2014 by Malcolm Robb <support@attavionics.com>
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//  *  Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//
//  *  Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// This file provides basic Windows implementation of Linux specific functions
// used in the dump1090 project. This allows dump1090 to be compiled and debugged
// using Microsoft Visual C++ 6.0
//
// Note that not all functions actually provide equivalent functionality to their
// Linux equivalents. They are simply stubs to allow the project to compile.
//
#ifndef __WINSTUBS_H
#define __WINSTUBS_H

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#include <basetsd.h>
#include <sys/timeb.h>
#include <io.h>
#include <malloc.h>
#include <share.h>
#include "win_time.h"

typedef UCHAR    uint8_t;
typedef USHORT   uint16_t;
typedef UINT32   uint32_t;
typedef UINT64   uint64_t;
typedef UINT32   mode_t;
typedef long     ssize_t;
typedef int      socklen_t;

#define M_PI 3.14159265358979323846

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_VS2013  // Define this in projects if using Visual Studio 2013 or higher
#define strtoll _strtoi64

#include <math.h>

_inline double round(double d) { return floor(d + 0.5); }
_inline double trunc(double d) { return (d>0) ? floor(d) : ceil(d); }
#endif

#define snprintf  _snprintf
#define vsnprintf _vsnprintf
#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp

#define  __attribute__(x)  /*NOTHING*/
#define inline _inline

#define PATH_MAX _MAX_PATH

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#ifdef __cplusplus
}
#endif

#endif // __WINSTUBS_H
