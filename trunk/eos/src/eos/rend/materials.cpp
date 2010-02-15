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

#include "eos/rend/materials.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
void Lambertian::BDRF(const bs::Normal & inDir,const bs::Normal & outDir,
                      const Intersection & inter,bs::ColourRGB & out) const
{
 // First get the relevant albedo...
  if (albedo) albedo->GetColour(inter.coord,inDir,inter.norm,out);
         else out = bs::ColourRGB(1.0,1.0,1.0);

 // Use the cosine rule to produce the relevent multiplier, and multiply...
  real32 mult = inDir * inter.norm;
  if (mult>=0.0) out *= mult;
}

nat32 Lambertian::Parameters() const
{
 return 1;
}

TextureType Lambertian::ParaType(nat32 index) const
{
 return TextureColour;
}

cstrconst Lambertian::ParaName(nat32 index) const
{
 return "albedo";
}

void Lambertian::ParaSet(nat32 index,class Texture * rhs)
{
 if (index==0) albedo = rhs;
}

//------------------------------------------------------------------------------
 };
};
