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

#include "eos/stereo/disp_post.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
EOS_FUNC void FillDispInf(svt::Field<real32> & disp,const svt::Field<bit> & valid)
{
 for (nat32 y=0;y<disp.Size(1);y++)
 {
  for (nat32 x=0;x<disp.Size(0);x++)
  {
   if (!valid.Get(x,y))
   {
    disp.Get(x,y) = 0.0;
   }
  }
 }
}

EOS_FUNC void FillDispLeft(svt::Field<real32> & disp,const svt::Field<bit> & valid)
{
 for (nat32 y=0;y<disp.Size(1);y++)
 {
  if (!valid.Get(0,y)) disp.Get(0,y) = 0.0;

  for (nat32 x=1;x<disp.Size(0);x++)
  {
   if (!valid.Get(x,y))
   {
    disp.Get(x,y) = disp.Get(x-1,y);
   }
  }
 }
}

//------------------------------------------------------------------------------
 };
};
