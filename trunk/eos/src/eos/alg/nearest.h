#ifndef EOS_ALG_NEAREST_H
#define EOS_ALG_NEAREST_H
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


/// \file nearest.h
/// Provides code to calculate nearest points for various primatives.

#include "eos/types.h"
#include "eos/math/vectors.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
/// This calculates the nearest point on an ellipsoid, that is axis aligned and 
/// at the origin, to a point in 3D space. Outputs the relevant point.
/// Note that it normalises its input, to maintain numerical stability.
/// Only one of the below is true:
/// (Implimentation derived from Graphics Gems IV, P.113 - P.119, Distance to an Ellipsoid, by John C. Hart.)
/// (Approach taken from www.geometrictools.com/Documentation/DistancePointToEllipsoid.pdf.)
/// (Alternate approach given in www.ma.ic.ac.uk/~rn/distance2ellipse.pdf was tested - despite claims otherwise its conditioning is even worse.)
/// \param ell The ellipsoids scallers for each of the axes.
/// \param point The point to get the closest to. Note that if its the origin the result will be quite random.
/// \param out Output point, the point on the ellipse closest to the given point.
/// \param tol Tolerance to try and obtain.
/// \param limit Maximum number of iterations to use trying to obtain the tolerance.
EOS_FUNC void PointEllipsoid(const math::Vect<3> & ell,const math::Vect<3> & point,math::Vect<3> & out,
                             real32 tol = 1e-6,nat32 limit = 1000);

//------------------------------------------------------------------------------
/// Given an ellipsoid that is axis aligned and at the origin this calculates 
/// all the points where the surface normal points at a given point, unless the
/// given point is in the centre in which case it returns no points to indicate 
/// all points.
/// By definition this will include the closest point on the ellipsoid to the
/// given point.
/// Warning: Not convinced this actually works.
/// \param ell The ellipsoids scallers for each of the axes.
/// \param point The point to for output points to point towards.
/// \param out Output points.
/// \param tol Tolerance to try and obtain.
/// \param limit Maximum number of iterations to use trying to obtain the tolerance.
/// \returns The number of points found, upto a maximum of 6.
EOS_FUNC nat32 PointEllipsoidDir(const math::Vect<3> & ell,const math::Vect<3> & point,math::Vect<3> out[6],
                                 real32 tol = 1e-6,nat32 limit = 1000);

//------------------------------------------------------------------------------
 };
};
#endif
