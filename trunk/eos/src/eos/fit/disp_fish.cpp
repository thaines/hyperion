//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/fit/disp_fish.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
DispFish::DispFish()
:range(3)
{}

DispFish::~DispFish()
{}

void DispFish::Set(const svt::Field<real32> & di,const stereo::DSC & ds)
{
 disp = di;
 dsc = &ds;
}

void DispFish::SetDtD(const real64 & a,const real64 & b)
{
 convA = a;
 convB = b;
}

void DispFish::SetRange(nat32 r)
{
 range = r;
}

void DispFish::Run(time::Progress * prog)
{
 prog->Push();
 
 // First create depth/cost pairs for all pixels covering the range...
 
 
 // Create the buffer to store R-contribution/weight pairs...
 
 
 // Iterate the pixels and calculate the distribution for each - this consists 
 // of an easy direction and hard concentration...
 
 
 prog->Pop();
}

void DispFish::Get(svt::Field<bs::Vert> & fish)
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++) fish.Get(x,y) = out.Get(x,y);
 }
}

bs::Vert & DispFish::GetFish(nat32 x,nat32 y)
{
 return out.Get(x,y);
}

//------------------------------------------------------------------------------
 };
};
