#ifndef EOS_STEREO_DIFFUSE_CORRELATION_H
#define EOS_STEREO_DIFFUSE_CORRELATION_H
//------------------------------------------------------------------------------
// Copyright 2009 Tom Haines

/// \file diffuse_correlation.h
/// Provides advanced correlation capabilities, that weight pixels via diffusion
/// and use colour ranges rather than points.

#include "eos/types.h"
#include "eos/time/progress.h"
#include "eos/bs/luv_range.h"


namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// Diffusion weight object - stores the weights associated with going in each
/// direction from each pixel in an image. Has a method to calculate from a
/// LuvRangeImage using a LuvRangeDist.
class EOS_CLASS DiffusionWeight
{
 public:
  /// &nbsp;
   DiffusionWeight();
  
  /// &nbsp;
   ~DiffusionWeight();


  /// Fills in the object from a LuvRangeImage and a LuvRangeDist. It takes the
  /// negative exponential of the distances to get relative weightings, and 
  /// makes an effort for stability. It supports a distance multiplier before 
  /// it does this and respects masks by not sending any weight that way, or off
  /// the image.
   void Create(const bs::LuvRangeImage & img, const bs::LuvRangeDist & dist, real32 distMult = 1.0, time::Progress * prog = null<time::Progress*>());


  /// Returns the weight for a given pixel for a given direction. Note that this
  /// weight will be normalised between the 4 directions, which have the typical
  /// 0=+ve x, 1=+ve y,2=-ve x,3=-ve y coding, unless the pixel is invalid in 
  /// which case they will all be zero.
   real32 Get(nat32 x,nat32 y,nat32 dir) const {return data.Get(x,y).dir[dir];}


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::DiffusionWeight";}


 private:
  struct Weight
  {
   real32 dir[4];
  };
  
  ds::Array2D<Weight> data;
};

//------------------------------------------------------------------------------
/// Given a LuvRangeImage and a scanline number this calculates a slice of 
/// diffusion scores, for a given number of steps.
/// Clever enough to cache storage between runs as long as the image
/// width and step count don't change.
/// Once done each pixel in the scanline will have a normalised set of weights 
/// for surrounding pixels within the given walking distance. Note that this is
/// never going to be that fast. It will always give values of zero to masked or
/// out of bound values.
class EOS_CLASS RangeDiffusionSlice
{
 public:
  /// &nbsp;
   RangeDiffusionSlice();

  /// &nbsp;
   ~RangeDiffusionSlice();
   
   
  /// Creates the data for a diffusion slice - you give it an image, the 
  /// y-coordinate of the slice to calculate, how many steps to walk and a 
  /// diffusion weight object and it constructs the slices diffuson masks, one for
  /// each pixel in the slice.
  /// Will not walk off edges of the image, or into masked off areas.
  /// For weighting from the distances takes the negative exponential of
  /// distance. (And offsets first, for stability.)
   void Create(nat32 y, nat32 steps, const bs::LuvRangeImage & img, const DiffusionWeight & dw, time::Progress * prog = null<time::Progress*>());


  /// Returns the width of the slice.
   nat32 Width() const;
   
  /// Returns the number of steps of the slice.
   nat32 Steps() const;
   
  /// Given a x-coordinate and a (u,v) window coordinate this returns the weight
  /// - out of range values will return 0.0, defined by abs(u) + abs(v) > steps.
   real32 Get(nat32 x,int32 u,int32 v) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::RangeDiffusionSlice";} 


 private:
  // Storage for state...
   nat32 steps;
   ds::Array2D<real32> data; // Stores the x coordinate in x and the y coordinate is a linearlisation of the diffusion values. Will not have anything for masked entries.
   ds::Array2D<nat32> offset; // Index from (u+steps,v+steps) to the above linearisation. Only valid when abs(u) + abs(v) <= steps.
};

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
 };
};
#endif
