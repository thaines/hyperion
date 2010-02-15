#ifndef EOS_FILTER_SMOOTHING_H
#define EOS_FILTER_SMOOTHING_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file smoothing.h
/// Provides smoothing algorithms that arn't simple Gaussian blurs.

#include "eos/types.h"

#include "eos/time/progress.h"
#include "eos/svt/field.h"
#include "eos/bs/colours.h"


namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This function smooths an image under the assumption that its been
/// quantisised and therefore for each value there is equal probability of it 
/// being within a given range of the value given, but zero of it being outside
/// - a top hat function. Uses diffusion to select the best n neighbours, then
/// uses linear interpolation to work out the ideal value for the pixel. It then
/// sets it to the closest value to the ideal within the range, i.e. it clamps.
/// This is done repeatedly until convergance, not many iterations are required.
/// \param in Input field to be smoothed. Presumed 0..1.
/// \param out Output field, can be the same as the input field.
/// \param dist Luv field used to find the distance between pixels for the diffusion selection.
/// \param walkLength How long the diffusions walk shall be.
/// \param fitN How many to select from the diffusion set for the linear fitting.
/// \param distMult Multiplier of distance, weightings for diffusion are calculated as exp(-distMult*{luv dist})
/// \param prog Progress bar.
/// \param levels How many levels the field has been quantisised by.
/// \param iters How many iterations to do.
EOS_FUNC void QuantSmooth(const svt::Field<real32> & in,svt::Field<real32> & out,
                          const svt::Field<bs::ColourLuv> & dist,
                          nat32 walkLength = 3,nat32 fitN = 8,real32 distMult = 1.0,
                          time::Progress * prog = null<time::Progress*>(),
                          nat32 levels = 256,nat32 iters = 16);

/// Similar to QuantSmooth, but less sophisticated. Minimises the local cost at
/// each pixel of capped differences with neighbours, keeping the pixel within
/// its relevant range. The cap is applied to the sum of red, green and blue 
/// differences, rather than each difference individually.
/// This makes wildly different pixels ineffective.
/// Iterative position updates with a checkboard pattern, gets stuck in local 
/// minimas but initialisation is by design so close that this doesn't really
/// matter.
EOS_FUNC void SimpleQuantSmooth(svt::Field<bs::ColourRGB> & inOut,
                                real32 cap = 0.1,time::Progress * prog = null<time::Progress*>(),
                                nat32 levels = 256,nat32 iters = 64);
//------------------------------------------------------------------------------
 };
};
#endif
