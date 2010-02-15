//------------------------------------------------------------------------------
// Copyright 2003 Tom Haines
#include "eos/time/times.h"

#include "eos/mem/alloc.h"
#include "eos/str/functions.h"

//------------------------------------------------------------------------------

#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>

#ifdef WIN32
 #include <windows.h>
#else
 #include <sys/times.h>
 #include <unistd.h>
 //#include <asm/msr.h>
#endif

namespace eos
{
 namespace time
 {
//------------------------------------------------------------------------------
EOS_FUNC nat32 Time()
{
 return ::time(0);
}

EOS_FUNC nat64 MilliTime()
{
 struct timeb t;
 ftime(&t);
 return nat64(t.time)*1000 + t.millitm;
}

EOS_FUNC real64 UltraTime()
{
 #ifdef WIN32
  LARGE_INTEGER time;
  LARGE_INTEGER freq;

  QueryPerformanceCounter(&time);
  QueryPerformanceFrequency(&freq);

  return real64(time.QuadPart)/real64(freq.QuadPart);	
 #else
  /*union // This code assumes an intel proccessor.
  {
   nat64 val;
   struct
   {
    nat32 low;
    nat32 high;
   } s;
  };
  val = 0;
  rdtsc(s.low,s.high);
  return real64(val)/real64();*/
  return real64(MilliTime())/1000.0; // Tempory, until I find a better way.
 #endif
}
EOS_FUNC void ThreadTime(real64 & user,real64 & system)
{
 #ifdef WIN32
  LARGE_INTEGER s;
  LARGE_INTEGER u;

  LARGE_INTEGER t;

  GetThreadTimes(GetCurrentThread(),(FILETIME*)&t,(FILETIME*)&t,(FILETIME*)&s,(FILETIME*)&u);

  system = real64(s.QuadPart)/10000000.0;
  user   = real64(u.QuadPart)/10000000.0;	
 #else
  struct tms tv;
  times(&tv);
  real64 tps = real64(sysconf(_SC_CLK_TCK));
  system = real64(tv.tms_stime)/tps;
  user = real64(tv.tms_utime)/tps;
 #endif
}

EOS_FUNC nat8 Timezone()
{
 tzset();
 return nat8(::timezone/(60*60));
}

//------------------------------------------------------------------------------
EOS_FUNC cstrconst MonthStr(Month m)
{
 const char * strs[12] = {"January","Febuary","March","April","May","June",
                          "July","August","September","October","November","December"};	
 return strs[m];
}

EOS_FUNC cstrconst MonthStrShort(Month m)
{
 const char * strs[12] = {"Jan","Feb","Mar","Apr","May","Jun",
                          "Jul","Aug","Sep","Oct","Nov","Dec"};
 return strs[m];
}

EOS_FUNC cstrconst DayStr(Day d)
{
 const char * strs[7] = {"Sunday","Monday","Tuesday","Wednessday","Thursday","Friday","Saturday"};	
 return strs[d];
}

EOS_FUNC cstrconst DayStrShort(Day d)
{
 const char * strs[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};	
 return strs[d];
}

//------------------------------------------------------------------------------
Date::Date(bit locale,nat32 time)
:length(0),str(null<cstr>())
{
 struct tm * t;

 time_t raw = time;
 if (locale) t = localtime(&raw);
        else t = gmtime(&raw);

 year    = t->tm_year+1900;
 month   = (Month)t->tm_mon;
 day	 = t->tm_mday-1;
 weekDay = (Day)t->tm_wday;
 hours   = t->tm_hour;
 minites = t->tm_min;
 seconds = t->tm_sec;
}

Date::~Date()
{
 mem::Free(str);
}

void Date::Correct()
{
 struct tm t;
  t.tm_sec   = seconds;
  t.tm_min   = minites;
  t.tm_hour  = hours;
  t.tm_mday  = day+1;
  t.tm_mon   = month;
  t.tm_year  = year+1900;

 mktime(&t);

 weekDay = Day(t.tm_wday);
}

nat32 Date::Time() const
{
 struct tm t;
  t.tm_sec   = seconds;
  t.tm_min   = minites;
  t.tm_hour  = hours;
  t.tm_mday  = day+1;
  t.tm_mon   = month;
  t.tm_year  = year+1900;
 return mktime(&t);
}

cstrconst Date::Str(cstrconst format) const
{
 using namespace eos::str;
 using namespace eos::mem;
 
 // First calculate the length of the final string...
  bit escape = false;
  cstrconst targ = format;
  nat32 len = 0;
  while (*targ)
  {
   if (escape) {len++; escape = false;}
   else
   {
    switch(*targ)
    {
     case 'Y': len += 4; break;
     case 'M': len += Length(MonthStr(month)); break;
     case 'D': len += Length(DayStr(weekDay)); break;
     case 'y': case 'm': case 'd': case 'H': case 'h': case 'G': case 'g': case 'i': case 's': case 'a': case 'A':
     len += 2; break;
     case '\\':
      escape = true;
     break;
     default:
      len++;
     break;
    }
   }
   targ++;
  }

 // If needed resize the internal string storage...
  if (length<len)
  {
   Free(str);
   str = Malloc<cstrchar>(len+1);
   length = len;
  }
  
 // Now we create the string were going to return...
  str[0] = 0;

  escape = false;
  cstrconst in = format;
  cstr out = str;
  while (*in)
  {
   if (escape)
   {
    *out = *in;
    *out++;
    *out = 0;
    escape = false;
   }
  else
   {
    switch(*in)
    {
     case 'Y':
      ToStr(year,out); // I'm guessing that it will be 4 digits - its unlikelly not to be!
     break;
     case 'y':
      if ((year%100)<10) {Append(out,"0");out++;}
      ToStr(eos::int32(year%100),out);
     break;
     case 'M':
      Append(out,MonthStr(month));
     break;
     case 'm':
      if ((month+1)<10) {Append(out,"0");out++;}
      ToStr(eos::int8(month+1),out);
     break;
     case 'D':
      Append(out,DayStr(weekDay));
     break;
     case 'd':
      if ((day+1)<10) {Append(out,"0");out++;}
      ToStr(eos::int8(day+1),out);
     break;
     case 'H':
      if (hours<10) {Append(out,"0");out++;}
      ToStr(hours,out);
     break;
     case 'h':
      if ((hours%12)<10) {Append(out,"0");out++;}
      ToStr(eos::int8(hours%12),out);
     case 'G':
      ToStr(hours,out);
     break;
     case 'g':
      ToStr(eos::int8(hours%12),out);
     break;
     case 'i':
      if (minites<10) {Append(out,"0");out++;}
      ToStr(minites,out);
     break;
     case 's':
      if (seconds<10) {Append(out,"0");out++;}
      ToStr(seconds,out);
     break;
     case 'a':
      if (hours<12) Append(out,"am");
               else Append(out,"pm");
     break;
     case 'A':
      if (hours<12) Append(out,"AM");
               else Append(out,"PM");
     case '\\':
      escape = true;
     break;
     default:
      *out = *in;
      out++;
      *out = 0;
     break;
    }
   }

   out += Length(out);
   in++;
  }

 return str;
}

cstr Date::MyStr(cstrconst format) const
{
 Str(format);
 
 cstr ret = str;
 str = null<cstr>();
 return ret;
}

void Date::FreeIS()
{
 mem::Free(str);
 str = null<cstr>();
}

//------------------------------------------------------------------------------
void RunningTime::SetNow()
{
 normal = UltraTime();
 ThreadTime(user,system);
}

//------------------------------------------------------------------------------
Timer::Timer()
:paused(true)
{}

Timer::~Timer()
{}

void Timer::Reset()
{
 total.normal = 0.0;
 total.user   = 0.0;
 total.system = 0.0;
 
 if (!paused)
 {
  start.SetNow();	 
 }
}

void Timer::Start()
{
 if (paused)
 {
  start.SetNow();
  paused = false;	 
 }	
}

void Timer::Stop()
{
 if (!paused)
 {
  RunningTime temp;
  temp.SetNow();
  
  total.normal += temp.normal - start.normal;
  total.user   += temp.user   - start.user;  
  total.system += temp.system - start.system;  
  
  paused = true;
 }
}

void Timer::Total(RunningTime & out)
{
 out = total;
 if (!paused)
 {
  RunningTime temp;
  temp.SetNow();
  
  out.normal += temp.normal - start.normal;
  out.user   += temp.user   - start.user;  
  out.system += temp.system - start.system;  	
 }
}

//------------------------------------------------------------------------------
 };
};
