#ifndef EOS_TIME_TIMES_H
#define EOS_TIME_TIMES_H
//------------------------------------------------------------------------------
// Copyright 2003 Tom Haines

/// \file times.h
/// Provides all the time oriented functionality.

/// \namespace eos::time
/// The time namespace provides all time related functionality.
/// Also includes the progress bar system for monitoring time consuming tasks.

#include "eos/types.h"
#include "eos/io/inout.h"

namespace eos
{
 namespace time
 {
//------------------------------------------------------------------------------
/// Returns the time in seconds since the unix epoch, 00:00:00 January 1, 1970 GMT (UTC).
EOS_FUNC nat32 Time();

/// Returns the time in milliseconds since the unix epoch. Simply a more accurate 
/// version of above.
EOS_FUNC nat64 MilliTime();

/// Returns the time in seconds since an arbitary point, as accuratly as the OS
/// will report it.
EOS_FUNC real64 UltraTime();

/// Thread times, for the current thread, for profilling.
/// Outputs time in seconds in regards to the start of the thread in question.
EOS_FUNC void ThreadTime(real64 & user,real64 & system);

/// Returns the current timezone for the current computer, -12 to 12, inclusive.
/// Positive implies going west, negative going east, from greenwich. Does not 
/// include daylight saving time at all.
EOS_FUNC nat8 Timezone();

//------------------------------------------------------------------------------
// Definitions of various things...
/// Enumeration of the months.
enum Month {January,Febuary,March,April,May,June,July,August,September,October,November,December};

/// Returns a string that represents the given month.
EOS_FUNC cstrconst MonthStr(Month m);

/// Returns a short string (3 characters) that represents the given month.
EOS_FUNC cstrconst MonthStrShort(Month m);


/// Enumeration of the days of the week.
enum Day {Sunday,Monday,Tuesday,Wednessday,Thursday,Friday,Saturday};

/// Returns a string that represents the given day.
EOS_FUNC cstrconst DayStr(Day d);

/// Returns a short string (3 characters) that represents the given day.
EOS_FUNC cstrconst DayStrShort(Day d);


/// Used by the Date class for specifying how equal two date have to be.
/// For instance ByHour implies that the year, month, day and hour must
/// all be equal, but that minute and second do not matter.
enum DateEqual {ByYear,ByMonth,ByDay,ByHour,ByMinute,BySecond};

//------------------------------------------------------------------------------
/// The date class, used to represent a date. Relativly simple, not suitable for
/// any sophisticated date managment, but fine for simple tasks such as displaying
/// it.
class EOS_CLASS Date
{
 public:
  /// Constructor with defauolt values.
  /// \param locale If false returns GMT, else returns the time in the current locale. Defaults to GMT.
  /// \param time The time to set to, in secodns since the epoch. Defaults to now.
    Date(bit locale = false,nat32 time = eos::time::Time());
   ~Date();

  /// Sets the weekDay to the correct value relative to the rest of the given information.
   void Correct();

  // Returns the the time in seconds since unix epoc. Should only be called on GMT time classes, i.e. where locale==false.
   nat32 Time() const;
  
  /// Returns a cstring of this time/date. Format letters are:<br>
  /// Y = 4 digit year.<br>
  /// y = 2 digit year.<br>
  /// M = textual representation of the month.<br>
  /// m = numeric representation of the month.<br>
  /// d = numerical day of month, leading 0's.<br>
  /// D = textual day of the week.<br>
  /// H = 24 hour clock with leading 0's.<br>
  /// h = 12 hour clock with leading 0's.<br>
  /// G = 24 hour clock without leading 0's.<br>
  /// g = 12 hour clock without leading 0's.<br>
  /// i = minites, with leading 0's.<br>
  /// s = seconds, with leading 0's.<br>
  /// a = lowercase am or pm.<br>
  /// A = uppercase AM or PM.<br>
  /// Any unrecognised letters are copied straight over.
  /// Suports \ as an escape chracater - the next character following will just be copied over.
   cstrconst Str(cstrconst format = "H:i:s") const;
   
  /// Same as Str, except the string is owned by the user rather than the object.
  /// Remember to call Malloc at a latter date on the returned pointer.
   cstr MyStr(cstrconst format = "H:i:s") const;
  
   
  /// As the Str(...) method results in internal storage being allocated
  /// you might want to free it - this does that.
   void FreeIS();


  int16 year; ///< Year, will never be less than 1970.
  Month month; ///< 0..11
   int8 day; ///< Day of the month, 0 is the first.
    Day weekDay; ///< 0 == sunday.
   int8 hours; ///< 0..23
   int8 minites; ///< 0..59
   int8 seconds; ///< 0..59
   
  /// &nbsp;
   static inline cstrconst TypeString(){return "eos::time::Date";}

 private:
  mutable nat32 length;
  mutable cstr str;
};

//------------------------------------------------------------------------------
/// Simple structure used to represent a time of execution - specifically 
/// includes real time, user time and system time. All measurments are in 
/// seconds and can either be an offset from some arbitary point or a duration
/// depending on context.
struct EOS_CLASS RunningTime
{
 public:
  /// &nbsp;
   RunningTime():normal(0.0f),user(0.0f),system(0.0f){}
 
  void SetNow(); ///< Fills the class with times appropriate for now in the current thread.
  
  real64 normal; ///< Actual time, including everything else thats happenning.
  real64 user; ///< Time spent executing the thread, excluding operating system services to the thread.
  real64 system; ///< Time spent by the operating system servicing the thread.
  
  /// &nbsp;
   static inline cstrconst TypeString(){return "eos::time::RunningTime";} 	
};

//------------------------------------------------------------------------------
/// Timer class, for timing events. Acts like a stop watch, with Start and Stop
/// methods, and total which outputs the total time elapsed. Uses the RunningTime
/// structure. Is constructed in the stopped state.
class EOS_CLASS Timer
{
 public:
   Timer();
  ~Timer();

  /// Resets the total time back to zero, so it can time a new event. Can be 
  /// called whilst un-paused, in which case it remains un-paused and starts
  /// counting from the curent point.
   void Reset();
 
     
  /// Starts the timer. If allready started does nothing.
   void Start();
   
  /// Stops the timer. If allready stopped does nothing.
   void Stop();
   
  /// Returns true if its paused, false if its currently running.
   bit Paused(){return paused;}

   
  /// Outputs the total time elapsed whilst its been in the un-paused state.
   void Total(RunningTime & out);
   
  /// Used to save/load - only makes sense to use this when it is paused.
   RunningTime & GetTotal(){return total;}

  /// &nbsp;
   static inline cstrconst TypeString(){return "eos::time::Timer";} 	
   
 protected:
     bit paused;
  RunningTime start;
  RunningTime total;
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
// IO integration for the above classes, as sensible...
using namespace eos::time;

template <typename T>
inline T & StreamWrite(T & lhs,const Date & rhs,Binary)
{
 lhs << rhs.year << rhs.month << rhs.day << rhs.weekDay << rhs.hours << rhs.minites << rhs.seconds;
 return lhs;
}	 

template <typename T>
inline T & StreamRead(T & lhs,Date & rhs,Binary)
{
 lhs >> rhs.year >> rhs.month >> rhs.day >> rhs.weekDay >> rhs.hours >> rhs.minites >> rhs.seconds;
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const Date & rhs,Text)
{
 lhs << rhs.Str("Y-m-d H:i:s (D)");
 return lhs;
}


template <typename T>
inline T & StreamWrite(T & lhs,const RunningTime & rhs,Binary)
{
 lhs << rhs.normal << rhs.user << rhs.system;
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,RunningTime & rhs,Binary)
{
 lhs >> rhs.normal >> rhs.user >> rhs.system;
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const RunningTime & rhs,Text)
{
 lhs << "{normal = " << rhs.normal << ", user = " << rhs.user << ", system = " << rhs.system << "}";
 return lhs;
}


template <typename T>
inline T & StreamWrite(T & lhs,const Timer & rhs,Binary)
{
 RunningTime out; rhs.Total(out);
 lhs << out; 
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,Timer & rhs,Binary)
{
 RunningTime in;
 lhs >> in;
 rhs.GetTotal() = in;
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const Timer & rhs,Text)
{
 RunningTime out; rhs.Total(out);
 lhs << out; 
 return lhs;
}
	 
//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
#endif
