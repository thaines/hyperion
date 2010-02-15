//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/mya/disparity.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
Disparity::Disparity(real32 mult,const svt::Field<real32> & disp)
:Ied(1,1)
{
 SetConst(0,mult);
 SetVariable(0,disp);
}

Disparity::Disparity(real32 mult,const svt::Field<real32> & disp,const svt::Field<bit> & valid)
:Ied(1,1)
{
 SetConst(0,mult);
 SetVariable(0,disp);
 SetValidity(valid);
}

Disparity::~Disparity()
{}

str::Token Disparity::Type(str::TokenTable & tt) const
{
 return tt("disp");
}

real32 Disparity::Distance(const math::Vector<real32> & a,const math::Vector<real32> & b) const
{
 bit ai = math::Equal(a[1],real32(0.0));
 bit bi = math::Equal(b[1],real32(0.0));
 if (ai||bi)
 {
  // Hairy situation - infinities have got involved...
   if (ai==bi) return 0.0;
          else return 1e100; // A big bad number.
 }
 else
 {
  return math::Abs((a[0]/a[1]) - (b[0]/b[1]));
 }
}

real32 Disparity::FitCost(const math::Vector<real32> & a,const Surface & surface,real32 x,real32 y) const
{
 math::Vect<2> b;
 surface.Get(x,y,b);
  
 bit ai = math::Equal(a[1],real32(0.0));
 bit bi = math::Equal(b[1],real32(0.0));
 if (ai||bi)
 {
  // Hairy situation - infinities have got involved...
   if (ai==bi) return 0.0;
          else return 1e10; // A big bad number.
 }
 else
 {
  return math::Abs((a[0]/a[1]) - (b[0]/b[1]));
 }
 return 1e10;
}

cstrconst Disparity::TypeString() const
{
 return "eos::mya::Disparity";
}

//------------------------------------------------------------------------------
 };
};
