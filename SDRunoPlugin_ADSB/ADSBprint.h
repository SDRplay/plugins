#pragma once

#include <mutex>
#include <cstdio>
#include <cstdarg>

#ifdef DEBUGCONSOLE
#define DEBUG_PRINT(format, ...) \
{ \
ADSBprint::Timestamp(); \
ADSBprint::printf(format, __VA_ARGS__); \
};
#else
#define DEBUG_PRINT(format, ...) { }
#endif

class ADSBprint
{
public:

   static int open();
   static void close();
   static void printf(const char *Format, ...);
   static void wprintf(const wchar_t *Format, ...);

   static void Timestamp();

private:

   static std::mutex _mtx;
   static FILE *fp;

};
