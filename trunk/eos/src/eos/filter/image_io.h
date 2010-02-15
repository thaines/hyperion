#ifndef EOS_FILTER_IMAGE_IO_H
#define EOS_FILTER_IMAGE_IO_H
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


/// \namespace eos::filter
/// Provides lots of image filters, all using the SVT type. This module will have
/// its functionality exposed directly in aegle, but its useful to have these 
/// avaliable to all algorithm implimentations.

/// \file image_io.h
/// Not really filters as such, just two methods to load images into and out of
/// the svt type, for the conveniance of writting test applications.

#include "eos/types.h"#include "eos/svt/var.h"
#include "eos/bs/colours.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// Loads an image, returning a svt::Var that contains a single field 'rgb'
/// containning a ColourRGB, or null if it can't load it. The Var will be two 
/// dimensional, the width and then the height of the image. (You can change 
/// the field name if you really want to.)
EOS_FUNC svt::Var * LoadImageRGB(svt::Core & core,cstrconst filename,cstrconst fieldname = "rgb");

/// Loads an image, returning a svt::Var that contains a single field 'l'
/// containning a ColourL, or null if it can't load it. The Var will be two 
/// dimensional, the width and then the height of the image. (You can change 
/// the field name if you really want to.)
EOS_FUNC svt::Var * LoadImageL(svt::Core & core,cstrconst filename,cstrconst fieldname = "l");


/// Saves an image, will only overwrite an existing file if overwrite is set.
/// Expects the same style of Var as returned by LoadImageRGB.
/// Returns true on success, false on failure. (You can change 
/// the field name if you really want to.)
EOS_FUNC bit SaveImageRGB(svt::Var * image,cstrconst filename,bit overwrite = false,cstrconst fieldname = "rgb");

/// Saves an image, will only overwrite an existing file if overwrite is set.
/// Expects the same style of Var as returned by LoadImageL.
/// Returns true on success, false on failure. (You can change 
/// the field name if you really want to.)
EOS_FUNC bit SaveImageL(svt::Var * image,cstrconst filename,bit overwrite = false,cstrconst fieldname = "l");


/// Saves an image, will only overwrite an existing file if overwrite is set.
/// Takes as input a field to save to disk, instead of a Var.
/// Returns true on success, false on failure.
EOS_FUNC bit SaveImage(const svt::Field<bs::ColourRGB> & in,cstrconst filename,bit overwrite = false);


/// Saves an image, will only overwrite an existing file if overwrite is set.
/// Takes as input a field to save to disk, instead of a Var.
/// Returns true on success, false on failure.
EOS_FUNC bit SaveImage(const svt::Field<bs::ColourL> & in,cstrconst filename,bit overwrite = false);

//------------------------------------------------------------------------------
 };
};
#endif
