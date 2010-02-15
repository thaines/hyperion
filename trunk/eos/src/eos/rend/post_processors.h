#ifndef EOS_REND_POST_PROCESSORS_H
#define EOS_REND_POST_PROCESSORS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file post_processors.h
/// Provides post-processors that do various operations to a ray image.

#include "eos/types.h"
#include "eos/rend/renderer.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// This adds noise to the image. This works by adding two gaussian 
/// distributions with mean zero, plus a constant, to the original light level.
/// The first gaussian, representing thermal noise, has a constant standard 
/// deviation set.
/// The second has a standard deviation calculated as the product of a user suplied 
/// constant and the original light level, this represents photon noise.
/// The constant represents the sensor bias, the fact that zero is never really
/// output. This is all an approximation obviously, but its better than a simple 
/// gaussian noise model. As the gaussian noise can reduce the level below zero
/// a check is applied to prevent this happenning. Standard deviations of zero 
/// are accepted to switch off components of the noise model.
class EOS_CLASS PostNoise : public PostProcessor
{
 public:
  /// Initialises it to have no effect - you must call Set or the class is 
  /// an official chocolate fireguard.
   PostNoise(Random & r):rand(r),perRay(false),bias(0.0),sd1(0.0),sdm2(0.0) {}
   
  /// &nbsp;
   ~PostNoise() {}


  /// Sets the noise parameters, bias is added to all values, sd1 is the 
  /// constant standard deviation and sdm2 is the multiplier of the 
  /// original response to get the standard deviation of the second gaussian.
  /// If perRay is true it adds the noise to each sampled ray, if false 
  /// it adds it on a per pixel basis. You will generally want per pixel, 
  /// as per ray and you will have to compensate for any anti-aliasing.
   void Set(real32 bias,real32 sd1,real32 sdm2,bit perRay = false);
   
  /// &nbsp;
   real32 Bias() const {return bias;}
   
  /// &nbsp;
   real32 Sd1() const {return sd1;}
   
  /// &nbsp;
   real32 Sdm2() const {return sdm2;}
   
  /// &nbsp;
   bit PerRay() const {return perRay;}


  /// &nbsp;
   void Apply(RayImage & ri,time::Progress * prog) const;


  /// &nbsp; 
   cstrconst TypeString() const {return "eos::rend::PostNoise";}


 private:
  Random & rand;

  bit perRay;
  real32 bias;
  real32 sd1;
  real32 sdm2;
};

//------------------------------------------------------------------------------
 };
};
#endif
