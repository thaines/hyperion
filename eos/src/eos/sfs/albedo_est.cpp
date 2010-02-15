//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

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

#include "eos/sfs/albedo_est.h"

#include "eos/ds/arrays.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
EOS_FUNC real32 AlbedoEstimate(const svt::Field<real32> & l,
                              const svt::Field<bs::Normal> & needle,
                              const bs::Normal & toLight,
                              real32 minL,real32 maxL)
{
 LogTime("eos::sfs::AlbedoEstimate");
 
 // Iterate the entire image and count how many suitable pixels exist.
  nat32 sampleCount = 0;
  for (nat32 y=0;y<l.Size(1);y++)
  {
   for (nat32 x=0;x<l.Size(0);x++)
   {
    real32 dot = needle.Get(x,y) * toLight;
    if ((l.Get(x,y)>minL)&&(l.Get(x,y)<maxL)&&(dot>0.0)) ++sampleCount;
   }
  }
  
 // Create array to store all the estimates in...
  ds::Array<real32> samples(sampleCount);
 
 // Fill in the array...
  nat32 index = 0;
  for (nat32 y=0;y<l.Size(1);y++)
  {
   for (nat32 x=0;x<l.Size(0);x++)
   {
    real32 dot = needle.Get(x,y) * toLight;
    if ((l.Get(x,y)>minL)&&(l.Get(x,y)<maxL)&&(dot>0.0))
    {
     samples[index] = l.Get(x,y)/dot;
     ++index;
    }
   }
  }
  log::Assert(index==sampleCount);

 // Sort...
  samples.SortNorm();
  
 // Return middle value...
  return samples[sampleCount/2];
}

//------------------------------------------------------------------------------
 };
};
