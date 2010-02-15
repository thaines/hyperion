#ifndef EOS_MATH_QUATERNIONS_H
#define EOS_MATH_QUATERNIONS_H
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


/// \file quaternions.h
/// Provides a standard quaternion implimentation, with some of the trimmings.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/math/functions.h"
#include "eos/math/vectors.h"
#include "eos/math/matrices.h"
#include "eos/math/mat_ops.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// A Quaternion, a real number with 3 complex numbers. 
/// For the purpose of rotations a simple relationship exists with the angle-axis
/// representation.
/// A 4 dimensional vector, the zeroth entry is the real component and the final
/// 3 are then i, j and k respectivly.
class EOS_CLASS Quaternion : public Vect<4>
{
 public:
  /// Does no actual initialisation, leaving the contents random.
   Quaternion() {}
  
  /// Initialises as a + bi + cj + dk.
   Quaternion(real32 a,real32 b,real32 c,real32 d)
   {(*this)[0] = a; (*this)[1] = b; (*this)[2] = c; (*this)[3] = d;}
   
  /// Sets the quaternion from an angle-axis representation where the
  /// axis is normalised. Creates a normalised quaternion.
   Quaternion(real32 angle,const Vect<3> & axis)
   {
    angle *= 0.5;
    real32 ca = math::Cos(angle);
    real32 sa = math::Sin(angle);
    
    (*this)[0] = ca;
    (*this)[1] = sa*axis[0];
    (*this)[2] = sa*axis[1];
    (*this)[3] = sa*axis[2];
   }
   
  /// &nbsp;
   Quaternion(const Quaternion & rhs):Vect<4>(rhs) {}
   
  /// &nbsp;
   ~Quaternion() {}


  /// &nbsp;
   Quaternion & operator *= (real32 v)
   {
    (*this)[0] *= v; (*this)[1] *= v; (*this)[2] *= v; (*this)[3] *= v;
    return *this;
   }
  
   Quaternion & operator *= (const Quaternion & rhs)
   {
    Quaternion lhs = *this;
     (*this)[0] = lhs[0]*rhs[0] - lhs[1]*rhs[1] - lhs[2]*rhs[2] - lhs[3]*rhs[3];
     (*this)[1] = lhs[0]*rhs[1] + lhs[1]*rhs[0] + lhs[2]*rhs[3] - lhs[3]*rhs[2];
     (*this)[2] = lhs[0]*rhs[2] + lhs[2]*rhs[0] + lhs[3]*rhs[1] - lhs[1]*rhs[3];
     (*this)[3] = lhs[0]*rhs[3] + lhs[3]*rhs[0] + lhs[1]*rhs[2] - lhs[2]*rhs[1];
    return *this;
   }

  /// &nbsp;
   Quaternion & operator /= (real32 v)
   {
    v = 1.0/v;
    return (*this) *= v;
   }
   
  /// &nbsp;
   Quaternion & operator /= (const Quaternion & rhs)
   {
    Quaternion invRhs = rhs;
    invRhs.Invert();
    return (*this) *= invRhs;
   }

 
  /// Inverts the Quaternion, so it represents the rotation that undoes its previous represente.
   void Invert()
   {
    Conjugate();
    *this /= LengthSqr();
   }
   
  /// Sets it to its conjugate, just negates elements 1 to 3 inclusive.
  /// Same as inverse if normalised.
   void Conjugate()
   {
    (*this)[1] = -(*this)[1];
    (*this)[2] = -(*this)[2];
    (*this)[3] = -(*this)[3];
   }

  /// When normalised this returns the angle of interpreting it in angle-axis
  /// form. The axis is simply the last 3 components of this 4-vector as a 3-vector.
  /// (i.e. the bit that is length 1 when the representation is normalised.)
   real32 Angle() const
   {
    return math::InvCos((*this)[0]) * 2.0;
   }
   
  
  /// Converts the quaternion to a 3x3 matrix. The templating allows you to give it a 4x4 matrix,
  /// for instance, and it will only set the top 3x3 corner.
  /// The quaternion must be normalised.
   template <typename M>
   void ToMat(M & out) const
   {    
    real32 aa = math::Sqr((*this)[0]);
    real32 ab = 2.0 * (*this)[0] * (*this)[1];
    real32 ac = 2.0 * (*this)[0] * (*this)[2];
    real32 ad = 2.0 * (*this)[0] * (*this)[3];

    real32 bb = math::Sqr((*this)[1]);
    real32 bc = 2.0 * (*this)[1] * (*this)[2];
    real32 bd = 2.0 * (*this)[1] * (*this)[3];
    
    real32 cc = math::Sqr((*this)[2]);
    real32 cd = 2.0 * (*this)[2] * (*this)[3];
    
    real32 dd = math::Sqr((*this)[3]);    
    
    out[0][0] = aa+bb-cc-dd; out[0][1] = bc-ad;       out[0][2] = ac-bd;
    out[0][0] = ad+bc;       out[0][1] = aa-bb+cc-dd; out[0][2] = cd-ab;
    out[0][0] = bd-ac;       out[0][1] = ab+cd;       out[0][2] = aa-bb-cc+dd;
   }
   
  /// This rotates a vector by the quaternion, simply uses the ToMat method
  /// to create a matrix which it then applies, as thats faster 
  /// than actually applying a quaternion directly.
  /// Note that if doing this repeatdly you should construct the matrix with
  /// ToMat and apply that repeatedly instead, this is just provided for one-off
  /// conveniance.
  /// in and out must not be the same vector.
   void Apply(const Vect<3> & in,Vect<3> & out)
   {
    Mat<3> mat;
    ToMat(mat);
    MultVect(mat,in,out);
   }
   
  
  /// Sets the quaternion to the Slerp interpolation of the two given
  /// quaternions. Do *not* give it itself, it will go horribly wrong.
  /// t is the interpolation factor, [0..1] please, with 0 being 
  /// assignment of q0 and 1 being assignment of q1.
  /// Fallsback on linear interpolation if they are too close,
  /// requires noamlised inputs and produces nomalised output.
   void Slerp(const Quaternion & q0,const Quaternion & q1,real32 t)
   {
    real32 dot = q0*q1;
    if (dot>0.999)
    {
     (*this)[0] = q0[0] + (q1[0]-q0[0])*t;
     (*this)[1] = q0[1] + (q1[1]-q0[1])*t;
     (*this)[2] = q0[2] + (q1[2]-q0[2])*t;
     (*this)[3] = q0[3] + (q1[3]-q0[3])*t;

     Normalise();
    }
    else
    {
     dot = math::Clamp(dot,real32(-1.0),real32(1.0)); // To avoid nasty death.
     real32 ta = math::InvCos(dot)*t;
     real32 tc = math::Cos(ta);
     real32 ts = math::Sin(ta);
     
     Quaternion qo(q1[0]-q0[0]*dot,q1[1]-q0[1]*dot,q1[2]-q0[2]*dot,q1[3]-q0[3]*dot);
     qo.Normalise();
     
     (*this)[0] = q0[0]*tc + qo[0]*ts;
     (*this)[1] = q0[1]*tc + qo[1]*ts;
     (*this)[2] = q0[2]*tc + qo[2]*ts;
     (*this)[3] = q0[3]*tc + qo[3]*ts;
    }
   }


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::math::Quaternion";}
};

//------------------------------------------------------------------------------
 };
};
#endif
