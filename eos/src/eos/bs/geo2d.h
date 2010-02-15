#ifndef EOS_BS_GEO2D_H
#define EOS_BS_GEO2D_H
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


/// \file geo2d.h
/// Provides 2d geometric primatives, such as points, squares and circles.

#include "eos/types.h"
#include "eos/math/functions.h"
#include "eos/math/vectors.h"

namespace eos
{
 namespace bs
 {
//------------------------------------------------------------------------------
/// An integer point, non-homogenous, inherits from and hence provides the 
/// functionality of the Vect class.
class EOS_CLASS Pos : public math::Vect<2,int32>
{
 public:
  /// Leaves the contained data random.
   Pos() {}

  /// &nbsp;
   Pos(int32 x,int32 y)
   {
    (*this)[0] = x; (*this)[1] = y;
   }

  /// &nbsp;
   Pos(const Pos & rhs):math::Vect<2,int32>(rhs) {}

  /// &nbsp;
   ~Pos() {}


  /// &nbsp;
   int32 & X() {return (*this)[0];}

  /// &nbsp;
   int32 & Y() {return (*this)[1];}


  /// &nbsp;
   const int32 & X() const {return (*this)[0];}

  /// &nbsp;
   const int32 & Y() const {return (*this)[1];}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::geo2d::Pos";}
};

//------------------------------------------------------------------------------
/// A rectangle.
class EOS_CLASS Rect
{
 public:
  /// &nbsp;
   Rect() {}
   
  /// &nbsp;
   Rect(const Pos & lowP,const Pos & highP):low(lowP),high(highP) {}
   
  /// &nbsp;
   ~Rect() {}

 
  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::geo2d::Rect";}
   
     
  /// The corner with the lowest two values.
   Pos low;
   
  /// The corner with the highest two values.
   Pos high;
};

//------------------------------------------------------------------------------
/// A Point, non-homogenous, inherits from and hence provides the 
/// functionality of the Vect class.
class EOS_CLASS Pnt : public math::Vect<2>
{
 public:
  /// Leaves the contained data random.
   Pnt() {}

  /// &nbsp;
   Pnt(real32 x,real32 y)
   {
    (*this)[0] = x; (*this)[1] = y;
   }

  /// &nbsp;
   Pnt(const Pnt & rhs):math::Vect<2>(rhs) {}

  /// &nbsp;
   Pnt(const class Point & rhs);

  /// &nbsp;
   ~Pnt() {}


  /// &nbsp;
   real32 & X() {return (*this)[0];}

  /// &nbsp;
   real32 & Y() {return (*this)[1];}


  /// &nbsp;
   const real32 & X() const {return (*this)[0];}

  /// &nbsp;
   const real32 & Y() const {return (*this)[1];}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::geo2d::Pnt";}
};

//------------------------------------------------------------------------------
/// A Vertex, homogenous in nature, inherits from and hence provides the 
/// functionality of the Vect class.
class EOS_CLASS Point : public math::Vect<3>
{
 public:
  /// Leaves the contained data random.
   Point() {}

  /// &nbsp;
   Point(real32 x,real32 y,real32 w = 1.0)
   {
    (*this)[0] = x; (*this)[1] = y; (*this)[2] = w;
   }

  /// &nbsp;
   Point(const Pnt & rhs)
   {
    (*this)[0] = rhs[0]; (*this)[1] = rhs[1]; (*this)[2] = 1.0;
   }

  /// &nbsp;
   Point(const Point & rhs):math::Vect<3>(rhs) {}

  /// &nbsp;
   ~Point() {}


  /// &nbsp;
   real32 & X() {return (*this)[0];}

  /// &nbsp;
   real32 & Y() {return (*this)[1];}

  /// &nbsp;
   real32 & W() {return (*this)[2];}


  /// &nbsp;
   const real32 & X() const {return (*this)[0];}

  /// &nbsp;
   const real32 & Y() const {return (*this)[1];}

  /// &nbsp;
   const real32 & W() const {return (*this)[2];}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::geo2d::Point";}
};

//------------------------------------------------------------------------------
// All the stuff that has to be defined out-of-order due to circular dependencies...
inline Pnt::Pnt(const class Point & rhs)
{
 real32 we = 1.0/rhs[2];
 (*this)[0] = rhs[0]*we;
 (*this)[1] = rhs[1]*we;
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
  inline T & StreamRead(T & lhs,eos::bs::Rect & rhs,Binary)
  {
   lhs >> rhs.low >> rhs.high;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Rect & rhs,Binary)
  {
   lhs << rhs.low << rhs.high;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Rect & rhs,Text)
  {
   lhs << rhs.low << "-" << rhs.high;
   return lhs;
  }


 };      
};
//------------------------------------------------------------------------------
#endif
