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

#include "eos/mya/needles.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
Needles::Needles(const svt::Field<bs::Normal> & dir)
:Ied(0,3)
{
 svt::Field<real32> n;
 dir.SubField(0*sizeof(real32),n); SetVariable(0,n);
 dir.SubField(1*sizeof(real32),n); SetVariable(1,n);
 dir.SubField(2*sizeof(real32),n); SetVariable(2,n);
}

Needles::Needles(const svt::Field<bs::Normal> & dir,const svt::Field<bit> & valid)
:Ied(0,3)
{
 svt::Field<real32> n;
 dir.SubField(0*sizeof(real32),n); SetVariable(0,n);
 dir.SubField(1*sizeof(real32),n); SetVariable(1,n);
 dir.SubField(2*sizeof(real32),n); SetVariable(2,n);
 SetValidity(valid);
}

Needles::~Needles()
{}

str::Token Needles::Type(str::TokenTable & tt) const
{
 return tt("needle");
}

real32 Needles::Distance(const math::Vector<real32> & a,const math::Vector<real32> & b) const
{
 return math::InvCos(a * b);
}

real32 Needles::FitCost(const math::Vector<real32> & a,const Surface & surface,real32 x,real32 y) const
{ 
 math::Vect<2> dx;
 math::Vect<2> dy; 
 surface.GetDelta(x,y,dx,dy);
 
 if (math::Equal(dx[1],real32(0.0))||math::Equal(dy[1],real32(0.0))) return math::pi*0.5;
 bs::Normal norm;
  norm[0] = dx[0]/dx[1];
  norm[1] = dy[0]/dy[1];
  norm[2] = 1.0;
 norm.Normalise();
 
 return math::InvCos(a * norm);
}

cstrconst Needles::TypeString() const
{
 return "eos::mya::Needles";
}

//------------------------------------------------------------------------------
 };
};
