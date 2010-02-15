//-----------------------------------------------------------------------------
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

#include "eos/filter/grad_angle.h"

#include "eos/math/constants.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace filter
 {
//-----------------------------------------------------------------------------
EOS_FUNC void GradAngle(const svt::Field<real32> & dx,const svt::Field<real32> & dy,svt::Field<real32> & gradiant,svt::Field<real32> & angle)
{
 nat32 width = gradiant.Size(0);
 nat32 height = gradiant.Size(1);
 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++) 
  {
   real32 dxv = dx.Get(x,y);
   real32 dyv = dy.Get(x,y);
   gradiant.Get(x,y) = math::Sqrt(dxv*dxv + dyv*dyv);
   angle.Get(x,y) = math::InvTan2(dyv,dxv);
  }
 }
}

EOS_FUNC void GradAngle(const svt::Field<real32> & in,const KernelVect & kernel,svt::Field<real32> & gradiant,svt::Field<real32> & angle)
{
 // Create an intermediate object for the differentiated input...
  svt::Var * temp = new svt::Var(in);
   real32 ini = 0.0;
   temp->Add("dx",ini);
   temp->Add("dy",ini);
  temp->Commit(false);


 // Generate the differentiated input...
  svt::Field<real32> dx;
  svt::Field<real32> dy;
   temp->ByName("dx",dx);
   temp->ByName("dy",dy);

  kernel.Apply(in,dx,false);
  kernel.Apply(in,dy,true);


 // And produce the output...
  GradAngle(dx,dy,gradiant,angle);


 // Clean up...
  delete temp;
}

//------------------------------------------------------------------------------
 };
};
