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

#include "eos/rend/rerender.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
EOS_FUNC void LambertianNeedleRender(const svt::Field<real32> & albedo,
                                     const svt::Field<bs::Normal> & needle,
                                     const bs::Normal & toLight,
                                     svt::Field<real32> & out)
{
 for (nat32 y=0;y<albedo.Size(1);y++)
 {
  for (nat32 x=0;x<albedo.Size(0);x++)
  {
   out.Get(x,y) = math::Clamp<real32>(albedo.Get(x,y) * (toLight * needle.Get(x,y)),0.0,1.0);
  }
 }
}

//------------------------------------------------------------------------------
 };
};
