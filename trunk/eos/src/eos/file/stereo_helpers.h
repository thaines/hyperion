#ifndef EOS_FILE_STEREO_HELPERS_H
#define EOS_FILE_STEREO_HELPERS_H
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


/// \file stereo_helpers.h
/// Provides a set of helper methods for saving the output of stereo algorithms,
/// just a load of tedius stuff that I do a lot when writting executables to 
/// impliment algorithms, and hence don't want to duplicate.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/bs/geo3d.h"
#include "eos/bs/colours.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
/// Saves a map of reals, simply absolutes them and scales so 0 is black and the
/// highest value is the strongest white.
/// Returns true on success.
EOS_FUNC bit SaveReal(const svt::Field<real32> & map,cstrconst fn);

/// Saves a map of reals, scales and offsets so the lowest is baclk and the highest white.
/// Returns true on success.
EOS_FUNC bit SaveSignedReal(const svt::Field<real32> & map,cstrconst fn);

/// Saves a segmentation as a colour map. You provide the segmentation and an
/// extensionless filename, it outputs a .bmp.
/// Returns true on success.
EOS_FUNC bit SaveSeg(const svt::Field<nat32> & seg,cstrconst fn);

/// Saves a disparity map, you provide the map and an extensionless filename,
/// it then saves two versions - a .bmp scaled to cover the entire range, and a 
/// .obf that can be loaded into Cyclops.
/// You can additionally provide a mask to indicate validity of disparity.
/// Returns true on success.
EOS_FUNC bit SaveDisparity(const svt::Field<real32> & disp,cstrconst fn,const svt::Field<bit> * mask = 0);

/// Generates and then saves a warp image, you provide a left image, a right 
/// image (For dimension extraction, can be the same as the left if you don't
/// care so much.) and a disparity map plus an extensionless filename.
/// It then saves a single .bmp with the filename.
/// You can additionally provide a mask to indicate validity of disparity.
/// Returns true on success.
EOS_FUNC bit SaveWarp(const svt::Field<bs::ColourRGB> & left,const svt::Field<bs::ColourRGB> & right,
                      const svt::Field<real32> & disp,cstrconst fn,const svt::Field<bit> * mask = 0);

/// Saves a needle map, you provide a needle map and an extensionless filename. 
/// It always outputs a colour rendering of the normals as a 
/// .bmp.
/// If ext is set true it also outputs a wavefront .obj file of little facets
/// to represent the normals, oriented so there normals match the given normal map.
/// Returns true on success.
EOS_FUNC bit SaveNeedle(const svt::Field<bs::Normal> & needle,cstrconst fn,bit ext = false);

/// Saves a gradient field, uses red and green for the gradiant vectors, but
/// also encodes magnitude in blue. The red (x) and green (y) cahnnels are
/// scaled identically to fit in range, with half way being 0.
/// Returns true on success. Always overwrites existing files.
EOS_FUNC bit SaveGradient(const svt::Field<real32> & dx,const svt::Field<real32> & dy,cstrconst fn);

//------------------------------------------------------------------------------
 };
};
#endif
