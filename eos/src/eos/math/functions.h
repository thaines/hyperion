#ifndef EOS_MATH_FUNCTIONS_H
#define EOS_MATH_FUNCTIONS_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \file math/functions.h
/// \brief Contains mathematical functions of value.

#include "eos/types.h"

#include <math.h>
#include <stdlib.h>

#include "eos/math/constants.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// Swaps two values, in the maths library as its the best place for it, and
/// quite a few operations here use it internally anyway. Note that there is
/// also mem::Swap for pointers to arrays of values.
template <typename T>
inline void Swap(T & a,T & b)
{
 T temp = a; a = b; b = temp;
}

//------------------------------------------------------------------------------
/// Calculates the sin of an angle given in radians.
template <typename T>
inline T Sin(T val)
{
 return static_cast<T>(::sin(val));
}

/// Calculates the cosine of an angle given in radians.
template <typename T>
inline T Cos(T val)
{
 return static_cast<T>(::cos(val));
}

/// Calculates the tangent of an angle given in radians.
template <typename T>
inline T Tan(T val)
{
 return static_cast<T>(::tan(val));
}

/// Calculates 1 divided by the sin of an angle given in radians.
template <typename T>
inline T Csc(T val)
{
 return static_cast<T>(1.0/::sin(val));
}

/// Calculates 1 divided by the cosine of an angle given in radians.
template <typename T>
inline T Sec(T val)
{
 return static_cast<T>(1.0/::cos(val));
}

/// Calculates 1 divided by the tangent of an angle given in radians.
template <typename T>
inline T Cot(T val)
{
 return static_cast<T>(1.0/::tan(val));
}


/// Calculates the both sin and cosine of an angle, given in radians.
/// \param val The input value, in radians.
/// \param outSin Output value, the sin of the input.
/// \param outCos Output value, the cos of the input.
template <typename T>
inline void SinCos(T val,T & outSin,T & outCos)
{
 outSin = static_cast<T>(::sin(val));
 outCos = static_cast<T>(::cos(val));
}


/// Calculates the inverse sin, output given in radians.
template <typename T>
inline T InvSin(T val)
{
 return static_cast<T>(::asin(val));
}

/// Calculates the inverse cosine, output given in radians.
template <typename T>
inline T InvCos(T val)
{
 return static_cast<T>(::acos(val));
}

/// Calculates the inverse tangent, output given in radians.
template <typename T>
inline T InvTan(T val)
{
 return static_cast<T>(::atan(val));
}

/// Calculates the inverse tangent a/b, output given in radians.
/// In additon it uses the signs to calculate which quadrant the
/// result is in and has no problem with b==0.
/// Output goes from -pi to +pi. Will not work if both a and b
/// are 0, but all other real inputs are acceptable.
template <typename T>
inline T InvTan2(T a,T b)
{
 return static_cast<T>(::atan2(a,b));
}

/// Calculates the sinh of an angle given in radians.
template <typename T>
inline T Sinh(T val)
{
 return static_cast<T>(::sinh(val));
}

/// Calculates the cosh of an angle given in radians.
template <typename T>
inline T Cosh(T val)
{
 return static_cast<T>(::cosh(val));
}

/// Calculates the tanh of an angle given in radians.
template <typename T>
inline T Tanh(T val)
{
 return static_cast<T>(::tanh(val));
}

/// Calculates 1 divide by the sinh of an angle given in radians.
template <typename T>
inline T Csch(T val)
{
 return static_cast<T>(1.0/::sinh(val));
}

/// Calculates 1 divide by the cosh of an angle given in radians.
template <typename T>
inline T Sech(T val)
{
 return static_cast<T>(1.0/::cosh(val));
}

/// Calculates 1 divide by the tanh of an angle given in radians.
template <typename T>
inline T Coth(T val)
{
 return static_cast<T>(1.0/::tanh(val));
}

/// Calculates the inverse sinh, outputs in radians.
template <typename T>
inline T InvSinh(T val)
{
 return static_cast<T>(::asinh(val));
}

/// Calculates the inverse cosh, outputs in radians.
template <typename T>
inline T InvCosh(T val)
{
 return static_cast<T>(::acosh(val));
}

/// Calculates the inverse sinh, outputs in radians.
template <typename T>
inline T InvTanh(T val)
{
 return static_cast<T>(::atanh(val));
}


/// Does exponentiation.
/// \param a First value
/// \param b Second value
/// \returns a^b (a to the power of b)
template <typename T>
inline T Pow(T a,T b)
{
 return static_cast<T>(::pow(a,b));
}

/// Squares a value, i.e. equivalent to Pow(val,2)
template <typename T>
inline T Sqr(T val)
{
 return val*val;
}

/// Cubes a value, i.e. equivalent to Pow(val,3)
template <typename T>
inline T Cube(T val)
{
 return val*val*val;
}

/// Calculates the square root.
template <typename T>
inline T Sqrt(T val)
{
 return static_cast<T>(::sqrt(val));
}

/// Calculates the inverse square root, i.e. 1/sqrt.
template <typename T>
inline T InvSqrt(T val)
{
 return static_cast<T>(static_cast<T>(1)/::sqrt(val));
}

/// Calculates log of a, base b.
template <typename T>
inline T Log(T a,T b)
{
 return static_cast<T>(::log(a)/::log(b));
}

/// Does the natural logarithm, i.e. log base e.
template <typename T>
inline T Ln(T val)
{
 return static_cast<T>(::log(val));
}

/// Calculates e to the power of the given value.
template <typename T>
inline T Exp(T val)
{
 return static_cast<T>(::exp(val));
}

/// Does the logarithm base 10.
template <typename T>
inline T Log10(T val)
{
 return static_cast<T>(::log10(val));
}


/// Calculates the triangular number of the given input.
template <typename T>
inline T Triangular(T val)
{
 T ret = T(0);
  for (nat32 i=1;T(i)<=val;i++) ret += T(i);
 return ret;
}

/// Calculates the factorial of the given input.
template <typename T>
inline T Factorial(T val)
{
 T ret = T(1);
  for (nat32 i=2;T(i)<=val;i++) ret *= T(i);
 return ret;
}

/// Calculates the length of a vector with the dimensions given,
/// i.e. sqrt(x*x + y*y).
template <typename T>
inline T Length2(T x,T y)
{
 return static_cast<T>(::hypot(x,y));
}

/// Similar to length2, but with 3 dimensions, i.e. calculates sqty(x*x + y*y + z*z).
template <typename T>
inline T Length3(T x,T y,T z)
{
 return static_cast<T>(::hypot(::hypot(x,y),z));
}


/// Constructs a real from the given numbers.
/// \returns val * 2^exp
template <typename T>
inline T MakeReal(T val,int32 exp)
{
 return static_cast<T>(::ldexp(val,exp));
}

/// Divides a real up into its components.
/// \param val The input value, val= outMult * 2^outExp
/// \param outMult A value in the range [0.5,1)
/// \param outExp The exponent.
template <typename T>
inline T SplitReal(T val,T & outMult,int32 & outExp)
{
 outMult = static_cast<T>(::frexp(val,&outExp));
}

/// Calculates modulus, x mod y, the remainder.
/// \returns Returns f, where x = a*y + f for an integer a. a is the largest
///          integer that does not produce a negative f.
template <typename T>
inline T Mod(T x,T y)
{
 return static_cast<T>(::fmod(x,y));
}

/// This returns the modulus of a number with 1, i.e. the bit after the decimal
/// point.
template <typename T>
inline T Mod1(T x)
{
 return Mod(x,static_cast<T>(1));
}

/// Calculates the absolute value. This is overloaded for nearly all integer
/// types as well, to make it efficient for them.
template <typename T>
inline T Abs(T val)
{
 return static_cast<T>(::fabs(val));
}


/// This returns the largest integer smaller or equal to the given value.
template <typename T>
inline T RoundDown(T val)
{
 return static_cast<T>(::floor(val));
}

/// This returns the smallest integer greater or equal to the given value.
template <typename T>
inline T RoundUp(T val)
{
 return static_cast<T>(::ceil(val));
}

/// This returns the given value rounded to the nearest integer.
template <typename T>
inline T Round(T val)
{
 return static_cast<T>(::ceil(val-static_cast<T>(0.5)));
}


/// Returns the minimum of two values.
template <typename T>
inline const T & Min(const T & a,const T & b)
{
 if (a<b) return a;
     else return b;
}

/// Returns the minimum of three values.
template <typename T>
inline const T & Min(const T & a,const T & b,const T & c)
{
 return Min(Min(a,b),c);
}

/// Returns the minimum of four values.
template <typename T>
inline const T & Min(const T & a,const T & b,const T & c,const T & d)
{
 return Min(Min(Min(a,b),c),d);
}

/// Returns the minimum of five values.
template <typename T>
inline const T & Min(const T & a,const T & b,const T & c,const T & d,const T & e)
{
 return Min(Min(Min(Min(a,b),c),d),e);
}

/// Returns the minimum of six values.
template <typename T>
inline const T & Min(const T & a,const T & b,const T & c,const T & d,const T & e,const T & f)
{
 return Min(Min(Min(Min(Min(a,b),c),d),e),f);
}

/// Returns the minimum of seven values.
template <typename T>
inline const T & Min(const T & a,const T & b,const T & c,const T & d,const T & e,const T & f,const T & g)
{
 return Min(Min(Min(Min(Min(Min(a,b),c),d),e),f),g);
}

/// Returns the minimum of eight values.
template <typename T>
inline const T & Min(const T & a,const T & b,const T & c,const T & d,const T & e,const T & f,const T & g,const T & h)
{
 return Min(Min(Min(Min(Min(Min(Min(a,b),c),d),e),f),g),h);
}

/// Returns the maximum of two values.
template <typename T>
inline const T & Max(const T & a,const T & b)
{
 if (a>b) return a;
     else return b;
}

/// Returns the maximum of three values.
template <typename T>
inline const T & Max(const T & a,const T & b,const T & c)
{
 return Max(Max(a,b),c);
}

/// Returns the maximum of four values.
template <typename T>
inline const T & Max(const T & a,const T & b,const T & c,const T & d)
{
 return Max(Max(Max(a,b),c),d);
}

/// Returns the maximum of five values.
template <typename T>
inline const T & Max(const T & a,const T & b,const T & c,const T & d,const T & e)
{
 return Max(Max(Max(Max(a,b),c),d),e);
}

/// Returns the maximum of six values.
template <typename T>
inline const T & Max(const T & a,const T & b,const T & c,const T & d,const T & e,const T & f)
{
 return Max(Max(Max(Max(Max(a,b),c),d),e),f);
}

/// Returns the maximum of seven values.
template <typename T>
inline const T & Max(const T & a,const T & b,const T & c,const T & d,const T & e,const T & f,const T & g)
{
 return Max(Max(Max(Max(Max(Max(a,b),c),d),e),f),g);
}

/// Returns the maximum of eight values.
template <typename T>
inline const T & Max(const T & a,const T & b,const T & c,const T & d,const T & e,const T & f,const T & g,const T & h)
{
 return Max(Max(Max(Max(Max(Max(Max(a,b),c),d),e),f),g),h);
}

/// Clamps a value to the given range.
template <typename T>
inline const T & Clamp(const T & val,const T & low,const T & high)
{
 if (val<low) return low;
 else if (val>high) return high;
 else return val;
}

//------------------------------------------------------------------------------
// Accelerated version(s) of the previous set, all designed for when using
// real32's - optimised for speed and not accuracy...

/*template <>
inline real32 InvSqrt<real32>(real32 val)
{
 //#ifdef EOS_ASM
 // asm("rsqrtss xmm0, val\n"
 //     "movss val, xmm0");
 //#else // Taken from http://www.math.purdue.edu/~clomont/Math/Papers/2003/InvSqrt.pdf - an improved version of the so called carmack method - basically newton-rhapston.
  real32 xhalf = 0.5f*val;
  int32 i = *(int*)&val;
  i = 0x5f375a86 - (i>>1);
  val = *(float*)&i;
  val = val*(1.5f-xhalf*val*val);
 //#endif
 return val;
}*/

//------------------------------------------------------------------------------
// All the abs variants...
template <>
inline int8 Abs<int8>(int8 val)
{
 return static_cast<int8>(::abs(val));
}

template <>
inline int16 Abs<int16>(int16 val)
{
 return static_cast<int16>(::abs(val));
}

template <>
inline int32 Abs<int32>(int32 val)
{
 return static_cast<int32>(::abs(val));
}

template <>
inline int64 Abs<int64>(int64 val)
{
 if (val<0) return -val;
       else return val;
}

//------------------------------------------------------------------------------
// Bit manipulation functions, seperate due to there specific nature...

/// Rotates the bits of the input left a given number of places.
inline nat32 RotLeft(nat32 in,nat32 rot)
{
 #ifdef EOS_WIN32
  return _lrotl(in,rot);
 #else
  return (in<<rot) | (rot>>(32-rot));
 #endif
}

/// Rotates the bits of the input right a given number of places.
inline nat32 RotRight(nat32 in,nat32 rot)
{
 #ifdef EOS_WIN32
  return _lrotr(in,rot);
 #else
  return (in>>rot) | (rot<<(32-rot));
 #endif
}


/// Returns the position of the highest bit in a number, with 1 as the first bit
/// and 0 as no bits, i.e. how many times you can shift the number right before
/// you get 0.
inline nat32 TopBit(nat32 in)
{
 nat32 ret = 0;

 if (in==0) return ret;
 if ((in>>16)!=0) {ret += 16; in >>= 16;}
 if (in==0) return ret;
 if ((in>>8)!=0) {ret += 8; in >>= 8;}
 if (in==0) return ret;
 if ((in>>4)!=0) {ret += 4; in >>= 4;}
 if (in==0) return ret;
 if ((in>>2)!=0) {ret += 2; in >>= 2;}
 if (in==0) return ret;

 ret += 1;

 return ret;
}


/// Returns the index of the first not set bit from a given pointer, with 0
/// being the first bit etc such that ptr[return>>3] |= 1<<(return&7) would set
/// the bit in question.
inline nat32 FindUnset(byte * ptr)
{
 nat32 ret = 0;
  while (*ptr==0xFF) {++ptr; ret += 8;}
  nat32 mo=0;
   while (*ptr&(1<<mo)) {++mo;}
  ret += mo;
 return ret;
}

/// Returns how many bits are set in a bit string. Optimised with sparse strings
/// in mind, where mostly bits are not set. (Ignore the sumTo parameter,
/// used internally for tail recursion.)
inline nat32 BitCount(byte * data,nat32 dataLength,nat32 sumTo = 0)
{
 switch (dataLength)
 {
  default:
  case 4: if (*((nat32*)data)==0) return BitCount(data+4,dataLength-4,sumTo);
  case 3:
  case 2: if (*((nat16*)data)==0) return BitCount(data+2,dataLength-2,sumTo);
  case 1: if (*data==0) return BitCount(data+1,dataLength-1,sumTo);
  {
   nat8 t = *data;
   if ((t&0xF0)!=0)
   {
    if (t&0x80) ++sumTo;
    if (t&0x40) ++sumTo;
    if (t&0x20) ++sumTo;
    if (t&0x10) ++sumTo;
   }
   if ((t&0x0F)!=0)
   {
    if (t&0x08) ++sumTo;
    if (t&0x04) ++sumTo;
    if (t&0x02) ++sumTo;
    if (t&0x01) ++sumTo;
   }
  }
  return BitCount(data+1,dataLength-1,sumTo);
  case 0: return sumTo;
 }
}

/// Compares two sequences of bits, returns the relationship where they first
/// differ. Returns -1 if the first different bit is high for lhs or 1 if the
/// first different bit is high for rhs. Returns 0 if the bit sequences are
/// identical.
inline int32 CompareBitSeq(byte * lhs,byte * rhs,nat32 dataLength)
{
 // Compare upto the first 4 bytes of the input data, if there is more data than
 // this we use recursion to manage it...
  switch (dataLength)
  {
   default:
   case 4: if (*((nat32*)lhs)==*((nat32*)rhs)) return CompareBitSeq(lhs+4,rhs+4,dataLength-4);
   case 3:
   case 2: if (*((nat16*)lhs)==*((nat16*)rhs)) return CompareBitSeq(lhs+2,rhs+2,dataLength-2);
   case 1: if (*(lhs)==*(rhs)) return CompareBitSeq(lhs+1,rhs+1,dataLength-1);
   {
    // We finally have a byte to compare, apply a divide and conquer approach...
     nat8 l = *lhs;
     nat8 r = *rhs;

     if ((l&0xF0)!=(r&0xF0))
     {
      // Diff in top 4 bits...
       if ((l&0xC0)!=(r&0xC0))
       {
        // Diff in bits 7 or 8...
         if ((l&0x80)!=(r&0x80)) return (l&0x80)?-1:1;
                            else return (l&0x40)?-1:1;
       }
       else
       {
        // Diff in bits 5 or 6...
         if ((l&0x20)!=(r&0x20)) return (l&0x20)?-1:1;
                            else return (l&0x10)?-1:1;
       }
     }
     else
     {
      // Diff in bottom 4 bits...
       if ((l&0x0C)!=(r&0x0C))
       {
        // Diff in bits 3 or 4...
         if ((l&0x08)!=(r&0x08)) return (l&0x08)?-1:1;
                            else return (l&0x04)?-1:1;
       }
       else
       {
        // Diff in bits 1 or 2...
         if ((l&0x02)!=(r&0x02)) return (l&0x02)?-1:1;
                            else return (l&0x01)?-1:1;
       }
     }
   }
   case 0: return 0;
  }
}

//------------------------------------------------------------------------------
// Floating point equality functions, using various techneques...

/// Floating point equality using an epsilon function. This function is only
/// really valid if you know the approximate scale of the numbers, you should
/// use the Equal function in most cases. Simply checks if the difference is
/// less than the given epsilon.
template <typename T>
inline bit EqualEpsilon(T a,T b,T epsilon)
{
 return Abs(a-b)<epsilon;
}

/// Provided so that templates that use Equal may work.
inline bit Equal(int32 a,int32 b) {return a==b;}

/// Provided so that templates that use Equal may work.
inline bit Equal(nat32 a,nat32 b) {return a==b;}

/// real32 version of equality, using the principal that floating point numbers
/// can be converted to integers in the same ordering. Note that this function
/// can screw up if dealing with NAN or INF. The given range is how many
/// floating point positions can be between two numbers for them to be concidered
/// equal, this has the advantage that it essentially scales with how much
/// precision the numbers you are dealing with can contain. You still have to
/// concider how complicated the calculation is and therefore how far out the
/// result could be however.
inline bit Equal(real32 a,real32 b,nat32 range = 1000)
{
 // Make 'a' a twos compliment integer...
  int32 ai = *(int32*)(void*)&a;
  if (ai<0) ai = 0x80000000 - ai; // Will break on non-twos compliment architecture. Not really made any more though.

 // Make 'b' a twos compliemnt integer...
  int32 bi = *(int32*)(void*)&b;
  if (bi<0) bi = 0x80000000 - bi;

 // And compare...
  return Abs(ai-bi)<=static_cast<int32>(range);
}

/// real64 version of equality, using the principal that floating point numbers
/// can be converted to integers in the same ordering. Note that this function
/// can screw up if dealing with NAN or INF. The given range is how many
/// floating point positions can be between two numbers for them to be concidered
/// equal, this has the advantage that it essentially scales with how much
/// precision the numbers you are dealing with can contain. You still have to
/// concider how complicated the calculation is and therefore how far out the
/// result could be however.
inline bit Equal(real64 a,real64 b,nat32 range = 1000)
{
 // Make 'a' a twos compliment integer...
  int64 ai = *(int64*)(void*)&a;
  if (ai<0) ai = 0x8000000000000000LL - ai; // Will break on non-twos compliment architecture. Not really made any more though.

 // Make 'b' a twos compliemnt integer...
  int64 bi = *(int64*)(void*)&b;
  if (bi<0) bi = 0x8000000000000000LL - bi;

 // And compare...
  return Abs(ai-bi)<=static_cast<int64>(range);
}

/// Checks for being equal to zero. I do this quite a lot it seems.
template <typename T>
inline bit IsZero(T x,nat32 range = 1000)
{
 return Equal(x,static_cast<T>(0),range);
}

/// Returns -1, 0 or 1, depending on the sign of the value. Uses IsZero to
/// determine when to return 0.
template <typename T>
inline int32 Sign(T x,nat32 range = 1000)
{
 if (IsZero(x,range)) return 0;
 if (x<static_cast<T>(0)) return -1;
                     else return 1;
}

/// Returns -1 or 1, depending on the sign of the value.
/// For zero as an input can return either.
template <typename T>
inline int32 SignNZ(T x)
{
 if (x<static_cast<T>(0)) return -1;
                     else return 1;
}

/// Returns true if the two numbers have the same sign, false if they do not.
/// Zero is considered to be both signs, with it returning true if either number
/// is zero.
template <typename T>
inline bit SameSign(T a,T b,nat32 range = 1000)
{
 int32 t = Sign(a,range) * Sign(b,range);
 return t!=-1;
}

//------------------------------------------------------------------------------
/// Floating point type checks...

//#ifdef EOS_64BIT // On my 64 bit computer these functions are totally broken :-( - they all think *all* bit sequences are finite.
// (The below will work as long as it is ieee compliant.)


/// Returns true if given a NaN, false otherwise.
inline bit IsNan(real32 n)
{
 nat32 & nn = *(nat32*)(void*)&n;
 bit expFF = (0x7F800000&nn)==0x7F800000;
 bit fracZero = (0x7FFFFF&nn)==0;
 return expFF and !fracZero;
}

/// Returns true if given a NaN, false otherwise.
inline bit IsNan(real64 n)
{
 nat64 & nn = *(nat64*)(void*)&n;
 bit expFF = (0x7FF0000000000000LLU&nn)==0x7FF0000000000000LLU;
 bit fracZero = (0x000FFFFFFFFFFFFFLLU&nn)==0;
 return expFF and !fracZero;
}

/// Returns true if given an infinity, false otherwise.
inline bit IsInf(real32 n)
{
 nat32 & nn = *(nat32*)(void*)&n;
 bit expFF = (0x7F800000&nn)==0x7F800000;
 bit fracZero = (0x7FFFFF&nn)==0;
 return expFF and fracZero;
}

/// Returns true if given an infinity, false otherwise.
inline bit IsInf(real64 n)
{
 nat64 & nn = *(nat64*)(void*)&n;
 bit expFF = (0x7FF0000000000000LLU&nn)==0x7FF0000000000000LLU;
 bit fracZero = (0x000FFFFFFFFFFFFFLLU&nn)==0;
 return expFF and fracZero;
}

/// Returns true if given a finite number, i.e. one that is neither infinite nor nan, false otherwise.
inline bit IsFinite(real32 n)
{
 nat32 & nn = *(nat32*)(void*)&n;
 bit expFF = (0x7F800000&nn)==0x7F800000;
 return !expFF;
}

/// Returns true if given a finite number, i.e. one that is neither infinite nor nan, false otherwise.
inline bit IsFinite(real64 n)
{
 nat64 & nn = *(nat64*)(void*)&n;
 bit expFF = (0x7FF0000000000000LLU&nn)==0x7FF0000000000000LLU;
 return !expFF;
}


/*#else


// #ifdef EOS_WIN32

/// Returns true if given a NaN, false otherwise.
inline bit IsNan(real32 n) {return isnan(n);}

/// Returns true if given an infinity, false otherwise.
inline bit IsInf(real32 n) {return isinf(n);}

/// Returns true if given a finite number, i.e. one that is neither infinite nor nan, false otherwise.
inline bit IsFinite(real32 n) {return finite(n);}

// #else

/// Returns true if given a NaN, false otherwise.
inline bit IsNan(real32 n) {return isnanf(n);}

/// Returns true if given an infinity, false otherwise.
inline bit IsInf(real32 n) {return isinff(n);}

/// Returns true if given a finite number, i.e. one that is neither infinite nor nan, false otherwise.
inline bit IsFinite(real32 n) {return finitef(n);}

// #endif


/// Returns true if given a NaN, false otherwise.
inline bit IsNan(real64 n) {return isnan(n);}

/// Returns true if given an infinity, false otherwise.
inline bit IsInf(real64 n) {return isinf(n);}

/// Returns true if given a finite number, i.e. one that is neither infinite nor nan, false otherwise.
inline bit IsFinite(real64 n) {return finite(n);}


// #endif
*/

//------------------------------------------------------------------------------
// Unit conversion functions...

/// Converts from radians to degrees.
template <typename T>
inline T ToDeg(T x)
{
 return	x*180.0/math::pi;
}

/// Converts from degrees to radians.
template <typename T>
inline T ToRad(T x)
{
 return	x*math::pi/180.0;
}

//------------------------------------------------------------------------------
// Probability distributions...

/// A Laplacian distribution, with mean 0.0. (Un-normalised, divide by 2*sd to normalise.)
template <typename T>
inline T Laplace(T sd,T x)
{
 return Exp(-Abs(x)/sd);
}

//------------------------------------------------------------------------------
/// A sigmoid implimentation, for implimenting soft-cutoffs.
/// Returns 1 when less than the cutoff and 0 where above, except it doesn't
/// because its soft. You provide the half life and threshold plus the value,
/// and it returns the [0..1] result. This is robust against extreme values.
template<typename T>
inline T SigmoidCutoff(T value,T cutoff,T halfLife)
{
 return 0.5 - 0.5*math::Tanh((value-cutoff)/(2.0*halfLife));
}

//------------------------------------------------------------------------------
 };
};
#endif
