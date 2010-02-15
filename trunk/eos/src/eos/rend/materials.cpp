//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
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
