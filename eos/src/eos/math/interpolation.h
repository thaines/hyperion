#ifndef EOS_MATH_INTERPOLATION_H
#define EOS_MATH_INTERPOLATION_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file interpolation.h
/// Provides interpolation stuff, just a bunch of curvy functions.

#include "eos/types.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// This fits a parabola to 3 points, returning the position of the maximum of 
/// the parabola. Assumes the points are at -1, 0 and 1 positions.
/// The returned coordinate is ushally considered an offset from the centre 
/// points actual position.
template <typename T>
inline T ParabolaMax(T neg,T centre,T pos)
{
 T a = centre;
 T c = static_cast<T>(0.5)*(neg+pos) - a;
 T b = pos - a - c;
     
 if (!math::IsZero(c)) return -b/(2.0*c);
                  else return static_cast<T>(0);
}

//------------------------------------------------------------------------------
 };
};
#endif
