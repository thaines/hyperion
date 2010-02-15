#ifndef EOS_LOG_LOGS_H
#define EOS_LOG_LOGS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


/// \file logs.h
/// Contains the logging system. Rather messy, due to all the dam circular dependency problems.

/// \namespace eos::log
/// This namespace contains all logging facilities. Despite being treated as as
/// a low level system it in fact a heavy user of some of the more advanced
/// modules in the system.

#include "eos/types.h"

namespace eos
{
 namespace file
 {
  class Csv;
 }
 namespace time
 {
  class Date;
 }
 
 struct LogDivObject {};
 
 /// A null object, passed to the logging system to indicate breaks between
 /// columns.
 /// Kept outside the logging system as it gets typed in too often to be 
 /// qualifying its namespace.
 inline LogDivObject LogDiv() {LogDivObject ret; return ret;} 
 
 namespace log
 {
//------------------------------------------------------------------------------
// The logging singlton, acts as a lockable cvs file that creates itself on
// first use, is flushed on unlock and records a whole host of default
// information (id, date/time, thread id, depth). Changes log file when the date
// changes, so you get a log file for each day, created in the log directory it
// automatically creates in the working directory.
class EOS_CLASS LogObj
{
 public:
  LogObj();
  ~LogObj();
   
  
  // Returns a log file to stream to, adds all the default stuff.
   file::Csv & BeginEntry();
   
  // Call when logging is finsihed. Do not forget, as there is locking.
   void EndEntry();
   
  // Adds some profiling information. Note that we use the pointer to 
  // the string as an index, not the index itself. We can get away with 
  // this as it has to be a constant hard typed string, so each one must
  // have a constant unique pointer.
   void StoreBlock(cstrconst name,real64 real,real64 user,real64 system); 
   
  // For access.
   nat32 & Depth() {return depth;}


 private:
  nat32 n; // Log entry number. Blah.
  nat32 depth;
  class file::Csv * out; // Current output file. Ushally null.
  class LogStore * ls; // Has to be declared like this to avoid circular dependency problems.

  // Helpers...
   void CreateFile(time::Date * d);
};

//------------------------------------------------------------------------------

EOS_VAR LogObj logger;

//------------------------------------------------------------------------------
// The object used for logging blocks of execution, ushally entire functions...
// Create the exiting log entry but not the starting entry. 
class EOS_CLASS BlockLogger
{
 public:
   BlockLogger(cstrconst name,bit le);
  ~BlockLogger();
  
 private:
  cstrconst name;
  bit logEnd;

  real64 real;
  real64 user;
  real64 system;
};

//------------------------------------------------------------------------------
/// \def LogLog(x)
/// Each logging request takes the form of a macro call containing a stream as
/// a sequence of << without a head object, the macro adds that in itself.
/// A log::Div() function is provided which will act as a divider between columns
/// in the csv file created by the logging system - this is useful for creating
/// output you might want to sort the log by.
/// For example,
///
/// LogLog("[mem.used] Current consumption {current,commit}" << log::Div() << current << log::Div() << commit);
///
/// This indicates three conventions that should be used at all times, so a
/// program can disect a log and analyse it.
/// - The first field should primarilly be a text description of the log entry for human consumption.
/// - A . seperated hierachy inside [] can open a log entry description, to classify entrys for selective viewing.
/// - A {} contained comma seperated list of column names can end the entry description, providing names to the following columns to provide meaning.
///
/// The LogLog version of the logging macro allways works, regardless of build. It
/// should never be used directly except for tempory logging. (Its called LogLog so
/// as to not clash with the mathematical Log.)

/// \def LogAlways(x)
/// See LogLog(x) for details of usage. This logging macro allways works.

/// \def LogDebug(x)
/// See LogLog(x) for details of usage. This logging macro allways works in debug builds.

/// \def LogMinor(x)
/// See LogLog(x) for details of usage. This logging macro optionally works when EOS_LOG_MINOR is defined.

/// \def LogBlock(n,p)
/// A variant on the ushall logging, usage is similar to the LogLog(x) macros with
/// p being identical in usage to the x and n a compile time constant string.
/// This version differs in that
/// it logs both when the current code block starts and ends. The start has
/// both n and p in the log, whilst the end has n only. It also records the time
/// consumed in the profiling system under the key n. This has to be manually
/// switched on for compilation, with EOS_LOG_BLOCKS. 
/// Only one can exist in any given block, as it creates a local variable,
/// and for timing to mean much it should be the first item in the block.
/// As a useful little trick a global depth parameter is maintained for each
/// thread, its incrimented with each LogBlock entry and decrimented with each
/// exit, and recorded with each log entry.
///
/// Traditional usage is to log a function, by example:
/// LogBlock("Function(nat32 x,nat32 y)",x << log::Div() << y);

/// \def LogTime(n)
/// This is identical to LogBlock(n,p), except it does not log entry/exit of each block.
/// This is more conveniant for profiling purposes where a log entry for each call 
/// is too major a slow down/memory hog/profiling distorter.

//------------------------------------------------------------------------------

#define LogLog(x) {eos::log::logger.BeginEntry() << x; eos::log::logger.EndEntry();}

#define LogAlways(x) LogLog(x)
#define LogError(x) LogLog(x)

#ifdef EOS_DEBUG
 #define LogDebug(x) LogLog(x)
#else
 #define LogDebug(x)
#endif

#ifdef EOS_LOG_MINOR
 #define LogMinor(x)
#else
 #define LogMinor(x)
#endif

#ifdef EOS_LOG_BLOCKS
 #define LogBlock(n,p) eos::log::logger.Depth() += 1; LogLog("[block] Begin {name,details}" << LogDiv() << n << LogDiv() << p); eos::log::BlockLogger eosLogBlockLoggerInstance(n,true)
 #define LogTime(n) eos::log::logger.Depth() += 1; eos::log::BlockLogger eosLogBlockLoggerInstance(n,false)
#else
 #define LogBlock(n,p)
 #define LogTime(n)
#endif

//------------------------------------------------------------------------------
EOS_FUNC void AssertFailure(cstrconst error);

/// \fn inline inline void Assert(bit test,cstrconst error)
/// A standard assertion function, only compiled in for debug versions, calls
/// abort if given false so the debuger can take control.
#ifdef EOS_DEBUG
 inline void Assert(bit test,cstrconst error = "Unspecified")
 {
  if (test==false) AssertFailure(error);   
 } 
#else
 inline void Assert(bit test,cstrconst error = "Bite my shiny metal arse") {}
#endif

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// I/O functions...
namespace eos
{
 namespace io
 {
 
 
  template <typename T,typename T2>
  inline T & StreamWrite(T & lhs,const eos::LogDivObject & rhs,T2)
  {
   lhs.FieldEnd();
   return lhs;
  }


 };      
};
//------------------------------------------------------------------------------
#endif
