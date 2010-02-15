#ifndef EOS_IO_COUNTER_H
#define EOS_IO_COUNTER_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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
