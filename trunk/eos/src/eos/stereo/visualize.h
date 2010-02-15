#ifndef EOS_STEREO_VISUALIZE_H
#define EOS_STEREO_VISUALIZE_H
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


/// \file visualize.h
/// Provides the ability to render disparities out to images.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/bs/colours.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// Renders out a stereo disparity map, middlebury style.
/// Renders black to white, multipliying the disparities by mult to get the 
/// [0..1] range required.
/// If mult is 0.0 it calculates a suitable mult to utilise the entire range.
EOS_FUNC void RenderDisp(const svt::Field<real32> & disp,svt::Field<bs::ColourL> & out,real32 mult = 0.0);

/// This is given a 3D field representing similarity or dis-similarity map and
/// a row to render, it then returns a Var containing a b&w image of the row 
/// normalised to fill the greyscale range, showing the values across the 
/// disparity. It also fills in the meta fields min and max with the actual
/// minimum and maximum values found pre-normalisation.
EOS_FUNC svt::Var * RenderDsiRow(const svt::Field<real32> & dsi,nat32 row);


//------------------------------------------------------------------------------
 };
};
#endif
