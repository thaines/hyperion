#ifndef EOS_FILTER_COLOUR_MATCHING_H
#define EOS_FILTER_COLOUR_MATCHING_H
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


/// \file colour_matching.h
/// Provides algorithms for making colours look the same, primarily for removing
/// colour differences between images.

#include "eos/types.h"

#include "eos/svt/field.h"
#include "eos/bs/colours.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// Given two images this matches them by applying nothing more than a
/// multiplicative factor to each colour channel.
/// Quite a complicated method of deciding the multiplier is used however.
/// As the multiplier for each colour field can be applied to either image it
/// is applied to the image which it will make darker, so as to not stray
/// outside range.
/// a and b are both inputs and outputs, depth is how many subdivision levels for
/// it to use for its histogram, i.e. 2^depth (adaptive) buckets shall exist.
/// weight indicates how much extra importance it gives to higher/lower values
/// in the colour range, i.e. 1.0 gives equal balance to matching across entire
/// colour range, greater than 1 biases towards matching higher values as you
/// usually want.
EOS_FUNC void SimpleImageMatch(svt::Field<bs::ColourRGB> & a,svt::Field<bs::ColourRGB> & b,nat32 depth = 4,real32 weight = 2.0);

//------------------------------------------------------------------------------
 };
};
#endif
