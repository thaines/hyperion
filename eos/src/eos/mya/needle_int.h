#ifndef EOS_MYA_NEEDLE_INT_H
#define EOS_MYA_NEEDLE_INT_H
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


/// \file needle_int.h
/// Needle map integration methods.

#include "eos/svt/field.h"
#include "eos/bs/geo3d.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
/// This integrates a needle map to generate a depth map, uses a path selection
/// scheme that chooses the flatest possible path. Uses a forest, which it merges
/// into a single tree by selecting the lowest delta path between two unconnected
/// trees each time.
EOS_FUNC void IntegrateNeedle(const svt::Field<bs::Normal> & needle,const svt::Field<bit> & mask,svt::Field<real32> & depth);

//------------------------------------------------------------------------------
/// A helper function, given a needle map this saves a Wavefront .obj 3D model
/// to a given filename, overwritting on request. You also provide a sampling
/// resolution for the model, so sampling every pixel is optional.
/// \param needle The needle map to be saved.
/// \param mask The mask associated with the needle map.
/// \param freq The frequency of sampling for pixels, set to 1 to sample every one, 5 to sample every 5th pixel etc.
/// \param fn Filename to save the .obj file to.
/// \param overwrite true to replace any existing file, false to fail if a file allready exists.
/// \returns true on success, false on failure.
EOS_FUNC bit SaveNeedleModel(const svt::Field<bs::Normal> & needle,const svt::Field<bit> & mask,nat32 freq,cstrconst fn,bit overwrite = false);

//------------------------------------------------------------------------------
 };
};
#endif
