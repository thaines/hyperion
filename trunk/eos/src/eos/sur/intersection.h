#ifndef EOS_SUR_INTERSECTION_H
#define EOS_SUR_INTERSECTION_H
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


/// \file intersection.h
/// Contains basic intersection algorithms between various primatives.

#include "eos/types.h"
#include "eos/bs/geo3d.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
/// This intersects a ray with a triangle. Returns true on success, false on
/// failure. On returning true it also supply the distance traveled along the
/// way to the point of collision and the triangular coordinates of the point of
/// collision. (These, rather conveniantly, being the weights for any
/// interpolation from the 3 vertices.)
EOS_FUNC bit RayTriIntersect(bs::Ray & ray,
                             const bs::Vert & a,const bs::Vert & b,const bs::Vert & c,
                             real32 & dist,real32 & wa,real32 & wb,real32 & wc);

//------------------------------------------------------------------------------
 };
};
#endif
