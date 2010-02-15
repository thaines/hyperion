#ifndef EOS_IO_FUNCTIONS_H
#define EOS_IO_FUNCTIONS_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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


/// \file io/functions.h
/// Provides some useful functions for io stuff, primarily endian handling.

#include "eos/types.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
/// Converts from big endian (Motorola) to the current platform.
template<typename T>
T FromBigEndian(T v);

template <>
inline int8 FromBigEndian<int8>(int8 v) {return v;}

template <>
inline int16 FromBigEndian<int16>(int16 v)
{
 byte * t = (byte*)(void*)&v;
 math::Swap(t[0],t[1]);
 return v;
}

template <>
inline int32 FromBigEndian<int32>(int32 v)
{
 byte * t = (byte*)(void*)&v;
 math::Swap(t[0],t[3]);
 math::Swap(t[1],t[2]);
 return v;
}

template <>
inline int64 FromBigEndian<int64>(int64 v)
{
 byte * t = (byte*)(void*)&v;
 math::Swap(t[0],t[7]);
 math::Swap(t[1],t[6]);
 math::Swap(t[2],t[5]);
 math::Swap(t[3],t[4]);
 return v;
}


template <>
inline nat8 FromBigEndian<nat8>(nat8 v) {return v;}

template <>
inline nat16 FromBigEndian<nat16>(nat16 v)
{
 byte * t = (byte*)(void*)&v;
 math::Swap(t[0],t[1]);
 return v;
}

template <>
inline nat32 FromBigEndian<nat32>(nat32 v)
{
 byte * t = (byte*)(void*)&v;
 math::Swap(t[0],t[3]);
 math::Swap(t[1],t[2]);
 return v;
}

template <>
inline nat64 FromBigEndian<nat64>(nat64 v)
{
 byte * t = (byte*)(void*)&v;
 math::Swap(t[0],t[7]);
 math::Swap(t[1],t[6]);
 math::Swap(t[2],t[5]);
 math::Swap(t[3],t[4]);
 return v;
}

//------------------------------------------------------------------------------
/// Converts from the current platform to big endian.
template<typename T>
inline T ToBigEndian(T v)
{
 return	FromBigEndian(v); // Its nice when the inverse and forward operations are identical:-)
}

//------------------------------------------------------------------------------
/// Converts to the current platform given a bit to indicate endiness - true for
/// big (motorola), false for little (intel).
template<typename T>
inline T ToCurrent(T v,bit big)
{
 if (big) return FromBigEndian(v);
     else return v;
}

//------------------------------------------------------------------------------
/// Fetches aa int16 from a byte stream.
inline int16 Int16FromBS(byte * bs)
{
 return int16(bs[0]) | (int16(bs[1])<<8);
}

/// Fetches an int32 from a byte stream.
inline int32 Int32FromBS(byte * bs)
{
 return int32(bs[0]) | (int32(bs[1])<<8) | (int32(bs[2])<<16) | (int32(bs[3])<<24);
}

/// Fetches a nat16 from a byte stream.
inline nat16 Nat16FromBS(byte * bs)
{
 return nat16(bs[0]) | (nat16(bs[1])<<8);
}

/// Fetches a nat32 from a byte stream.
inline nat32 Nat32FromBS(byte * bs)
{
 return nat32(bs[0]) | (nat32(bs[1])<<8) | (nat32(bs[2])<<16) | (nat32(bs[3])<<24);
}

//------------------------------------------------------------------------------
 };
};
#endif

