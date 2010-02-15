#ifndef EOS_FIT_IMAGE_SPHERE_H
#define EOS_FIT_IMAGE_SPHERE_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file image_sphere.h
/// Provides a tool to fit a sphere to an image given a list of pixels on the
/// occluding edge of the sphere.

#include "eos/types.h"
#include "eos/time/progress.h"
#include "eos/bs/geo2d.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/arrays.h"
#include "eos/ds/lists.h"
#include "eos/cam/files.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
/// This class calibrates the position of a sphere in 3D space given a cameras
/// calibration, the spheres radius and a set of pixels on the edge of the
/// sphere. Simply uses LM on the obvious optimisation problem.
class EOS_CLASS ImageOfSphere
{
 public:
  /// &nbsp;
   ImageOfSphere();
  
  /// &nbsp;
   ~ImageOfSphere();
   
  
  /// Sets the camera to use.
   void SetCamera(const cam::CameraFull & cf);
   
  /// Sets the radius of the sphere, defaults to 1.
   void SetRadius(real32 rad); 
   
  /// Adds a pixel coordinate - must be in the coordinate system of the camera.
  /// These coordinates are the edge of the sphere in the image - i.e. rays cast
  /// from them should graze the output sphere.
  /// Must be called at least 3 times before Run, assuming no degeneracy.
   void AddPixel(real32 x,real32 y);


  /// Runs the algorithm - returns true on success, false on failure...
   bit Run(time::Progress * prog = null<time::Progress*>());


  /// Returns the position of the light source.
   const bs::Vert & Centre() const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::fit::ImageOfSphere";}


 private:
  // Input...
   cam::CameraFull cf;
   real32 radius;
   
   ds::List<bs::Pnt> edge;
   
  // Output...
   bs::Vert centre;
   
  // Error function for LM...
   struct ErrData
   {
    ds::Array<bs::Ray> ray;
    real32 radius;
   };
   static void ErrorFunc(const math::Vect<3,real32> & pv,math::Vect<1,real32> & err,const ErrData & oi);
   
  // Function that calculates the centre of a sphere given 3 rays from the same
  // starting position and a radius...
   static void SphereFromRays(real32 radius,const bs::Vert & start,
                              const bs::Normal & rayA,const bs::Normal & rayB,const bs::Normal & rayC,
                              bs::Vert & out);
                              
};

//------------------------------------------------------------------------------
 };
};
#endif
