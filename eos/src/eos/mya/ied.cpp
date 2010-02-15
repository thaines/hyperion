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

#include "eos/mya/ied.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
Ied::Ied(nat32 cf,nat32 vf)
:constFeats(cf),variableFeats(vf)
{}

Ied::~Ied()
{}

nat32 Ied::Length() const
{
 return constFeats.Size() + variableFeats.Size();
}

nat32 Ied::Width() const
{
 return variableFeats[0].Size(0);
}

nat32 Ied::Height() const
{
 return variableFeats[0].Size(1);
}

bit Ied::Valid(nat32 x,nat32 y) const
{
 if (valid.Valid()&&(valid.Get(x,y)==false)) return false;
                                        else return true;
}

bit Ied::Feature(nat32 x,nat32 y,math::Vector<real32> & out) const
{
 if (valid.Valid()&&(valid.Get(x,y)==false)) return false;

  nat32 ovi = 0;
  for (nat32 i=0;i<constFeats.Size();i++)
  {
   out[ovi] = constFeats[i];
   ++ovi;
  }
  for (nat32 i=0;i<variableFeats.Size();i++)
  {
   out[ovi] = variableFeats[i].Get(x,y);
   ++ovi;
  }
 
 return true;
}

void Ied::SetValidity(const svt::Field<bit> & v)
{
 valid = v;
}

void Ied::SetConst(nat32 index,real32 value)
{
 constFeats[index] = value;
}

void Ied::SetVariable(nat32 index,const svt::Field<real32> & value)
{
 variableFeats[index] = value;
}

//------------------------------------------------------------------------------
 };
};
