//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/fit/disp_norm.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
DispNorm::DispNorm()
:dsc(null<stereo::DSC*>()),dscMult(1.0),range(20),
minSd(0.0),maxSd(10.0),maxIters(1000)
{}

DispNorm::~DispNorm()
{}

void DispNorm::Set(const svt::Field<real32> & d,const stereo::DSC & ds,real32 dscM)
{
 disp = d;
 dsc = &ds;
 dscMult = dscM;
}

void DispNorm::SetMask(const svt::Field<bit> & m)
{
 mask = m;
}

void DispNorm::SetRange(nat32 r)
{
 range = r;
}

void DispNorm::SetClamp(real32 minS,real32 maxS)
{
 minSd = minS;
 maxSd = maxS;
}

void DispNorm::SetMaxIters(nat32 mi)
{
 maxIters = mi;
}

void DispNorm::Run(time::Progress * prog)
{
 prog->Push();

 // ************************************************************

 prog->Pop();
}

void DispNorm::Get(svt::Field<real32> & sd)
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++) sd.Get(x,y) = out.Get(x,y);
 }
}

real32 DispNorm::GetSd(nat32 x,nat32 y)
{
 return out.Get(x,y);
}

//------------------------------------------------------------------------------
 };
};
