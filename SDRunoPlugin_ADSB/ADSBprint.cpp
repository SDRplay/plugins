#include <cstdlib>
#include "ADSBprint.h"
#include "winstubs.h"

std::mutex ADSBprint::_mtx;
FILE *ADSBprint::fp = NULL;

int ADSBprint::open()
{
   std::lock_guard<std::mutex> _(_mtx);

   if (fp == NULL)
   {
      fp = _fsopen("C:\\tmp\\ADSBplugin.log", "w", _SH_DENYWR);
      if (fp == NULL)
      {
         return 1;
      }
   }
   return 0;
}

void ADSBprint::close()
{
   std::lock_guard<std::mutex> _(_mtx);

   if (fp != NULL)
   {
      fclose(fp);
      fp = NULL;
   }
}

void ADSBprint::printf(const char *Format, ...)
{
   std::lock_guard<std::mutex> _(_mtx);

   if (fp == NULL)
   {
      return;
   }

   va_list Args;
   va_start(Args, Format);
   vfprintf_s(fp, Format, Args);
   fflush(fp);
   va_end(Args);
   return;
}

void ADSBprint::wprintf(const wchar_t *Format, ...)
{
   std::lock_guard<std::mutex> _(_mtx);

   if (fp == NULL)
   {
      return;
   }

   va_list Args;
   va_start(Args, Format);
   vfwprintf_s(fp, Format, Args);
   fflush(fp);
   va_end(Args);
   return;
}

void ADSBprint::Timestamp(void)
{

	struct timeval tv;
	time_t nowtime;
	struct tm* nowtm;
	char tmbuf[64], buf[64];

	gettimeofday(&tv, NULL);
	nowtime = tv.tv_sec;
	nowtm = localtime(&nowtime);
	strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
	snprintf(buf, sizeof buf, "%s.%06ld", tmbuf, tv.tv_usec);
	ADSBprint::printf("%s ", buf);
}

