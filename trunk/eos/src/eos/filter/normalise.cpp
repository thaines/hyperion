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

#include "eos/filter/normalise.h"

#include "eos/math/functions.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC void Normalise(svt::Field<real32> & f)
{
 // First pass to find the minimum and maximum...
  real32 min = f.Get(0,0);
  real32 max = f.Get(0,0);
  for (nat32 y=0;y<f.Size(1);y++)
  {
   for (nat32 x=0;x<f.Size(0);x++)
   {
    min = math::Min(min,f.Get(x,y));
    max = math::Max(max,f.Get(x,y));
   }	  
  }
 
 // Second pass to adjust...
  for (nat32 y=0;y<f.Size(1);y++)
  {
   for (nat32 x=0;x<f.Size(0);x++)
   {
    f.Get(x,y) = (f.Get(x,y)-min)/(max-min);
   }	  
  }
}

//------------------------------------------------------------------------------
 };
};
