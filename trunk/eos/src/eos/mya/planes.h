#ifndef EOS_MYA_PLANES_H
#define EOS_MYA_PLANES_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file planes.h
/// Provides an implimentation of a Surface type for planes.

#include "eos/mya/surfaces.h"
#include "eos/ds/lists.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
/// An implimentation of a plane surface representation, see parent for details.
class EOS_CLASS PlaneSurface : public Surface
{
 public:
  /// &nbsp;
   PlaneSurface();

  /// &nbsp;
   ~PlaneSurface();

  /// Returns a copy of this object, you must ultimatly call delete on the returned pointer.
   PlaneSurface * Clone() const;


  /// &nbsp;
   bit operator == (const Surface & rhs) const;

  /// &nbsp;
   void Get(real32 x,real32 y,math::Vect<2> & z) const;

  /// &nbsp;
   void GetDelta(real32 x,real32 y,math::Vect<2> & dx,math::Vect<2> & dy) const;


  // For editting, internal use only really...
  // Note the requirements on the first 4 entrys being normalised.
   math::Vect<4> & Data();

  /// &nbsp;
   cstrconst TypeString() const;
   
 private:
  // A homogenous representation, the dot product of this with a homogenous point
  // returns 0 if its on the plane.
  // We scale this vector so the first 3 components as a vector have length 1,
  // to make comparisons easy. (First 3 form plane normal with last distance from
  // origin with this scaling.)
   math::Vect<4> plane;
};

//------------------------------------------------------------------------------
///  A plane based surface fitter, see parent for details.
class EOS_CLASS PlaneFitter : public Fitter
{
 public:
  /// &nbsp;
   PlaneFitter();
   
  /// &nbsp;
   ~PlaneFitter();


  /// &nbsp;
   void Reset();

  /// &nbsp;
   bit Add(nat32 type,real32 x,real32 y,const math::Vector<real32> & data);

  /// &nbsp;
   bit Extract(Surface & out) const;


  /// &nbsp;
   cstrconst TypeString() const;
   
 private:
  static const real32 angMult = 0.02; // The multiplier of angles to weight them compared to distance in the error functions.
 
  // We store a linked list of typed data collected, plus counters of each data
  // type so we know when enough data is avaliable...
   nat32 dispCount;
   nat32 needleCount;
   nat32 sfsCount;
   
   struct Node
   {
    enum {disp,needle,sfs} type;
    math::Vect<4> v; // The 4 vector from the lhs matrix followed by the single entry in the vector on the rhs.
   };
   ds::List<Node> data;
   
  // Error metric function for LM...
   static void LMfunc(const math::Vector<real32> & pv, math::Vector<real32> & err, const PlaneFitter & self);
};

//------------------------------------------------------------------------------
/// A plane based surface type, see parent for details. Supports the following data types:
/// - 'disp' - disparity data, represented as a 2 vector being first the focal length 
/// multiplied by the distance between cameras and second the disparity value. (This happens
/// to be a homogenous depth representation.)
/// - 'needle' - normals, represented as simple 3 vectors.
/// - 'sfs' - sfs data, represented by a 4 vector, first a light directon normal followed by
/// the cos of the angle from the light direction to the surface normal at that point. 
/// (Essentially a definition of a cone.) Note that the last item, the angle, must not be 0 
/// or negative.
class EOS_CLASS PlaneSurfaceType : public SurfaceType
{
 public:
  /// &nbsp;
   PlaneSurfaceType();
 
  /// &nbsp;
   ~PlaneSurfaceType();


  /// &nbsp;
   PlaneSurface * NewSurface() const;

  /// &nbsp;
   PlaneFitter * NewFitter() const;
   
   
  /// &nbsp;
   bit IsMember(Surface * s) const;
   
   
  /// &nbsp;
   bit Supports(str::TokenTable & tt,str::Token tok,nat32 & outType) const;

  /// &nbsp;
   nat32 Degrees() const;
   
  /// &nbsp;
   nat32 TypeDegrees(nat32 type) const;


  /// &nbsp;
   cstrconst TypeString() const; 
};

//------------------------------------------------------------------------------
 };
};
#endif
