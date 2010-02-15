#ifndef EOS_BS_COLOURS_H
#define EOS_BS_COLOURS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \namespace eos::bs
/// Provides basic structures, standard data structures that are simple and used
/// regularly. Includes the dom, as well as colours and geometric primatives.

/// \file colours.h
/// Provides representations of colour, including conversion between the various 
/// formats. Quite extensive. Conversions have mostly been obtained from the
/// website www.brucelindbloom.com.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/math/functions.h"
#include "eos/io/inout.h"

namespace eos
{
 namespace bs
 {
//------------------------------------------------------------------------------
// List of classes defined in this file, for out of order referencing...
// (Excluding versions with alpha.)
class ColourL;
class ColourRGB;
class ColourRGBs;
class ColourYcbcr;
class ColourLuv;
class ColourXYZ;

class ColRGB;

//------------------------------------------------------------------------------
/// Empty class from which all Colour types inherit, for structuring.
class EOS_CLASS Colour
{};

//------------------------------------------------------------------------------
/// Represents a single greyscale value, refered to as luminance. For the 
/// purposes of conversion it is assumed to represent the y in Y' cb cr colour 
/// space. (It is not a proper colour space, especially as it dosn't do colour!)
class EOS_CLASS ColourL : public Colour
{
 public:
  /// &nbsp;
   ColourL() {}

  /// &nbsp;
   ColourL(real32 rl):l(rl) {}

  /// &nbsp;
   ColourL(const ColourL & rhs):l(rhs.l) {}

  /// &nbsp;
   ColourL(const ColourRGB & rhs) {*this = rhs;}

  /// &nbsp;
   ColourL(const ColourRGBs & rhs) {*this = rhs;}

  /// &nbsp;
   ColourL(const ColourYcbcr & rhs) {*this = rhs;}

  /// &nbsp;
   ColourL(const ColourLuv & rhs) {*this = rhs;}

  /// &nbsp;
   ColourL(const ColourXYZ & rhs) {*this = rhs;}

  /// &nbsp;
   ColourL(const ColRGB & rhs) {*this = rhs;}


  /// &nbsp;
   ColourL & operator = (real32 rl) {l = rl; return *this;}

  /// &nbsp;
   ColourL & operator = (const ColourL & rhs) {l = rhs.l; return *this;}

  /// &nbsp;
   ColourL & operator = (const ColourRGB & rhs);

  /// &nbsp;
   ColourL & operator = (const ColourRGBs & rhs);

  /// &nbsp;
   ColourL & operator = (const ColourYcbcr & rhs);

  /// &nbsp;
   ColourL & operator = (const ColourLuv & rhs);

  /// &nbsp;
   ColourL & operator = (const ColourXYZ & rhs);

  /// &nbsp;
   ColourL & operator = (const ColRGB & rhs);


  /// &nbsp;
   static cstrconst TypeString() {return "eos::bs::ColourL";}

 
  /// [0..1]
   real32 l;
};

//------------------------------------------------------------------------------
/// Represents the traditional RGB colour space. We assume it has the D65 white
/// point but no specific gamma when converting. This is what most people do, 
/// though it is technically wrong. See ColourRGBs for a real rgb implimentation
/// with correct conversions. Essentially, it assumes it has the same gamma as 
/// whatever it is being converted to.
class EOS_CLASS ColourRGB : public Colour
{
 public:
  /// &nbsp;
   ColourRGB() {}

  /// &nbsp;
   ColourRGB(real32 rr,real32 rg,real32 rb):r(rr),g(rg),b(rb) {}

  /// &nbsp;
   ColourRGB(const ColourL & rhs):r(rhs.l),g(rhs.l),b(rhs.l) {}

  /// &nbsp;
   ColourRGB(const ColourRGB & rhs):r(rhs.r),g(rhs.g),b(rhs.b) {}

  /// &nbsp;
   ColourRGB(const ColourRGBs & rhs) {*this = rhs;}

  /// &nbsp;
   ColourRGB(const ColourYcbcr & rhs) {*this = rhs;}

  /// &nbsp;
   ColourRGB(const ColourLuv & rhs) {*this = rhs;}

  /// &nbsp;
   ColourRGB(const ColourXYZ & rhs) {*this = rhs;}

  /// &nbsp;
   ColourRGB(const ColRGB & rhs) {*this = rhs;}


  /// &nbsp;
   ColourRGB & operator = (const ColourL & rhs) {r=rhs.l; g=rhs.l; b=rhs.l; return *this;}
 
  /// &nbsp;
   ColourRGB & operator = (const ColourRGB & rhs) {r=rhs.r; g=rhs.g; b=rhs.b; return *this;}

  /// &nbsp;
   ColourRGB & operator = (const ColourRGBs & rhs);

  /// &nbsp;
   ColourRGB & operator = (const ColourYcbcr & rhs);

  /// &nbsp;
   ColourRGB & operator = (const ColourLuv & rhs);

  /// &nbsp;
   ColourRGB & operator = (const ColourXYZ & rhs);

  /// &nbsp;
   ColourRGB & operator = (const ColRGB & rhs);


  /// &nbsp;
   ColourRGB & operator *= (real32 rhs) {r*=rhs; g*=rhs; b*=rhs; return *this;}

  /// &nbsp;
   ColourRGB & operator *= (const ColourRGB & rhs) {r*=rhs.r; g*=rhs.g; b*=rhs.b; return *this;}

  /// &nbsp;
   ColourRGB & operator /= (real32 rhs) {*this *= 1.0/rhs; return *this;}

  
  /// &nbsp;
   ColourRGB & operator += (const ColourRGB & rhs) {r+=rhs.r; g+=rhs.g; b+=rhs.b; return *this;}


  /// &nbsp;
   static cstrconst TypeString() {return "eos::bs::ColourRGB";}


  /// [0..1]
   real32 r;

  /// [0..1]
   real32 g;

  /// [0..1]
   real32 b;
};

//------------------------------------------------------------------------------
/// Identical to ColourRGB in every aspect except it compensates for gamma
/// correctly. This represents a rgb colour in the sRGB colour space (D65 white)
/// and responds accordingly for conversions. Note that converting between 
/// ColourRGB and ColourRGBs results in no change to the data, only a change when
/// in teh future it is transformed into a real colour space.
class EOS_CLASS ColourRGBs : public Colour
{
 public:
  /// &nbsp;
   ColourRGBs() {}

  /// &nbsp;
   ColourRGBs(real32 rr,real32 rg,real32 rb):r(rr),g(rg),b(rb) {}

  /// &nbsp;
   ColourRGBs(const ColourL & rhs):r(rhs.l),g(rhs.l),b(rhs.l) {}

  /// &nbsp;
   ColourRGBs(const ColourRGB & rhs):r(rhs.r),g(rhs.g),b(rhs.b) {}

  /// &nbsp;
   ColourRGBs(const ColourRGBs & rhs):r(rhs.r),g(rhs.g),b(rhs.b) {}

  /// &nbsp;
   ColourRGBs(const ColourYcbcr & rhs) {*this = rhs;}

  /// &nbsp;
   ColourRGBs(const ColourLuv & rhs) {*this = rhs;}

  /// &nbsp;
   ColourRGBs(const ColourXYZ & rhs) {*this = rhs;}

  
  /// &nbsp;
   ColourRGBs & operator = (const ColourL & rhs) {r=rhs.l; g=rhs.l; b=rhs.l; return *this;}
 
  /// &nbsp;
   ColourRGBs & operator = (const ColourRGB & rhs) {r=rhs.r; g=rhs.g; b=rhs.b; return *this;}

  /// &nbsp;
   ColourRGBs & operator = (const ColourRGBs & rhs) {r=rhs.r; g=rhs.g; b=rhs.b; return *this;}

  /// &nbsp;
   ColourRGBs & operator = (const ColourYcbcr & rhs);

  /// &nbsp;
   ColourRGBs & operator = (const ColourLuv & rhs);

  /// &nbsp;
   ColourRGBs & operator = (const ColourXYZ & rhs);


  /// &nbsp;
   ColourRGBs & operator *= (real32 rhs) {r*=rhs; g*=rhs; b*=rhs; return *this;}

  /// &nbsp;
   ColourRGBs & operator /= (real32 rhs) {*this *= 1.0/rhs; return *this;}

  
  /// &nbsp;
   ColourRGBs & operator += (const ColourRGBs & rhs) {r+=rhs.r; g+=rhs.g; b+=rhs.b; return *this;}


  /// &nbsp;
   static cstrconst TypeString() {return "eos::bs::ColourRGBs";}


  /// [0..1]
   real32 r;

  /// [0..1]
   real32 g;

  /// [0..1]
   real32 b;
};

//------------------------------------------------------------------------------
/// Represents the Y' Cb Cr colour space. This is not a proper colour space, 
/// much like rgb, and is not implimented 'correctly' because its not really 
/// possible. It does all convertions without gamma correction, like ColourRGB.
/// Uses the Jpeg full-range formulars without the 0.5 offset leaving them in 
/// 0..1 and -0.5..0.5 ranges.
class EOS_CLASS ColourYcbcr : public Colour
{
 public:
  /// &nbsp;
   ColourYcbcr() {}

  /// &nbsp;
   ColourYcbcr(real32 yy,real32 ccb,real32 ccr):y(yy),cb(ccb),cr(ccr) {}

  /// &nbsp;
   ColourYcbcr(const ColourL & rhs) {*this = rhs;}

  /// &nbsp;
   ColourYcbcr(const ColourRGB & rhs) {*this = rhs;}

  /// &nbsp;
   ColourYcbcr(const ColourRGBs & rhs) {*this = rhs;}

  /// &nbsp;
   ColourYcbcr(const ColourYcbcr & rhs):y(rhs.y),cb(rhs.cb),cr(rhs.cr) {}

  /// &nbsp;
   ColourYcbcr(const ColourLuv & rhs) {*this = rhs;}

  /// &nbsp;
   ColourYcbcr(const ColourXYZ & rhs) {*this = rhs;}


  /// &nbsp;
   ColourYcbcr & operator = (const ColourL & rhs);
 
  /// &nbsp;
   ColourYcbcr & operator = (const ColourRGB & rhs);

  /// &nbsp;
   ColourYcbcr & operator = (const ColourRGBs & rhs);

  /// &nbsp;
   ColourYcbcr & operator = (const ColourYcbcr & rhs) {y = rhs.y; cb = rhs.cb; cr = rhs.cr; return *this;}

  /// &nbsp;
   ColourYcbcr & operator = (const ColourLuv & rhs);

  /// &nbsp;
   ColourYcbcr & operator = (const ColourXYZ & rhs);


  /// &nbsp;
   static cstrconst TypeString() {return "eos::bs::ColourYcbcr";}


  /// [0..1]
   real32 y;

  /// [-0.5..0.5]
   real32 cb;

  /// [-0.5..0.5]
   real32 cr;
};

//------------------------------------------------------------------------------
/// Represents the L*u*v* colour space.
class EOS_CLASS ColourLuv : public Colour
{
 public:
  /// &nbsp;
   ColourLuv() {}

  /// &nbsp;
   ColourLuv(real32 ll,real32 uu,real32 vv):l(ll),u(uu),v(vv) {}

  /// &nbsp;
   ColourLuv(const ColourL & rhs) {*this = rhs;}

  /// &nbsp;
   ColourLuv(const ColourRGB & rhs) {*this = rhs;}

  /// &nbsp;
   ColourLuv(const ColourRGBs & rhs) {*this = rhs;}

  /// &nbsp;
   ColourLuv(const ColourYcbcr & rhs) {*this = rhs;}

  /// &nbsp;
   ColourLuv(const ColourLuv & rhs):l(rhs.l),u(rhs.u),v(rhs.v) {}

  /// &nbsp;
   ColourLuv(const ColourXYZ & rhs) {*this = rhs;}


  /// &nbsp;
   ColourLuv & operator = (const ColourL & rhs);
 
  /// &nbsp;
   ColourLuv & operator = (const ColourRGB & rhs);

  /// &nbsp;
   ColourLuv & operator = (const ColourRGBs & rhs);

  /// &nbsp;
   ColourLuv & operator = (const ColourYcbcr & rhs);

  /// &nbsp;
   ColourLuv & operator = (const ColourLuv & rhs) {l = rhs.l; u = rhs.u; v = rhs.v; return *this;}

  /// &nbsp;
   ColourLuv & operator = (const ColourXYZ & rhs);
   
   
  /// Replaces this colour with a mix of this colour and another.
  /// Linear, obviously, t==0 makes no change, t==1 is assignment.
   void Mix(real32 t,const ColourLuv & rhs)
   {
    l = (1.0-t)*l + t*rhs.l;
    u = (1.0-t)*u + t*rhs.u;
    v = (1.0-t)*v + t*rhs.v;
   }


  /// &nbsp;
   ColourLuv & operator += (const ColourLuv & rhs) {l += rhs.l; u += rhs.u; v += rhs.v; return *this;}

  /// &nbsp;
   ColourLuv & operator -= (const ColourLuv & rhs) {l -= rhs.l; u -= rhs.u; v -= rhs.v; return *this;}


  /// Returns the eucledian difference between this colour and the given.
   real32 Diff(const ColourLuv & rhs) const
   {
    return math::Sqrt(math::Sqr(l-rhs.l) +
                      math::Sqr(u-rhs.u) +
                      math::Sqr(v-rhs.v));
   }


  /// &nbsp;
   static cstrconst TypeString() {return "eos::bs::ColourLuv";}


  /// [0,100]
   real32 l;

  /// [-134,220]
   real32 u;

  /// [-140,122]
   real32 v;
};

//------------------------------------------------------------------------------
/// Represents the XYZ colour space. Defined to be used as an intermediary
/// rather than actually used, unless your being particularly anal. We presume 
/// the D65 white point for it, that being X = 0.950467, Y = 0.9999996, Z = 1.088969
class EOS_CLASS ColourXYZ : public Colour
{
 public:
  /// &nbsp;
   ColourXYZ() {}

  /// &nbsp;
   ColourXYZ(real32 xx,real32 yy,real32 zz):x(xx),y(yy),z(zz) {}

  /// &nbsp;
   ColourXYZ(const ColourL & rhs) {*this = rhs;}

  /// &nbsp;
   ColourXYZ(const ColourRGB & rhs) {*this = rhs;}

  /// &nbsp;
   ColourXYZ(const ColourRGBs & rhs) {*this = rhs;}

  /// &nbsp;
   ColourXYZ(const ColourYcbcr & rhs) {*this = rhs;}

  /// &nbsp;
   ColourXYZ(const ColourLuv & rhs) {*this = rhs;}

  /// &nbsp;
   ColourXYZ(const ColourXYZ & rhs):x(rhs.x),y(rhs.y),z(rhs.z) {}


  /// &nbsp;
   ColourXYZ & operator = (const ColourL & rhs);
 
  /// &nbsp;
   ColourXYZ & operator = (const ColourRGB & rhs);

  /// &nbsp;
   ColourXYZ & operator = (const ColourRGBs & rhs);

  /// &nbsp;
   ColourXYZ & operator = (const ColourYcbcr & rhs);

  /// &nbsp;
   ColourXYZ & operator = (const ColourLuv & rhs);

  /// &nbsp;
   ColourXYZ & operator = (const ColourXYZ & rhs) {x = rhs.x; y = rhs.y; z = rhs.z; return *this;}


  /// &nbsp;
   static cstrconst TypeString() {return "eos::bs::ColourXYZ";}


  /// ~[0..1]
   real32 x;

  /// ~[0..1]
   real32 y;

  /// ~[0..1]
   real32 z;
};

//------------------------------------------------------------------------------
/// Class that 'adds' alpha to any other colour type, note that typedefs have
/// been provided of all colour classes with this modification, so you will not
/// have to use this class directly.
template <typename T>
class EOS_CLASS Alpha : public T
{
 public:
  /// Leaves the contents of the class random.
   Alpha() {}
   
  /// Constructs the class given the base colour type and an alpha value.
   template <typename T2>
   Alpha(const T2 & col,real32 aa):T(col),a(aa) {}
   
  /// Constructs the class from any alpha type.
   template <typename T2>
   Alpha(const T2 & col):T(col),a(col.a) {}
   
  /// We can only assign from Alpha types, a limitation of the language.
   template <typename T2>
   Alpha<T> & operator = (const T2 & rhs) {*this = rhs; a = rhs.a;return *this;}
 
  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::bs::Alpha<" << typestring<T>() << ">");
    return ret;
   }

   
  /// [0..1]
   real32 a;
};

//------------------------------------------------------------------------------
/// &nbsp;
typedef Alpha<ColourL> ColourLA;

/// &nbsp;
typedef Alpha<ColourRGB> ColourRGBA;

/// &nbsp;
typedef Alpha<ColourRGBs> ColourRGBsA;

/// &nbsp;
typedef Alpha<ColourYcbcr> ColourYcbcrA;

/// &nbsp;
typedef Alpha<ColourLuv> ColourLuvA;

/// &nbsp;
typedef Alpha<ColourXYZ> ColourXYZA;

//------------------------------------------------------------------------------
/// A RGB colour class that uses bytes instead of floats, used exclusivly for
/// interfacing with awkward systems or when memory is tight.
class EOS_CLASS ColRGB : public Colour
{
 public:
  /// &nbsp;
   ColRGB() {}
   
  /// &nbsp;
   ColRGB(byte rr,byte gg,byte bb):r(rr),g(gg),b(bb) {}
   
  /// &nbsp;
   ColRGB(const ColRGB & rhs):r(rhs.r),g(rhs.g),b(rhs.b) {}
   
  /// &nbsp;
   ColRGB(const ColourRGB & rhs):r(byte(rhs.r*255.0)),g(byte(rhs.g*255.0)),b(byte(rhs.b*255.0)) {}


  /// &nbsp;
   ColRGB & operator = (const ColRGB & rhs) {r = rhs.r; g = rhs.g; b = rhs.b; return *this;}

  /// &nbsp;
   ColRGB & operator = (const ColourL & rhs)
   {ColourRGB temp = rhs; *this = temp; return *this;}

  /// &nbsp;
   ColRGB & operator = (const ColourRGB & rhs)
   {r = byte(rhs.r*255.0); g = byte(rhs.g*255.0); b = byte(rhs.b*255.0); return *this;}


  /// &nbsp;
   static cstrconst TypeString() {return "eos::bs::ColRGB";}


 byte r; ///< &nbsp;
 byte g; ///< &nbsp;
 byte b; ///< &nbsp;
};

//------------------------------------------------------------------------------
// Operators thet can not be defined earlier due to the problems of forward decleration...
inline ColourL & ColourL::operator = (const ColourRGB & rhs)
{
 l = 0.299*rhs.r + 0.587*rhs.g + 0.114*rhs.b; 
 return *this;
}

inline ColourL & ColourL::operator = (const ColourRGBs & rhs)
{
 l = 0.299*rhs.r + 0.587*rhs.g + 0.114*rhs.b; 
 return *this;
}

inline ColourL & ColourL::operator = (const ColourYcbcr & rhs)
{
 l = rhs.y;
 return *this;
}

inline ColourL & ColourL::operator = (const ColourLuv & rhs)
{
 ColourRGB rgb = rhs;
 *this = rgb;
 return *this;
}

inline ColourL & ColourL::operator = (const ColourXYZ & rhs)
{
 ColourRGBs rgb = rhs;
 *this = rgb;
 return *this;       
}

inline ColourL & ColourL::operator = (const ColRGB & rhs)
{
 l = real32(rhs.r + rhs.g + rhs.b)/(255.0*3.0);
 return *this;
}

inline ColourRGB & ColourRGB::operator = (const ColourRGBs & rhs)
{
 r = rhs.r;
 g = rhs.g;
 b = rhs.b;
 return *this;
}

inline ColourRGB & ColourRGB::operator = (const ColourYcbcr & rhs)
{
 r = rhs.y + 1.402*rhs.cr;
 g = rhs.y - 0.34414*rhs.cb - 0.71414*rhs.cr;
 b = rhs.y + 1.772*rhs.cb;
 return *this;       
}

inline ColourRGB & ColourRGB::operator = (const ColourLuv & rhs)
{
 ColourXYZ xyz = rhs;
 *this = xyz;
 return *this;
}

inline ColourRGB & ColourRGB::operator = (const ColourXYZ & rhs)
{
 r = rhs.x*3.24071   + rhs.y*-1.53726 +  rhs.z*-0.498571;
 g = rhs.x*-0.969258 + rhs.y*1.87599 +   rhs.z*0.0415557;
 b = rhs.x*0.0556352 + rhs.y*-0.203996 + rhs.z*1.05707;

 return *this;      
}

inline ColourRGB & ColourRGB::operator = (const ColRGB & rhs)
{
 r = real32(rhs.r)/255.0;
 g = real32(rhs.g)/255.0;
 b = real32(rhs.b)/255.0;  
 return *this;
}

inline ColourRGBs & ColourRGBs::operator = (const ColourYcbcr & rhs)
{
 ColourRGB rgb = rhs;
 *this = rgb;
 return *this;       
}

inline ColourRGBs & ColourRGBs::operator = (const ColourLuv & rhs)
{
 ColourXYZ xyz = rhs;
 *this = xyz;
 return *this;
}

inline ColourRGBs & ColourRGBs::operator = (const ColourXYZ & rhs)
{
 r = rhs.x*3.24071   + rhs.y*-1.53726 +  rhs.z*-0.498571;
 g = rhs.x*-0.969258 + rhs.y*1.87599 +   rhs.z*0.0415557;
 b = rhs.x*0.0556352 + rhs.y*-0.203996 + rhs.z*1.05707;

 r = (r<=0.0031308)?(12.92*r):(1.055*math::Pow(r,real32(1.0/2.4))-0.055);
 g = (g<=0.0031308)?(12.92*g):(1.055*math::Pow(g,real32(1.0/2.4))-0.055);
 b = (b<=0.0031308)?(12.92*b):(1.055*math::Pow(b,real32(1.0/2.4))-0.055);            

 return *this;      
}

inline ColourYcbcr & ColourYcbcr::operator = (const ColourL & rhs)
{
 ColourRGB rgb = rhs;
 *this = rgb;
 return *this;       
}

inline ColourYcbcr & ColourYcbcr::operator = (const ColourRGB & rhs)
{
 y = 0.299*rhs.r + 0.587*rhs.g + 0.114*rhs.b;
 cb = -0.168736*rhs.r - 0.331264*rhs.g + 0.5*rhs.b;
 cr = 0.5*rhs.r - 0.418688*rhs.g - 0.081312*rhs.b;
 return *this;      
}

inline ColourYcbcr & ColourYcbcr::operator = (const ColourRGBs & rhs)
{
 ColourRGB rgb = rhs;
 *this = rgb;
 return *this;       
}

inline ColourYcbcr & ColourYcbcr::operator = (const ColourLuv & rhs)
{
 ColourRGB rgb = rhs;
 *this = rgb;
 return *this;       
}

inline ColourYcbcr & ColourYcbcr::operator = (const ColourXYZ & rhs)
{
 ColourRGB rgb = rhs;
 *this = rgb;
 return *this;       
}

inline ColourLuv & ColourLuv::operator = (const ColourL & rhs)
{
 ColourYcbcr ic = rhs;
 *this = ic;
 return *this;
}

inline ColourLuv & ColourLuv::operator = (const ColourRGB & rhs)
{
 ColourXYZ ic = rhs;
 *this = ic;						
 return *this;
}

inline ColourLuv & ColourLuv::operator = (const ColourRGBs & rhs)
{
 ColourXYZ ic = rhs;
 *this = ic;						
 return *this;
}

inline ColourLuv & ColourLuv::operator = (const ColourYcbcr & rhs)
{
 ColourRGB rgb = rhs;
 *this = rgb;
 return *this;       
}

inline ColourLuv & ColourLuv::operator = (const ColourXYZ & rhs)
{
 real32 yr = rhs.y / 0.9999996;
 real32 div = rhs.x + 15.0*rhs.y + 3.0*rhs.z;
 if (!math::Equal(div,real32(0.0)))
 {
  real32 ud = (4.0*rhs.x)/div;
  real32 vd = (9.0*rhs.y)/div;

  const real32 udr = (4.0*0.950467)/(0.950467 + 15.0*0.9999996 + 3.0*1.088969);
  const real32 vdr = (9.0)/(0.950467 + 15.0*0.9999996 + 3.0*1.088969);

  l = (yr>(216.0/24389.0))?(116.0*math::Pow(yr,real32(1.0/3.0)) - 16.0):(yr*24389.0/27.0);
  u = 13.0*l*(ud -udr);
  v = 13.0*l*(vd -vdr);
 }
 else
 {
  l = 0.0;
  u = 0.0;
  v = 0.0;
 }

 return *this;
}

inline ColourXYZ & ColourXYZ::operator = (const ColourL & rhs)
{
 ColourYcbcr ic = rhs;
 *this = ic;
 return *this;   
}

inline ColourXYZ & ColourXYZ::operator = (const ColourRGB & rhs)
{
 x = rhs.r*0.412424 + rhs.g*0.357579 + rhs.b*0.180464; // Sums to 0.950467
 y = rhs.r*0.212656 + rhs.g*0.715158 + rhs.b*0.0721856; // Sums to 0.9999996
 z = rhs.r*0.0193324 + rhs.g*0.119193 + rhs.b*0.950444; // Sums to 1.0889694

 return *this;
}

inline ColourXYZ & ColourXYZ::operator = (const ColourRGBs & rhs)
{
 real32 r = (rhs.r<=0.04045)?(rhs.r/12.92):(math::Pow((rhs.r+0.055)/1.055,2.4));
 real32 g = (rhs.g<=0.04045)?(rhs.g/12.92):(math::Pow((rhs.g+0.055)/1.055,2.4));
 real32 b = (rhs.b<=0.04045)?(rhs.b/12.92):(math::Pow((rhs.b+0.055)/1.055,2.4));

 x = r*0.412424 + g*0.357579 + b*0.180464; // Sums to 0.950467
 y = r*0.212656 + g*0.715158 + b*0.0721856; // Sums to 0.9999996
 z = r*0.0193324 + g*0.119193 + b*0.950444; // Sums to 1.0889694

 return *this;
}

inline ColourXYZ & ColourXYZ::operator = (const ColourYcbcr & rhs)
{
 ColourRGB rgb = rhs;
 *this = rgb;
 return *this;       
}

inline ColourXYZ & ColourXYZ::operator = (const ColourLuv & rhs)
{
 if (!math::Equal(rhs.l,real32(0.0)))
 {
  const real32 u0 = (4.0*0.950467)/(0.950467 + 15.0*0.9999996 + 3.0*1.088969);
  const real32 v0 = (9.0)/(0.950467 + 15.0*0.9999996 + 3.0*1.088969);

  y = (rhs.l>((216.0/24389.0)*(24389.0/27.0)))?(math::Pow((rhs.l+16.0)/116.0,3.0)):(rhs.l/(24389.0/27.0));

  real32 a = (1.0/3.0)*(((52.0*rhs.l)/(rhs.u + 13.0*rhs.l*u0))-1.0); 
  real32 b = -5.0*y;
  real32 c = -1.0/3.0;
  real32 d = y*(((39.0*rhs.l)/(rhs.v+13.0*rhs.l*v0))-5.0);

  x = (d - b)/(a - c);
  z = x*a + b;
 }
 else
 {
  x = 0.0;
  y = 0.0;
  z = 0.0;
 }

 return *this;
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
  inline T & StreamRead(T & lhs,eos::bs::ColourL & rhs,Binary)
  {
   lhs >> rhs.l;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourL & rhs,Binary)
  {
   lhs << rhs.l;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourL & rhs,Text)
  {
   lhs << "l(" << rhs.l << ")";
   return lhs;
  }


  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::ColourRGB & rhs,Binary)
  {
   lhs >> rhs.r >> rhs.g >> rhs.b;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourRGB & rhs,Binary)
  {
   lhs << rhs.r << rhs.g << rhs.b;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourRGB & rhs,Text)
  {
   lhs << "rgb(" << rhs.r << "," << rhs.g << "," << rhs.b << ")";
   return lhs;
  }

  
  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::ColourRGBs & rhs,Binary)
  {
   lhs >> rhs.r >> rhs.g >> rhs.b;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourRGBs & rhs,Binary)
  {
   lhs << rhs.r << rhs.g << rhs.b;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourRGBs & rhs,Text)
  {
   lhs << "rgb(" << rhs.r << "," << rhs.g << "," << rhs.b << ")";
   return lhs;
  }
  
 
  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::ColourYcbcr & rhs,Binary)
  {
   lhs >> rhs.y >> rhs.cb >> rhs.cr;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourYcbcr & rhs,Binary)
  {
   lhs << rhs.y << rhs.cb << rhs.cr;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourYcbcr & rhs,Text)
  {
   lhs << "y'cbcr(" << rhs.y << "," << rhs.cb << "," << rhs.cr << ")";
   return lhs;
  }
  

  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::ColourLuv & rhs,Binary)
  {
   lhs >> rhs.l >> rhs.u >> rhs.v;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourLuv & rhs,Binary)
  {
   lhs << rhs.l << rhs.u << rhs.v;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourLuv & rhs,Text)
  {
   lhs << "luv(" << rhs.l << "," << rhs.u << "," << rhs.v << ")";
   return lhs;
  }
  
      
  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::ColourXYZ & rhs,Binary)
  {
   lhs >> rhs.x >> rhs.y >> rhs.z;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourXYZ & rhs,Binary)
  {
   lhs << rhs.x << rhs.y << rhs.z;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourXYZ & rhs,Text)
  {
   lhs << "xyz(" << rhs.x << "," << rhs.y << "," << rhs.z << ")";
   return lhs;
  }
  
  
 
  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::ColourLA & rhs,Binary)
  {
   lhs >> rhs.l >> rhs.a;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourLA & rhs,Binary)
  {
   lhs << rhs.l << rhs.a;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourLA & rhs,Text)
  {
   lhs << "la(" << rhs.l << "," << rhs.a << ")";
   return lhs;
  }


  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::ColourRGBA & rhs,Binary)
  {
   lhs >> rhs.r >> rhs.g >> rhs.b >> rhs.a;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourRGBA & rhs,Binary)
  {
   lhs << rhs.r << rhs.g << rhs.b << rhs.a;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourRGBA & rhs,Text)
  {
   lhs << "rgba(" << rhs.r << "," << rhs.g << "," << rhs.b << "," << rhs.a << ")";
   return lhs;
  }

  
  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::ColourRGBsA & rhs,Binary)
  {
   lhs >> rhs.r >> rhs.g >> rhs.b >> rhs.a;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourRGBsA & rhs,Binary)
  {
   lhs << rhs.r << rhs.g << rhs.b << rhs.a;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourRGBsA & rhs,Text)
  {
   lhs << "rgba(" << rhs.r << "," << rhs.g << "," << rhs.b << "," << rhs.a << ")";
   return lhs;
  }
  
 
  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::ColourYcbcrA & rhs,Binary)
  {
   lhs >> rhs.y >> rhs.cb >> rhs.cr >> rhs.a;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourYcbcrA & rhs,Binary)
  {
   lhs << rhs.y << rhs.cb << rhs.cr << rhs.a;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourYcbcrA & rhs,Text)
  {
   lhs << "y'cbcra(" << rhs.y << "," << rhs.cb << "," << rhs.cr << "," << rhs.a << ")";
   return lhs;
  }
  

  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::ColourLuvA & rhs,Binary)
  {
   lhs >> rhs.l >> rhs.u >> rhs.v >> rhs.a;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourLuvA & rhs,Binary)
  {
   lhs << rhs.l << rhs.u << rhs.v << rhs.a;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourLuvA & rhs,Text)
  {
   lhs << "luva(" << rhs.l << "," << rhs.u << "," << rhs.v << "," << rhs.a << ")";
   return lhs;
  }
  
      
  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::ColourXYZA & rhs,Binary)
  {
   lhs >> rhs.x >> rhs.y >> rhs.z >> rhs.a;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourXYZA & rhs,Binary)
  {
   lhs << rhs.x << rhs.y << rhs.z << rhs.a;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColourXYZA & rhs,Text)
  {
   lhs << "xyza(" << rhs.x << "," << rhs.y << "," << rhs.z << "," << rhs.a << ")";
   return lhs;
  }



  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::ColRGB & rhs,Binary)
  {
   lhs >> rhs.r >> rhs.g >> rhs.b;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColRGB & rhs,Binary)
  {
   lhs << rhs.r << rhs.g << rhs.b;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::ColRGB & rhs,Text)
  {
   lhs << "rgb{" << rhs.r << "," << rhs.g << "," << rhs.b << "}";
   return lhs;
  }

 
 };      
};
//------------------------------------------------------------------------------
#endif
