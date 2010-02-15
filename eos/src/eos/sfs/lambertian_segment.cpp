//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/sfs/lambertian_segment.h"

#include "eos/math/gaussian_mix.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
LamSeg::LamSeg()
:adjProb(0.1),matchMin(0.2),iters(10)
{}

LamSeg::~LamSeg()
{}

void LamSeg::SetMaps(const svt::Field<real32> & i,const svt::Field<bs::Normal> & n)
{
 irr = i;
 needle = n;

 log::Assert((irr.Size(0)==needle.Size(0))&&(irr.Size(1)==needle.Size(1)));

 sd.Resize(irr.Size(0),irr.Size(1));
 for (nat32 y=0;y<sd.Height();y++)
 {
  for (nat32 x=0;x<sd.Width();x++) sd.Get(x,y) = 1.0;
 }
}

void LamSeg::SetNoise(nat32 x,nat32 y,real32 s)
{
 sd.Get(x,y) = s;
}

void LamSeg::Set(real32 ap,real32 mp,nat32 i)
{
 adjProb = ap;
 matchMin = mp;
 iters = i;
}

void LamSeg::Set(const svt::Field<bs::ColourLuv> & i,real32 min,real32 max,real32 t,real32 h)
{
 image = i;
 minAdjProb = min;
 maxAdjProb = max;
 threshold = t;
 halflife = h;
}

void LamSeg::SetModels(nat32 count)
{
 model.Size(count);
}

void LamSeg::SetModel(nat32 ind,const bs::Normal & mod)
{
 model[ind] = mod;
}

void LamSeg::Run(time::Progress * prog)
{
 LogDebug("eos::sfs::LamSeg::Run");
 prog->Push();
 
 // Create and fill in the ModelSeg...
  prog->Report(0,3);
  inf::ModelSeg ms;
  ms.SetSize(irr.Size(0),irr.Size(1),model.Size());
  real32 diffCost = -math::Ln(adjProb);
  real32 costCap = -math::Ln(matchMin);
  ms.SetParas(image.Valid()?1.0:diffCost,costCap,iters);
  
  prog->Push();
  real32 adjCostBase = -math::Ln(maxAdjProb);
  real32 adjCostMult = -math::Ln(minAdjProb) - adjCostBase;
    
  for (nat32 y=0;y<irr.Size(1);y++)
  {
   prog->Report(y,irr.Size(1));
   for (nat32 x=0;x<irr.Size(0);x++)
   {
    real32 divSd = 0.5/math::Sqr(sd.Get(x,y));
    for (nat32 m=0;m<model.Size();m++)
    {
     real32 irrDelta = irr.Get(x,y) - (model[m] * needle.Get(x,y));
     real32 cost = math::Sqr(irrDelta) * divSd; 
     ms.AddCost(x,y,m,cost);
    }
    
    if (image.Valid())
    {
     real32 mx = 0.0;
     if (x!=irr.Size(0)-1)
     {
      real32 dist = math::Sqrt(math::Sqr(image.Get(x,y).u - image.Get(x+1,y).u) + 
                               math::Sqr(image.Get(x,y).v - image.Get(x+1,y).v));
      mx = adjCostBase + adjCostMult * math::SigmoidCutoff(dist,threshold,halflife);
     }
     
     real32 my = 0.0;
     if (y!=irr.Size(1)-1)
     {
      real32 dist = math::Sqrt(math::Sqr(image.Get(x,y).u - image.Get(x,y+1).u) + 
                               math::Sqr(image.Get(x,y).v - image.Get(x,y+1).v));
      my = adjCostBase + adjCostMult * math::SigmoidCutoff(dist,threshold,halflife);
     }
     ms.SetMult(x,y,mx,my);    
    }
   }
  }
  prog->Pop();



 // Run...
  prog->Report(1,3);
  ms.Run(prog);



 // Extract the results...
  prog->Report(2,3);
  pixToMod.Resize(irr.Size(0),irr.Size(1));
  for (nat32 y=0;y<pixToMod.Height();y++)
  {
   for (nat32 x=0;x<pixToMod.Width();x++)
   {
    pixToMod.Get(x,y) = ms.Model(x,y);
   }
  }

 prog->Pop();
}

nat32 LamSeg::Models() const
{
 return model.Size();
}

const bs::Normal & LamSeg::Model(nat32 ind) const
{
 return model[ind];
}

nat32 LamSeg::Pixel(nat32 x,nat32 y) const
{
 return pixToMod.Get(x,y);
}

void LamSeg::GetModel(svt::Field<nat32> & out) const
{
 for (nat32 y=0;y<pixToMod.Height();y++)
 {
  for (nat32 x=0;x<pixToMod.Width();x++)
  {
   out.Get(x,y) = pixToMod.Get(x,y);
  }
 }
}

void LamSeg::GetModel(svt::Field<bs::Normal> & out) const
{
 for (nat32 y=0;y<pixToMod.Height();y++)
 {
  for (nat32 x=0;x<pixToMod.Width();x++)
  {
   out.Get(x,y) = model[pixToMod.Get(x,y)];
  }
 }
}

void LamSeg::GetSplit(svt::Field<real32> & irr,svt::Field<bs::Normal> & dir)
{
 for (nat32 y=0;y<pixToMod.Height();y++)
 {
  for (nat32 x=0;x<pixToMod.Width();x++)
  {
   dir.Get(x,y) = model[pixToMod.Get(x,y)];
   
   irr.Get(x,y) = dir.Get(x,y).Length();
   dir.Get(x,y) /= irr.Get(x,y);
  }
 }
}

//------------------------------------------------------------------------------
 };
};
