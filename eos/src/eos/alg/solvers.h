#ifndef EOS_ALG_SOLVERS_H
#define EOS_ALG_SOLVERS_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


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
