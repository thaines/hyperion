#ifndef EOS_SFS_ALBEDO_EST_H
#define EOS_SFS_ALBEDO_EST_H
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


/// \file albedo_est.h
/// Provides some simple albedo estimation stuff.

#include "eos/types.h"
#include "eos/bs/geo3d.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
/// Simple function for estimation the albedo - finds the mode of the
/// inputs. Mode is done to avoid outliers. Only uses pixels in the given range,
/// so as to avoid specularity effects and the extreme volatility of small values.
EOS_FUNC real32 AlbedoEstimate(const svt::Field<real32> & l,
                               const svt::Field<bs::Normal> & needle,
                               const bs::Normal & toLight,
                               real32 minL = 0.1,real32 maxL = 0.9);

//------------------------------------------------------------------------------
 };
};
#endif
