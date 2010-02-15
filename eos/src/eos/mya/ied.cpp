//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
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
