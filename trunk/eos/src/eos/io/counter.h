#ifndef EOS_IO_COUNTER_H
#define EOS_IO_COUNTER_H
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


/// \file counter.h
/// Provides a simple out streaming class, that counts how many bytes have been 
/// written without actually doing anything with them.
/// Generally used when you need to know how big something is before you write 
/// it, or when you want to choose how many of something to write out to fill a
/// limited space.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/io/out.h"

namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
/// A simple out class that simply counts what is written to it, without 
/// actually doing anything with the data.
template <typename ET>
class EOS_CLASS Counter : public Out<ET>
{
 public:
  /// &nbsp;
   Counter():sum(0) {}

  /// &nbsp;
   ~Counter() {}


  /// Returns a reference to the number of bytes written, so you can read it and set it.
   nat32 & Sum() {return sum;}


  /// &nbsp;
   nat32 Write(const void * in,nat32 bytes) {sum += bytes; return bytes;}

  /// &nbsp;
   nat32 Pad(nat32 bytes) {sum += bytes; return bytes;}


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::io::Counter<" << typestring<ET>() << ">");
    return ret;
   }


 private:
  nat32 sum;
};

//------------------------------------------------------------------------------
 };
};
#endif
