#ifndef EOS_FILTER_GRAD_BILATERAL_H
#define EOS_FILTER_GRAD_BILATERAL_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

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


/// \file grad_bilateral.h
/// Contains functions for working out image gradiants using bilateral filters.

#include "eos/types.h"
#include "eos/bs/colours.h"
#include "eos/svt/field.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This calculates the gradient of an image using the differential of a 
/// Gaussian of the spatial component of the image in two directions (With a 
/// normal Gaussian perpendicular) and a Gaussian in the domain of the image to
/// weight it.
EOS_FUNC void GradBilatGauss(const svt::Field<bs::ColourLuv> & in,svt::Field<real32> & dx,svt::Field<real32> & dy,
                             real32 spatialSd,real32 domainSd,real32 winSdMult = 2.0,
                             time::Progress * prog = null<time::Progress*>());

//------------------------------------------------------------------------------
 };
};
#endif
