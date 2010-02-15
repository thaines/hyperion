#ifndef EOS_MYA_SPHERES_H
#define EOS_MYA_SPHERES_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file spheres.h
/// Provides an implimentation of a Surface type for spheres.

#include "eos/mya/surfaces.h"
#include "eos/ds/lists.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
/// An implimentation of a sphere surface representation, see parent for details.
/// A slight variation on a sphere, in that it is only a half-sphere, with a
/// depth scaler so it can be squished and explicitly stick out/in depending on
/// the data.
class EOS_CLASS SphereSurface : public Surface
{
 public:
  /// &nbsp;
   SphereSurface();

  /// &nbsp;
   ~SphereSurface();

  /// Returns a copy of this object, you must ultimatly call delete on the returned pointer.
   SphereSurface * Clone() const;


  /// &nbsp;
   bit operator == (const Surface & rhs) const;

  /// &nbsp;
   void Get(real32 x,real32 y,math::Vect<2> & z) const;

  /// &nbsp;
   void GetDelta(real32 x,real32 y,math::Vect<2> & dx,math::Vect<2> & dy) const;


  // For editting, internal use only really...
   math::Vect<3> & Location();
   real32 & Radius(); // Must be >0.
   real32 & Scale();

  /// &nbsp;
   cstrconst TypeString() const;
   
 private:
  // A simple location/radius representation, with the squishing parameter...
   math::Vect<3> loc;
   real32 rad;
   real32 scale;
};

//------------------------------------------------------------------------------
///  A sphere based surface fitter, see parent for details.
class EOS_CLASS SphereFitter : public Fitter
{
 public:
  /// &nbsp;
   SphereFitter();
   
  /// &nbsp;
   ~SphereFitter();


  /// &nbsp;
   void Reset();

  /// &nbsp;
   bit Add(nat32 type,real32 x,real32 y,const math::Vector<real32> & data);

  /// &nbsp;
   bit Extract(Surface & out) const;


  /// &nbsp;
   cstrconst TypeString() const;
   
 private:
  static const real32 angMult = 0.9; // The multiplier of angles to weight them compared to distance in the error functions.

  // We store a linked list of typed data collected, plus counters of each data
  // type so we know when enough data is avaliable...
   nat32 dispCount;
   nat32 needleCount;
   nat32 sfsCount;
   
   struct Node
   {
    enum {disp,needle,sfs} type;
    math::Vect<6> v; // The data for the sphere fitting.
   };
   ds::List<Node> data;
   
  // Error function for LM...
   static void LMfunc(const math::Vector<real32> & pv,math::Vector<real32> & err,const SphereFitter & self);
};

//------------------------------------------------------------------------------
/// A sphere based surface type, see parent for details. Supports the following data types:
/// - 'disp' - disparity data, represented as a 2 vector being first the focal length 
/// multiplied by the distance between cameras and second the disparity value. (This happens
/// to be a homogenous depth representation.)
/// - 'needle' - normals, represented as simple 3 vectors.
/// - 'sfs' - sfs data, represented by a 4 vector, first a light directon normal followed by
/// the cos of the angle from the light direction to the surface normal at that point. 
/// (Essentially a definition of a cone.) Note that the last item, the angle, must not be 0 
/// or negative.
class EOS_CLASS SphereSurfaceType : public SurfaceType
{
 public:
  /// &nbsp;
   SphereSurfaceType();
 
  /// &nbsp;
   ~SphereSurfaceType();


  /// &nbsp;
   SphereSurface * NewSurface() const;

  /// &nbsp;
   SphereFitter * NewFitter() const;
   
   
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
