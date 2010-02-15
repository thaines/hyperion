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

#include "eos/rend/post_processors.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
void PostNoise::Set(real32 b,real32 sd,real32 sdm,bit pr)
{
 bias = b;
 sd1 = sd;
 sdm2 = sdm;
 perRay = pr;
}

void PostNoise::Apply(RayImage & ri,time::Progress * prog) const
{
 prog->Push();
  if (perRay)
  {
   for (nat32 y=0;y<ri.Height();y++)
   {
    prog->Report(y,ri.Height());
    for (nat32 x=0;x<ri.Width();x++)
    {
     for (nat32 i=0;i<ri.Rays(x,y);i++)
     {
      bs::ColourRGB & targ = ri.Ray(x,y,i).irradiance;
      targ.r = math::Max<real32>(0.0,targ.r + bias + rand.Gaussian(sd1) + rand.Gaussian(targ.r*sdm2));
      targ.g = math::Max<real32>(0.0,targ.g + bias + rand.Gaussian(sd1) + rand.Gaussian(targ.g*sdm2));
      targ.b = math::Max<real32>(0.0,targ.b + bias + rand.Gaussian(sd1) + rand.Gaussian(targ.b*sdm2));
     }
    }
   }
  }
  else
  {
   for (nat32 y=0;y<ri.Height();y++)
   {
    prog->Report(y,ri.Height());
    for (nat32 x=0;x<ri.Width();x++)
    {
     bs::ColourRGB average = bs::ColourRGB(0.0,0.0,0.0);
      for (nat32 i=0;i<ri.Rays(x,y);i++)
      {
       bs::ColourRGB temp = ri.Ray(x,y,i).irradiance;
       temp *= ri.Ray(x,y,i).weight;
       average += temp;
      }
     average /= ri.RaysWeightSum(x,y,i);
     
     bs::ColourRGB noise;
      noise.r = bias + rand.Gaussian(sd1) + rand.Gaussian(average.r*sdm2);
      noise.g = bias + rand.Gaussian(sd1) + rand.Gaussian(average.g*sdm2);
      noise.b = bias + rand.Gaussian(sd1) + rand.Gaussian(average.b*sdm2);

     for (nat32 i=0;i<ri.Rays(x,y);i++)
     {
      bs::ColourRGB & targ = ri.Ray(x,y,i).irradiance;
       targ += noise;
       targ.r = math::Max(targ.r,real32(0.0));
       targ.g = math::Max(targ.g,real32(0.0));
       targ.b = math::Max(targ.b,real32(0.0));
     }
    }
   }  
  }
 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
