#ifndef EOS_FILTER_NORMALISE_H
#define EOS_FILTER_NORMALISE_H
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


/// \file normalise.h
/// Normalises an image.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/svt/var.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// Given a field of reals this normalises them to the 0..1 range, useful for 
/// visualisation.
EOS_FUNC void Normalise(svt::Field<real32> & f);

//------------------------------------------------------------------------------
 };
};
#endif
