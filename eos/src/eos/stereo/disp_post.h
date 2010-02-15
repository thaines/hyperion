#ifndef EOS_STEREO_DISP_POST_H
#define EOS_STEREO_DISP_POST_H
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


/// \file disp_post.h
/// Provides post proccessing for disparity maps, to fill in missing values etc.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// Sets all invalid disparities to 0, so they are effectivly at infinity.
EOS_FUNC void FillDispInf(svt::Field<real32> & disp,const svt::Field<bit> & valid);

/// Sets all invalid disparities to there left neighbour, the fill in techneque
/// proposed with the middlebury test.
EOS_FUNC void FillDispLeft(svt::Field<real32> & disp,const svt::Field<bit> & valid);

//------------------------------------------------------------------------------
 };
};
#endif
