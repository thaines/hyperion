#ifndef EOS_REND_RERENDER_H
#define EOS_REND_RERENDER_H
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


/// \file rerender.h
/// Provides tools for re-rendering images based on changing global parameters,
/// such as light source direction for instance.

#include "eos/types.h"
#include "eos/bs/geo3d.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// Given an albedo and a normal for each point in an image this assumes basic
/// lambertion lighting, from an infinite light source that dosn't cast shadows,
/// and re-renders the image accordingly.
/// \param albedo Albedo for each pixel in the image.
/// \param needle Surface normal for each pixel in the image.
/// \param toLight Normal pointing towards light source at infinity, its length is the strength of he light. (Any longer than 1 and the image will probably suffer from saturation.)
/// \param out The output brightness map.
EOS_FUNC void LambertianNeedleRender(const svt::Field<real32> & albedo,
                                     const svt::Field<bs::Normal> & needle,
                                     const bs::Normal & toLight,
                                     svt::Field<real32> & out);

//------------------------------------------------------------------------------
 };
};
#endif
