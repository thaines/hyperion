#ifndef EOS_SUR_SUBDIVIDE_H
#define EOS_SUR_SUBDIVIDE_H
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


/// \file subdivide.h
/// Subdivides a mesh.

#include "eos/types.h"

#include "eos/sur/mesh.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
/// Given an input mesh that must be triangulated this returns a new mesh that 
/// has had each triangle subdivided along each edge the given number of times,
/// with the centres filled in accordingly.
/// (divs==0 will result in the original input.)
/// The subdivisions are flat, so the mesh remains the same shape.
/// Remember to delete the returned mesh.
EOS_FUNC Mesh * Subdivide(Mesh & in,nat32 divs);

//------------------------------------------------------------------------------
 };
};
#endif
