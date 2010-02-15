#ifndef EOS_STEREO_DIFFUSE_CORRELATION_H
#define EOS_STEREO_DIFFUSE_CORRELATION_H
//------------------------------------------------------------------------------
// Copyright 2009 Tom Haines

/// \file diffuse_correlation.h
/// Provides advanced correlation capabilities, that weight pixels via diffusion
/// and use colour ranges rather than points.

#include "eos/types.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// Given a LuvRangeImage and a scanline number this calculates a slice of 
/// diffusion scores, for a given number of steps, weighted by a given distance
/// metric. Clever enough to cache storage between runs as long as the image
/// width and step count don't change.
/// Once done each pixel in the scanline will have a normalised set of weights 
/// for surrounding pixels within the given walking distance. Note that this is
/// never going to be that fast.
class EOS_CLASS RangeDiffusionSlice
{
 public:
  /// &nbsp;
   RangeDiffusionSlice();

  /// &nbsp;
   ~RangeDiffusionSlice();


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::RangeDiffusionSlice";} 


 private:
  
};

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
 };
};
#endif
