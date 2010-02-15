#ifndef EOS_REND_VISUALISE_H
#define EOS_REND_VISUALISE_H
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


/// \file visualise.h
/// Provides simple functions for visualising certain types of data as images.

#include "eos/types.h"
#include "eos/bs/colours.h"
#include "eos/bs/geo3d.h"
#include "eos/svt/field.h"
#include "eos/file/wavefront.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// Converts a needle map into a colour image. Any normals pointing away from the 
/// viewer are set to black, for normals pointing towards the viewer red indicates
/// the x axis, green the y axis and blue the z axis. A normal pointing full left 
/// has no red, a normal pointing full right has 100% red. A normal pointing full
/// south has no green, a normal pointing full up has 100% green. A normal with 
/// z==0 has no blue, a normal pointing directly at the viewer has 100% blue.
/// rev allows you to reverse the direction of all needles, so you can save out
/// two images containning all info when normals are no behaving.
EOS_FUNC void VisNeedleMap(const svt::Field<bs::Normal> & in,svt::Field<bs::ColourRGB> & out,bit rev = false);

/// Similar to VisNeedleMap, except this creates a 3D model of faces, on a plane,
/// where each face is rotated into the direction of the corresponing normal.
/// This visualisation works very well in blender, where you can swicth on normal
/// viewing and set there length.
/// This is addative to the output, anything in there at the start is preserved.
EOS_FUNC void NeedleMapToModel(const svt::Field<bs::Normal> & in,file::Wavefront & out);

//------------------------------------------------------------------------------
 };
};
#endif
