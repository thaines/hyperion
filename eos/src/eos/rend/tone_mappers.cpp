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

#include "eos/rend/tone_mappers.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
void ToneScaler::Apply(const RayImage & ri,svt::Field<bs::ColourRGB> & out,time::Progress * prog) const
{
 prog->Push();


  // First null the output...
   prog->Report(0,4);
   for (nat32 y=0;y<ri.Height();y++)
   {
    for (nat32 x=0;x<ri.Width();x++) out.Get(x,y) = bs::ColourRGB(0.0,0.0,0.0);
   }


  // Now sumate all the values from the ray image into the output, weighting 
  // them according to ray counts and weights...
   prog->Report(1,4);
   for (nat32 y=0;y<ri.Height();y++)
   {
    for (nat32 x=0;x<ri.Width();x++)
    {
     real32 mult = 1.0/ri.RaysWeightSum(x,y);
     for (nat32 i=0;i<ri.Rays(x,y);i++)
     {
      bs::ColourRGB col = ri.Ray(x,y,i).irradiance;
      col *= ri.Ray(x,y,i).weight * mult;
      out.Get(x,y) += col;
     }
    }
   }


  // Now, depending on mode, calculate the multiplier...
   prog->Report(2,4);
   real32 mult;
   if (meanM)
   {
    real32 mean = 0.0;
    for (nat32 y=0;y<ri.Height();y++)
    {
     for (nat32 x=0;x<ri.Width();x++) mean += (out.Get(x,y).r+out.Get(x,y).g+out.Get(x,y).b)/3.0;
    }
    mean /= real32(ri.Width()*ri.Height());
    mult = 0.5/mean;
   }
   else
   {
    real32 max = 0.0;
    for (nat32 y=0;y<ri.Height();y++)
    {
     for (nat32 x=0;x<ri.Width();x++) max = math::Max(max,out.Get(x,y).r,out.Get(x,y).g,out.Get(x,y).b);
    }
    mult = 1.0/max;
   }


  // And finally apply the multiplier...
   prog->Report(3,4);
   for (nat32 y=0;y<ri.Height();y++)
   {
    for (nat32 x=0;x<ri.Width();x++) out.Get(x,y) *= mult;
   }
   

 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
