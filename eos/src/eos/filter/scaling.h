#ifndef EOS_FILTER_SCALING_H
#define EOS_FILTER_SCALING_H
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


/// \file scaling.h
/// Scales stuff. Blah.

#include "eos/types.h"
#include "eos/bs/colours.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// Scales a field of reals, you provide a scaler factor and it iterates the
/// entire output field, pixels outside the input are extended.
/// Uses a pixel wide hat filter precisly weighted, so will pixalise as you zoom.
EOS_FUNC void ScaleExtend(const svt::Field<real32> & in,svt::Field<real32> & out,real32 scaler);

/// Same as the basic ScaleExtend, but with masking support.
/// Doesn't extend, obviously. Masked regions in the output are zeroed.
EOS_FUNC void ScaleExtend(const svt::Field<real32> & in,const svt::Field<bit> & inMask,
                          svt::Field<real32> & out,svt::Field<bit> & outMask,
                          real32 scaler);

/// Same as the real32 version, except for bs::ColourRGB.
EOS_FUNC void ScaleExtend(const svt::Field<bs::ColourRGB> & in,svt::Field<bs::ColourRGB> & out,real32 scaler);

/// Same as the real32 version, except for bs::ColourRGB.
EOS_FUNC void ScaleExtend(const svt::Field<bs::ColourRGB> & in,const svt::Field<bit> & inMask,
                          svt::Field<bs::ColourRGB> & out,svt::Field<bit> & outMask,
                          real32 scaler);

//------------------------------------------------------------------------------
 };
};
#endif
