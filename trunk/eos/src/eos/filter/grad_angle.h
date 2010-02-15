#ifndef EOS_FILTER_GRAD_ANGLE_H
#define EOS_FILTER_GRAD_ANGLE_H
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


/// \file grad_angle.h
/// Provides methods to calculate the gradiant and angle of each pixel in an 
/// image, using a given differentiation.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/filter/kernel.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// Given the differentials of a field in the x and y directions this outputs
/// the gradiant and angle fields. Not a particularly complex method, just 
/// provided for abstraction and cos I never get the signs right. The angle 
/// will be perpendicular to the edge, pointing to the higher valued side.
/// Angle is [-pi..pi].
EOS_FUNC void GradAngle(const svt::Field<real32> & dx,const svt::Field<real32> & dy,svt::Field<real32> & gradiant,svt::Field<real32> & angle);

/// Given an input field and a differentiation kernel this outputs the gradiant
/// and angle fields. (The differentiation kernel is assumed to be seperable into
/// vectors, as I've never come across a case otherwise.) The kernel given should
/// be dx, dy is assumed to be its transpose.
EOS_FUNC void GradAngle(const svt::Field<real32> & in,const KernelVect & kernel,svt::Field<real32> & gradiant,svt::Field<real32> & angle);

//------------------------------------------------------------------------------
 };
};
#endif
