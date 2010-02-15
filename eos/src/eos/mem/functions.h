#ifndef EOS_MEM_FUNCTIONS_H
#define EOS_MEM_FUNCTIONS_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

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


/// \file mem/functions.h
/// Contains templated equivalents to C style memory manipulation 
/// functions, plus a few extras. (Templated to be typesafe.)

#include "eos/types.h"

#include <string.h>

namespace eos
{
 namespace mem
 {
//------------------------------------------------------------------------------
/// Nulls a block of memory. Takes a pointer to a data type and a number of 
/// these types to null, which defaults to 1. (i.e. sizeof(T)*num bytes will be 
/// nulled.)
template <typename T>
inline void Null(T * data,nat32 num = 1)
{
 ::memset(data,0,sizeof(T)*num);
}

/// The reverse of Null, sets every bit to true.
template <typename T>
inline void NotNull(T * data,nat32 num = 1)
{
 ::memset(data,0xFFFFFFFF,sizeof(T)*num);
}

/// This copys data from one given pointer to another. Undefined in the event 
/// the data overlaps. num defines how many complete objects to copy, and 
/// defaults to 1, i.e. sizeof(T)*num bytes are shifted.
template <typename T>
inline void Copy(T * to,const T * from,nat32 num = 1)
{
 ::memcpy(to,from,sizeof(T)*num);
}

/// Identical to Copy, except it handles overlaping data blocks correctly.
template <typename T>
inline void Move(T * to,const T * from,nat32 num = 1)
{
 ::memmove(to,from,sizeof(T)*num);
}

/// Compares two blocks of memory as bytes (unsigned), a byte at a time till they
/// differ. Returns 0 if they are equal, <0 if the first differing byte is lesser
/// for lhs. >0 if the first differing byte is less for rhs.
template <typename T>
inline int32 Compare(const T * lhs,const T * rhs,nat32 num = 1)
{
 return ::memcmp(lhs,rhs,sizeof(T)*num);	
}

/// Swaps two blocks of memory efficiently (-ish).
template <typename T>
inline void Swap(T * lhs,T * rhs,nat32 num = 1)
{
 byte * a = (byte*)lhs;
 byte * b = (byte*)rhs;
 nat32 n = sizeof(T)*num;
 
 while (n>=4)
 {
  nat32 t = *(nat32*)a;
  *(nat32*)a = *(nat32*)b;
  *(nat32*)b = t;

  a += 4;
  b += 4;
  n -= 4;	 
 }
 
 while (n>=1)
 {
  byte t = *a;
  *a = *b;
  *b = t;
  
  ++a;
  ++b;
  --n;
 }
}

/// Swaps two pointers arround. (I have no idea why this exists, but havn't 
/// deleted it as its presumably used somewhere in my code.)
template <typename T>
inline void PtrSwap(T *& lhs,T *& rhs)
{
 T * temp = lhs;
 lhs = rhs;
 rhs = temp;
}

//------------------------------------------------------------------------------
/// This is for improving cache performance. This will prefetch data into the 
/// cache, if called some time before the use of the data this will save time 
/// when the program reaches that point as the data will allready be in the 
/// cache. The size of the block grabed is unspecified, but can ushally be 
/// expected to be a cache line, which is ushally at least 32 bytes.
///
/// This version is for if you are intending to write rather than read, if 
/// you are intending to do both this is the version to use.
/// This version is for a one-off use of the data, as opposed to repeated 
/// use.
template <typename T>
inline void PrefetchWriteOnce(T * ptr)
{
 __builtin_prefetch(ptr,1,0);
}

/// This is for improving cache performance. This will prefetch data into the 
/// cache, if called some time before the use of the data this will save time 
/// when the program reaches that point as the data will allready be in the 
/// cache. The size of the block grabed is unspecified, but can ushally be 
/// expected to be a cache line, which is ushally at least 32 bytes.
///
/// This version is for if you are intending to write rather than read, if 
/// you are intending to do both this is the version to use.
/// This version is for a repeated use of data, rather than a one off use.
template <typename T>
inline void PrefetchWrite(T * ptr)
{
 __builtin_prefetch(ptr,1,3);
}

/// This is for improving cache performance. This will prefetch data into the 
/// cache, if called some time before the use of the data this will save time 
/// when the program reaches that point as the data will allready be in the 
/// cache. The size of the block grabed is unspecified, but can ushally be 
/// expected to be a cache line, which is ushally at least 32 bytes.
///
/// This version is for if you are intending to read rather than write.
/// This version is for a one-off use of the data, as opposed to repeated 
/// use.
template <typename T>
inline void PrefetchReadOnce(T * ptr)
{
 __builtin_prefetch(ptr,0,0);
}

/// This is for improving cache performance. This will prefetch data into the 
/// cache, if called some time before the use of the data this will save time 
/// when the program reaches that point as the data will allready be in the 
/// cache. The size of the block grabed is unspecified, but can ushally be 
/// expected to be a cache line, which is ushally at least 32 bytes.
///
/// This version is for if you are intending to read rather than write.
/// This version is for a repeated use of data, rather than a one off use.
template <typename T>
inline void PrefetchRead(T * ptr)
{
 __builtin_prefetch(ptr,0,3);
}

//------------------------------------------------------------------------------
 };
};
#endif
