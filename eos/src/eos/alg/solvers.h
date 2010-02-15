#ifndef EOS_ALG_SOLVERS_H
#define EOS_ALG_SOLVERS_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file solvers.h
/// Provides highly specialist solvers, for solving specific equations.
/// Quite esoteric.

#include "eos/types.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
/// Given the equation a*x + b*y + c*x^2 + d*y^2 where x^2+y^2 = n this outputs
/// the x and y that maximise the equation. Internally remaps the equation to a
/// polynomial that produces all the extrema locations and some other points, it
/// then tests them to find and output the best.
EOS_FUNC void MaximiseFixedLenDualQuad(real32 a,real32 b,real32 c,real32 d,real32 n,real32 & outX,real32 & outY);

//------------------------------------------------------------------------------
 };
};
#endif
