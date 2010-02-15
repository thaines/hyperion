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

#include "eos/rend/samplers.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
void GridAA::Render(Job & job,time::Progress * prog)
{
 prog->Push();
 
 nat32 height = job.Camera().Height();
 nat32 width = job.Camera().Width();
 bs::Vert start;
 bit constantStart = job.Camera().ConstantStart(start);
 
 ds::List<Renderable*> inside;
 TaggedRay ray;


 if (constantStart) job.DB().Inside(start,inside);

 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   prog->Report(y*width+x,width*height);
   
   for (nat32 v=0;v<dimSamps;v++)
   {
    for (nat32 u=0;u<dimSamps;u++)
    {
     real32 xp = real32(x) + real32(u)/real32(dimSamps+1);
     real32 yp = real32(y) + real32(v)/real32(dimSamps+1);
     job.Camera().ViewRay(xp,yp,ray);
     ray.weight = 1.0;
     
     if (!constantStart)
     {
      inside.Reset();
      job.DB().Inside(ray.s,inside);
     }
     
     job.Rend().Cast(ray,job.DB(),inside,job.LightList(),job.BG());
     
     job.RI().AddRay(x,y,ray);
    }
   }

  }
 }
 
 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
