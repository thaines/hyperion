#ifndef EOS_MATH_BESSEL_H
#define EOS_MATH_BESSEL_H
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


/// \file bessel.h
/// Provides functions for calculating bessel functions.

#include "eos/types.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// Calculates the modified bessel function of the first kind.
/// The order is given as twice the order, so it supports only whole and half orders.
/// Iterative in nature, accuracy indicates how accurate it should be.
/// Limit is a cap on how many iterations.
/// Highest input this can take is 84, to produce a value of about 1e35, any
/// larger and it will return a finite but wrong answer.
EOS_FUNC real32 ModBesselFirst(nat32 orderX2,real32 x,real32 accuracy = 0.0001,nat32 limit = 1000);

//------------------------------------------------------------------------------
/// Calculates the inverse modified bessel function of the first kind, order 0.
/// Iterative in nature, accuracy indicates how accurate it should be; limit is 
/// an iteration cap. Note that this is iterative in several dimensions all at
/// once, put simply, its slow. (It uses Newton iterations on a function that is
/// calculated by iteration.)
/// Highest value this will return is about 90, which is an input of around 1e38
/// - any higher and you will get a nan.
EOS_FUNC real32 InverseModBesselFirstOrder0(real32 x,real32 accuracy = 0.0001,nat32 limit = 1000);

//------------------------------------------------------------------------------
 };
};
#endif
