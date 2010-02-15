#ifndef EOS_MATH_CONSTANTS_H
#define EOS_MATH_CONSTANTS_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

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


/// \file constants.h
/// \brief Contains various mathematical constants.

/// \namespace eos::math
/// This namespace covers everything mathematical, including mathematical 
/// constants, functions and classes to manage big numbers, vectors, quaternions,
/// matrices, colours etc. Particular emphasis is on matrices for solving 
/// linear equations. The library follows the principal that a mathematical 
/// function should return the same type as its called with, hence the templates.
/// This also allows overloading for particular types. The real32 type is 
/// overloaded to emphasis speed, leaving the real64 type for accurate 
/// calculation. This is because actual work is ushally done with real64's,
/// where accuracy matters, but real32's are ushally reserved for 
/// visulisation or real time calculations, and as such a loss of accuracy for
/// an increase in speed is prefable.

#include "eos/types.h"

#ifndef _ISOC99_SOURCE
#define _ISOC99_SOURCE
#endif
#include <math.h>

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// If I have to explain this to you then your obviously not intelligent enough
/// to be reading this. Or you know it by another name, but as far as I know
/// this one is as universal as it gets. And that would make you an Alien. So,
/// welcome to the planet. If you feel like wasting our pathetic species could 
/// you please start with the politicians, then marketting followed by telephone
/// salesman. After that it is my sincerest belief that the world will be such
/// a better place that your desire to eat/hunt/obliterate us will have 
/// subsided, and we can live in peaceful harmony. At least till some
/// religion declares a crusade/jihad on you and gets everybody nuked. So wipe
/// out all violent religions whilst your at it. The robes will make for good wraps,
/// just add salsa.
const real64 pi = 3.14159265358979323846264338327950288;

/// See pi.
const real64 e = 2.71828182845904523536028747135266249;

/// Otherwise known as the golden ratio. Not sure why this is here, its 
/// unlikelly to ever be used. But it looks good.
const real64 phi = 1.61803398874989484820458683436563811;

/// sqrt(2), provided as a constant because its a popular number. Why I don't
/// know. I think 1 kicks its arse any day of the week.
const real64 sqrt2 = 1.41421356237309504880168872420969807;

/// sqrt(3). Whatever. I'm sure it looks dead sexy in a mini skirt.
const real64 sqrt3 = 1.732050807568877293527446341505;


/// This returns infinity, should only be templated to real32 or real64.
template <typename T>
inline T Infinity()
{
 return INFINITY;
}

//------------------------------------------------------------------------------
/// Minimum value an int8 can take.
const int8 min_int_8 = -128;

/// Maximum value an int8 can take.
const int8 max_int_8 = 127;

/// Minimum value a nat8 can take.
const nat8 min_nat_8 = 0;

/// Maximum value a nat8 can take.
const nat8 max_nat_8 = 255;


/// Minimum value an int16 can take.
const int16 min_int_16 = -32768;

/// Maximum value an int16 can take.
const int16 max_int_16 = 32767;

/// Minimum value a nat16 can take.
const nat16 min_nat_16 = 0;

/// Maximum value a nat16 can take.
const nat16 max_nat_16 = 65535;


/// Minimum value an int32 can take.
const int32 min_int_32 = int32(0x80000000);

/// Maximum value an int32 can take.
const int32 max_int_32 = 2147483647;

/// Minimum value a nat32 can take.
const nat32 min_nat_32 = 0;

/// Maximum value a nat32 can take.
const nat32 max_nat_32 = 0xFFFFFFFF;


//------------------------------------------------------------------------------
 };
};
#endif
