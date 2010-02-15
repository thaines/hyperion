#ifndef EOS_SFS_ZHENG_H
#define EOS_SFS_ZHENG_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file zheng.h
/// Contains an implimentation of the SfS component of 'Estimation of Illuminant
/// Direction, Albedo, and Shape from Shading' by Zheng and Chellappa.

#include "eos/types.h"

#include "eos/time/progress.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/arrays2d.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
/// Impliments of the SfS component of 'Estimation of Illuminant
/// Direction, Albedo, and Shape from Shading' by Zheng and Chellappa.
class EOS_CLASS Zheng
{
 public:
  /// &nbsp;
   Zheng();
   
  /// &nbsp;
   ~Zheng();


  /// Sets the image to use.
   void SetImage(const svt::Field<real32> & image);

  /// Sets an albedo map to use.
   void SetAlbedo(const svt::Field<real32> & albedo);

  /// Sets the light source direction, this comes as a normal that
  /// points towards the light at its infinite position.
  /// Note that this algorithm can not handle the light source (0,0,1), if it
  /// gets that it offsets it a little to avoid the problems that causes.
   void SetLight(bs::Normal & norm);
   
  /// Set the parameters to use.
   void SetParas(real32 mu = 1.0,nat32 iters = 512,real32 diffDelta = 0.0001);


  /// Blah.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the calculated needle map.
   void GetNeedle(svt::Field<bs::Normal> & out) const;
   
  /// Extracts the calculated depth map. Note that its reversed, 
  /// larger means closer to the camera. Also note that the average depth is 
  /// arbitary, though initialised to 0.
   void GetDepth(svt::Field<real32> & out) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  // Parameters...
   nat32 iters; // Number of iters per hierachy level to do.
   real32 mu;
   real32 diffDelta;
   
  // Helper structures...
   struct Pixel
   {
    real32 p; // dz/dx
    real32 q; // dz/dy
    real32 z;

    Pixel & operator = (const Pixel & rhs)
    {
     p = rhs.p;
     q = rhs.q;
     z = rhs.z;
     return *this;
    }

    Pixel & operator += (const Pixel & rhs)
    {
     p += rhs.p;
     q += rhs.q;
     z += rhs.z;
     return *this;
    }
   };
  
  // Inputs...
   svt::Field<real32> image;
   svt::Field<real32> albedo;
   bs::Normal toLight;
   
  // Outputs...
   ds::Array2D<Pixel> out;


  // Helper methods...
   void DoIterations(ds::Array2D<real32> & irr,ds::Array2D<Pixel> & data,ds::Array2D<Pixel> & delta,time::Progress * prog);
};

//------------------------------------------------------------------------------
 };
};
#endif
