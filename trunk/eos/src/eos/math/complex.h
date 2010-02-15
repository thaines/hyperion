#ifndef EOS_MATH_COMPLEX_H
#define EOS_MATH_COMPLEX_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file complex.h
/// Provides a complex number class.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/math/functions.h"
#include "eos/io/inout.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// A complex number class, templated on the type of real number used.
/// Acts as expected, impliments most operators including overloading ^ to be 
/// exponentiation. 
/// Represents the complex number as this->x + this->y * i;
template <typename T = real32>
class EOS_CLASS Complex
{
 public:
  /// The real number type, for templated code.
   typedef T type;
 
  /// Does no initialisation, leaves the contents random.
   Complex() {}
   
  /// Sets it to xx + yy*i.
   Complex(const T & xx,const T & yy = static_cast<T>(0)):x(xx),y(yy) {}
   
  /// &nbsp;
   Complex(const Complex<T> & rhs):x(rhs.x),y(rhs.y) {}


  /// &nbsp;
   Complex<T> & operator = (const T & rhs) {x = rhs; y = static_cast<T>(0); return *this;}

  /// &nbsp;
   Complex<T> & operator = (const Complex<T> & rhs) {x = rhs.x; y = rhs.y; return *this;}


  /// &nbsp;
   T Arg() const {return InvTan2(y,x);}
   
  /// &nbsp;
   T Mod() const {return Sqrt(Sqr(x) + Sqr(y));}

  /// &nbsp;
   T Real() const {return x;}
   
  /// &nbsp;
   T Imag() const {return y;}

   
  /// &nbsp;
   Complex<T> & operator += (const T & rhs) {x += rhs; return *this;}

  /// &nbsp;
   Complex<T> & operator -= (const T & rhs) {x -= rhs; return *this;}
   
  /// &nbsp;
   Complex<T> & operator *= (const T & rhs) {x *= rhs; y *= rhs; return *this;}
   
  /// &nbsp;
   Complex<T> & operator /= (const T & rhs) {x /= rhs; y /= rhs; return *this;}
   
  /// &nbsp;
   Complex<T> & operator ^= (const T & rhs)
   {
    T arg = Arg()*rhs;
    T mod = Pow(Mod(),rhs);
    x = mod * Cos(arg);
    y = mod * Sin(arg);
    return *this;
   }


  /// &nbsp;
   Complex<T> operator + (const T & rhs) const {Complex<T> ret(*this); ret += rhs; return ret;}

  /// &nbsp;
   Complex<T> operator - (const T & rhs) const {Complex<T> ret(*this); ret -= rhs; return ret;}
   
  /// &nbsp;
   Complex<T> operator * (const T & rhs) const {Complex<T> ret(*this); ret *= rhs; return ret;}
   
  /// &nbsp;
   Complex<T> operator / (const T & rhs) const {Complex<T> ret(*this); ret /= rhs; return ret;}
   
  /// &nbsp;
   Complex<T> operator ^ (const T & rhs) const {Complex<T> ret(*this); ret ^= rhs; return ret;}


  /// &nbsp;
   Complex<T> & operator += (const Complex<T> & rhs) {x += rhs.x; y += rhs.y; return *this;}

  /// &nbsp;
   Complex<T> & operator -= (const Complex<T> & rhs) {x -= rhs.x; y -= rhs.y; return *this;}
   
  /// &nbsp;
   Complex<T> & operator *= (const Complex<T> & rhs)
   {
    T tx = x*rhs.x - y*rhs.y;
    T ty = x*rhs.y + y*rhs.x;
    x = tx;
    y = ty;
    return *this;
   }
   
  /// &nbsp;
   Complex<T> & operator /= (const Complex<T> & rhs)
   {
    T tx = x*rhs.x + y*rhs.y;
    T ty = y*rhs.x - x*rhs.y;
    T div = math::Sqr(rhs.x) + math::Sqr(rhs.y);
    x = tx/div;
    y = ty/div;
    return *this;
   }
   
  /// &nbsp;
   Complex<T> & operator ^= (const Complex<T> & rhs)
   {
    T arg = Arg();
    T mod = Mod();

    T mult = Pow(mod,rhs.x*static_cast<T>(0.5)) * Exp(-rhs.y*arg);
    T th = rhs.x*arg + static_cast<T>(0.5)*rhs.y*Ln(mod);

    x = mult*Cos(th);
    y = mult*Sin(th);
    return *this;
   }


  /// &nbsp;
   Complex<T> operator + (const Complex<T> & rhs) const {Complex<T> ret(*this); ret += rhs; return ret;}

  /// &nbsp;
   Complex<T> operator - (const Complex<T> & rhs) const {Complex<T> ret(*this); ret -= rhs; return ret;}
   
  /// &nbsp;
   Complex<T> operator * (const Complex<T> & rhs) const {Complex<T> ret(*this); ret *= rhs; return ret;}
   
  /// &nbsp;
   Complex<T> operator / (const Complex<T> & rhs) const {Complex<T> ret(*this); ret /= rhs; return ret;}
   
  /// &nbsp;
   Complex<T> operator ^ (const Complex<T> & rhs) const {Complex<T> ret(*this); ret ^= rhs; return ret;}


  /// Sets this to its conjugate.
   void Conjugate() {y = -y;}
  
  /// Sets this to its natural logarithm. 
   void Ln()
   {
    T mod = Mod();
    T arg = Arg();
    
    x = Ln(mod);
    y = Mod(arg,static_cast<T>(2.0*pi));
   }


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::math::Complex<" << typestring<T>() << ">");
    return ret;
   }


 /// The real component.
  T x;
 
 /// The imaginary component.
  T y;	
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// IO stream integration for the above classes...
namespace eos
{
 namespace io
 {


  template <typename T, typename VT>
  inline T & StreamRead(T & lhs,math::Complex<VT> & rhs,Binary)
  {
   lhs >> rhs.x >> rhs.y;
   return lhs;	  
  }
  
  template <typename T, typename VT>
  inline T & StreamWrite(T & lhs,const math::Complex<VT> & rhs,Binary)
  {
   lhs << rhs.x << rhs.y;
   return lhs;
  }
  
  template <typename T, typename VT>
  inline T & StreamWrite(T & lhs,const math::Complex<VT> & rhs,Text)
  {
   lhs << rhs.x;
   if (rhs.y>=0.0) lhs << "+";
   lhs << rhs.y << "i";
   return lhs;
  }  


 };
};

//------------------------------------------------------------------------------

#endif
