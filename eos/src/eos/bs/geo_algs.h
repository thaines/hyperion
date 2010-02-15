#ifndef EOS_BS_GEO_ALGS_H
#define EOS_BS_GEO_ALGS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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


/// \file geo_algs.h
/// Provides geometric algorithms, for working with geometric 2D and 3D primatives.

#include "eos/types.h"
#include "eos/math/functions.h"
#include "eos/math/vectors.h"
#include "eos/bs/geo2d.h"
#include "eos/bs/geo3d.h"

namespace eos
{
 namespace bs
 {
//------------------------------------------------------------------------------
/// This is given a line and a plane, it calculates the intersection in terms of
/// the line position. Returns true on success, false on failure. (failure 
/// implying the line and plane are parrellel.)
EOS_FUNC bit IntPlaneLine(const Line & l,const Plane & p,real32 & out);

//------------------------------------------------------------------------------
/// This calculates a measure of 'distance' between two planes, as defined by the paper
///' A global matching framework for stereo computation' by Tao, Sawhney and Kumar.
EOS_FUNC real32 PlaneDistance(const Pnt & l1,const PlaneABC & p1,const Pnt & l2,const PlaneABC & p2);

//------------------------------------------------------------------------------
 };
};
#endif
