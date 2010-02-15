#ifndef EOS_STR_FUNCTIONS_H
#define EOS_STR_FUNCTIONS_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \namespace eos::str
/// Provides the string class and standard cstring functions.

/// \file str/functions.h
/// Provides a set of standard cstring functions, as a simple abstraction layer
/// to any actual implimentation. Also provides a whole load of extra stuff that
/// proven to be useful in the general sense.

#include "eos/types.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"

#include <string.h>
#include <stdlib.h>

namespace eos
{
 namespace str
 {
//------------------------------------------------------------------------------

/// Returns the length of a cstring.
EOS_FUNC inline nat32 Length(cstrconst str)
{
 return ::strlen(str);
}

/// Returns a pointer to the null that ends a null terminated string.
EOS_FUNC inline cstr End(cstr str)
{
 return str + ::strlen(str);
}

/// Copies a string into the given string, overwritting the current contents.
/// The dest string must be large enough to contain the sre string.
EOS_FUNC inline void Copy(cstr dest,cstrconst src)
{
 ::strcpy(dest,src);
}

/// Appends the given string onto the end of the dest string. The dest string 
/// must contain enough space to do this.
EOS_FUNC inline void Append(cstr dest,cstrconst src)
{
 ::strcat(dest,src);
}

/// Comparse two strings, returns 0 if they are identical, <0 if lhs is less than
/// rhs, >0 visa versa.
EOS_FUNC inline int32 Compare(cstrconst lhs,cstrconst rhs)
{
 return ::strcmp(lhs,rhs);
}

/// Duplicates a string, using mem::Malloc - you must call mem::Free on the returned 
/// value of this function, eventually.
EOS_FUNC inline cstr Duplicate(cstrconst str)
{
 nat32 len = Length(str);
 cstr ret = mem::Malloc<cstrchar>(len+1);
 mem::Copy(ret,const_cast<cstr>(str),len+1);
 return ret;       
}

/// Returns true if the first given string starts with the second given string.
EOS_FUNC inline bit AtStart(cstrconst str,cstrconst start)
{
 while (*start)
 {
  if (*start!=*str) return false;
  ++start; ++str;
 }
 return true;
}

/// Returns true if the first given string ends with the second given string.
EOS_FUNC inline bit AtEnd(cstrconst str,cstrconst end)
{
 nat32 strLen = Length(str);
 nat32 endLen = Length(end);
 if (strLen<endLen) return false;
 str += strLen-endLen;

 while (*str)
 {
  if (*end!=*str) return false;
  ++end; ++str;
 }
 return true;
}

//------------------------------------------------------------------------------

/// This converts the given thing to a cstring, requires that the buffer pointed
/// to contains enough for the maximum size of the output string. Requires the
/// object in question to impliment a void ToString(cstr out) method, or for this
/// function to have been overloaded for the type in question. All base types 
/// have been overloaded to work with this.
template <typename T>
inline void ToStr(const T & obj,cstr out)
{
 obj.ToString(out);
}

//------------------------------------------------------------------------------
// Helpers used below, not really for direct use...
template <typename T,int maxSize> 
inline void IntToStr(T val,cstr res)
{
 // We build it in reverse, and then reverse it when writting it out...
 cstrchar buf[maxSize]; 
 cstr out = buf;

 do
 {
  *out = '0' + val%10;   
  val = val/10;
  ++out;
 }
 while (val!=0);
 
 while (out>buf)
 {
  --out;
  *res = *out;
  ++res;
 }
 *res = 0;
}

template <typename T,int maxSize> 
inline void IntToStrNeg(T val,cstr res)
{
 // We build it in reverse, and then reverse it when writting it out...
 cstrchar buf[maxSize]; 
 cstr out = buf;
 bool neg;
 if (val<0)
 {
  val = -val; 
  neg = true;
 } else neg = false;

 do
 {
  *out = '0' + val%10;   
  val = val/10;
  ++out;
 }
 while (val!=0);
 
 if (neg)
 {
  *out = '-';
  ++out;
 }
 
 while (out>buf)
 {
  --out;
  *res = *out;
  ++res;
 }
 *res = 0;
}

//------------------------------------------------------------------------------
// Overloaded versions of the above to string template, for all the basic types...
template <>
inline void ToStr<nat8>(const nat8 & obj,cstr out)
{
 IntToStr<nat8,3>(obj,out);
}

template <>
inline void ToStr<nat16>(const nat16 & obj,cstr out)
{
 IntToStr<nat16,5>(obj,out);
}

template <>
inline void ToStr<nat32>(const nat32 & obj,cstr out)
{
 IntToStr<nat32,10>(obj,out);
}

template <>
inline void ToStr<nat64>(const nat64 & obj,cstr out)
{
 IntToStr<nat64,20>(obj,out);
}

template <>
inline void ToStr<int8>(const int8 & obj,cstr out)
{
 IntToStrNeg<int8,4>(obj,out);
}

template <>
inline void ToStr<int16>(const int16 & obj,cstr out)
{
 IntToStrNeg<int16,6>(obj,out);
}

template <>
inline void ToStr<int32>(const int32 & obj,cstr out)
{
 IntToStrNeg<int32,11>(obj,out);
}

template <>
inline void ToStr<int64>(const int64 & obj,cstr out)
{
 IntToStrNeg<int64,20>(obj,out);
}

template <>
inline void ToStr<real32>(const real32 & obj,cstr out)
{
 ::gcvt(obj,8,out);
}

template <>
inline void ToStr<real64>(const real64 & obj,cstr out)
{
 ::gcvt(obj,8,out);
}

template <>
inline void ToStr<bit>(const bit & obj,cstr out)
{
 if (obj) Copy(out,"true");
     else Copy(out,"false");
}

//------------------------------------------------------------------------------
// Helper template for below...
template <typename T> 
inline void IntToHex(T val,cstr res)
{
 const nat32 maxSize = sizeof(T)*2 + 1;
 nat32 shift = sizeof(T);
 for (nat32 i=0;i<maxSize;i+=2)
 {
  byte p = byte(val>>(--shift));
  res[i]   = "0123456789ABCDEF"[p>>4];
  res[i+1] = "0123456789ABCDEF"[p&0x0F];
 }
 res[maxSize] = 0;
}

//------------------------------------------------------------------------------
/// This converts a 8 bit natural to a string, out should be at least 3 bytes in size.
inline void ToHex(nat8 obj,cstr out)
{
 IntToHex<nat8>(obj,out);        
}

/// This converts a 16 bit natural to a string, out should be at least 5 bytes in size.
inline void ToHex(nat16 obj,cstr out)
{
 IntToHex<nat16>(obj,out);        
}

/// This converts a 32 bit natural to a string, out should be at least 9 bytes in size.
inline void ToHex(nat32 obj,cstr out)
{
 IntToHex<nat32>(obj,out);        
}

/// This converts a 64 bit natural to a string, out should be at least 17 bytes in size.
inline void ToHex(nat64 obj,cstr out)
{
 IntToHex<nat64>(obj,out);        
}

//------------------------------------------------------------------------------
EOS_VAR int8 CharToHexMap[256];

/// This function is given a single character and returns an integer, -1 if the 
/// character given is not a hex code or 0..15 as its relevent hex value.
inline int8 HexToVal(cstrchar c) 
{
 return CharToHexMap[nat8(c)];
}

//------------------------------------------------------------------------------
/// This converts a string (null terminator not needed) to a structure, must be 
/// given its return natural. The input string must have sizeof(T)*2 chars to it.
template <typename T>
inline T FromHex(cstrconst str,bit & err)
{
 T ret = 0;
 err = false;
  for (nat32 i=0;i<sizeof(T);i++)
  {
   int8 a = HexToVal(str[i*2]);
   int8 b = HexToVal(str[i*2+1]);
   if ((a==-1)||(b==-1)) {err = true; break;}
   
   ret |= ((a<<4) | b) << (sizeof(T)-i-1);    
  } 
 return ret;
}

//------------------------------------------------------------------------------
/// Converts a c string to an integer, if the string is not an integer it
/// transilates as much as possible, but will fail on 0 as its return value.
inline int32 ToInt32(cstrconst str)
{
 return int32(atoi(str));
}

/// Converts a C string to a real, if the string is not a real it will transilate
/// as much as possible, but fail on 0.0 as its return value.
inline real32 ToReal32(cstrconst str)
{
 return real32(atof(str));
}

//------------------------------------------------------------------------------
 };
};
#endif
