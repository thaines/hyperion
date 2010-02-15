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

#include "eos/inf/fig_variables.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
Grid2D::Grid2D(nat32 labels,nat32 width,nat32 height)
:paras(3)
{
 paras[0] = labels;
 paras[1] = width;
 paras[2] = height;
}

Grid2D::~Grid2D()
{}

Grid2D * Grid2D::Clone() const
{
 return new Grid2D(paras[0],paras[1],paras[2]);
}

const math::Vector<nat32> & Grid2D::Paras() const
{
 return paras;
}

nat32 Grid2D::Labels() const
{
 return paras[0];
}

nat32 Grid2D::Vars(nat32 level) const
{
 nat32 width  = paras[1];
 nat32 height = paras[2];

 width >>= level;
 height >>= level;

 if (width==0) width = 1;
 if (height==0) height = 1;

 return width*height;
}

nat32 Grid2D::MaxLevel() const
{
 return math::Max(math::TopBit(paras[1]),math::TopBit(paras[2])) - 1;
}

void Grid2D::Transfer(nat32 level,VariableTransfer & vt) const
{
 // Calculate the levels dimensions...
  nat32 width  = paras[1];
  nat32 height = paras[2];

  width >>= level;
  height >>= level;

  if (width==0) width = 1;
  if (height==0) height = 1;
  
 // Calculate the width of level+1...
  nat32 widthP1 = paras[1];
  widthP1 >>= level+1;
  if (widthP1==0) widthP1 = 1;
 
 // Shave any last bits from the width/height...
  nat32 shaveWidth  = width & (~1);  
  nat32 shaveHeight = height & (~1);
 
 // Iterate all members and for each one copy from the relevent variable...
  for (nat32 y=0;y<shaveHeight;y++)
  {
   for (nat32 x=0;x<shaveWidth;x++)
   {
    vt.Transfer((y>>1)*widthP1+(x>>1),y*width+x);
   }
  }
  
 // Do the edge cases, which do not map correctly, setting them all to flatline...
  if (shaveWidth!=width)
  {
   for (nat32 i=0;i<shaveHeight;i++)
   {  
    vt.Flatline(i*width+shaveWidth);
   }
  }
  if (shaveHeight!=height)
  {
   for (nat32 i=0;i<shaveWidth;i++)
   { 
    vt.Flatline(shaveHeight*width+i);
   }
  }  
  if ((shaveWidth!=width)&&(shaveHeight!=height))
  {
   vt.Flatline(shaveHeight*width+shaveWidth);    
  }
}

cstrconst Grid2D::TypeString() const
{
 return "eos::inf::Grid2D";
}

//------------------------------------------------------------------------------
 };
};
