#ifndef EOS_TIME_FORMAT_H
#define EOS_TIME_FORMAT_H
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


/// \file format.h
/// Provides specialised formating capability for time/date related stuff.

#include "eos/types.h"
#include "eos/io/out.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace time
 {
//------------------------------------------------------------------------------
// Dummy struct used by below.
struct SecondCount
{
 real64 seconds;
};

//------------------------------------------------------------------------------
/// Conversion function, this is called on a real64 when streaming it into a 
/// text stream to treat it as a measure in seconds. It then formats is 
/// appropriatly by scaling it into a suitable domain and adding the correct
/// scale indication. It returns two seperate units, i.e. seconds and 
/// milliseconds for instance.
/// The units given are:
/// - c - centurys. (Taken as 100*365.25 days.)
/// - y - years. (Taken as 365.25 days)
/// - w - weeks.
/// - d - days.
/// - h - hours.
/// - m - minutes.
/// - s - seconds.
/// - ms - milliseconds.
/// - us - microseconds.
/// - ns - nanoseconds.
EOS_FUNC inline SecondCount FormatSeconds(real64 seconds)
{
 SecondCount ret;
  ret.seconds = seconds;
 return ret;
}

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// I/O functions...
namespace eos
{
 namespace io
 {
 

  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::time::SecondCount & rhs,Text)
  {
   real64 seconds = rhs.seconds;
    if (seconds>=(60.0*60.0)) // Length of hour.
    {
     if (seconds>=(60.0*60.0*24.0*7.0)) // Length of week
     {
      if (seconds>=(60.0*60.0*24.0*365.25)) // length of year
      {
       if (seconds>=(60.0*60.0*24.0*365.25*100.0)) // Length of centuary
       {
        // Centuarys...
         nat32 years = nat32(math::RoundDown(seconds/(60.0*60.0*24.0*365.25)));
         nat32 centuarys = years / 100;
         years %= 100;
         
         lhs << centuarys << "c," << years << "y";
       }
       else
       {
        // Years...
         nat32 years = nat32(math::RoundDown(seconds/60.0*60.0*24.0*365.25));
         seconds -= years*(60.0*60.0*24.0*365.25);
         nat32 weeks = nat32(math::RoundDown(seconds/(60.0*60.0*24.0*7.0)));
         
         lhs << years << "y," << weeks << "w";
       }
      }
      else
      {
       // Weeks...
        nat32 days = nat32(math::RoundDown(seconds/(60.0*60.0*24.0)));
        nat32 weeks = days/7;
        days %= 7;
        
        lhs << weeks << "w," << days << "d";
      }
     }
     else
     {
      if (seconds>=(60.0*60.0*24.0)) // Length of day
      {
       // Days...
        nat32 hours = nat32(math::RoundDown(seconds/(60.0*60.0)));
        nat32 days = hours/24;
        hours %= 24;
        
        lhs << days << "d," << hours << "h";
      }
      else
      {
       // Hours...
        nat32 minutes = nat32(math::RoundDown(seconds/60.0));
        nat32 hours = minutes/60;
        minutes %= 60;
        
        lhs << hours << "h," << minutes << "m";
      }
     }
    }
    else
    {
     if (seconds>=(0.001)) // Length of milli second.
     {
      if (seconds>=(1.0))
      {
       if (seconds>=(60.0))
       {
        // Minutes...
         nat32 secs = nat32(math::RoundDown(seconds));
         nat32 minutes = secs/60;
         secs %= 60;
         
         lhs << minutes << "m," << secs << "s";
       }
       else
       {
        // Seconds...
         nat32 millisecs = nat32(math::RoundDown(seconds*1000.0));
         nat32 seconds = millisecs/1000;
         millisecs %= 1000;
         
         lhs << seconds << "s," << millisecs << "ms";
       }
      }
      else
      {
       // Milliseconds...
        nat32 microsecs = nat32(math::RoundDown(seconds*1000.0*1000.0));
        nat32 millisecs = microsecs/1000;
        microsecs %= 1000;
        
        lhs << millisecs << "ms," << microsecs << "us";
      }
     }
     else
     {
      if (seconds>=(0.000001)) // Length of microsecond
      {
       // Microseconds...
        nat32 nanosecs = nat32(math::RoundDown(seconds*1000.0*1000.0*1000.0));
        nat32 microsecs = nanosecs*1000;
        nanosecs %= 1000;
        
        lhs << microsecs << "us," << nanosecs << "ns";
      }
      else
      {
       // Nanoseconds...
        nat32 nanosecs = nat32(math::RoundDown(seconds*1000.0*1000.0*1000.0));
        lhs << nanosecs << "ns";       
      }
     }
    }
   return lhs;
  }


 };      
};
//------------------------------------------------------------------------------
#endif
