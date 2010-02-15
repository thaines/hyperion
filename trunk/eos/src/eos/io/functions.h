#ifndef EOS_IO_FUNCTIONS_H
#define EOS_IO_FUNCTIONS_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file io/functions.h
/// Provides some useful functions for io stuff, primarily endian handling.

#include "eos/types.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
/// Converts from big endian to the current platform.
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
 };
};
#endif
