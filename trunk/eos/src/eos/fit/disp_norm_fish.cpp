//------------------------------------------------------------------------------
// Copyright 2009 Tom Haines
#include "eos/fit/disp_norm_fish.h"

#include "eos/math/gaussian_mix.h"
#include "eos/sfs/sfs_bp.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
DispNormFish::DispNormFish()
:prob(0.1),minK(0.1),maxK(10.0)
{}

DispNormFish::~DispNormFish()
{}

void DispNormFish::Set(const svt::Field<real32> & d,const svt::Field<real32> & s)
{
 disp = d;
 sd = s;
}

void DispNormFish::SetMask(const svt::Field<bit> & m)
{
 mask = m;
}

void DispNormFish::SetPair(const cam::CameraPair & p)
{
 pair = p;
}

void DispNormFish::SetRegion(real32 p)
{
 prob = p;
}

void DispNormFish::SetRange(real32 nK,real32 xK)
{
 minK = nK;
 maxK = xK;
}
 
void DispNormFish::Run(time::Progress * prog)
{
 prog->Push();
 
 // Calculate the standard deviation multiplier for a bivariate gaussian needed
 // to obtain a region containing a probability mass of prob around the mean...
 // (Also resize the output array.)
  prog->Report(0,2+disp.Size(1));
  real32 mahDist = math::ChiSquareCulmInv(prob,2);
  LogDebug("test chi square " << math::ChiSquareCulmInv(0.95,10)); // Should output approx 18.3
  
  out.Resize(disp.Size(0),disp.Size(1));

 
 // Setup ready to calculate a Fisher distribution concentration given an
 // angular range that should contain prob around the concentration direction...
  prog->Report(1,2+disp.Size(1));
  prog->Push();
  sfs::FisherAngProb fap;
  fap.Make(prob,minK,maxK,1000,180,prog);
  prog->Pop();


 // Iterate every pixel and do the maths...
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   prog->Report(2+y,2+disp.Size(1));
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    // Set the concentration to zero where we don't have enough information...
     if (mask.Valid()&&(mask.Get(x,y)==false))
     {
      out.Get(x,y) = bs::Vert(0.0,0.0,0.0);
      continue;
     }
     
     bit safeXN = (x>0) && (!mask.Valid() || mask.Get(x-1,y));
     bit safeXP = (x+1<disp.Size(0)) && (!mask.Valid() || mask.Get(x+1,y));
     bit safeYN = (y>0) && (!mask.Valid() || mask.Get(x,y-1));
     bit safeYP = (y+1<disp.Size(1)) && (!mask.Valid() || mask.Get(x,y+1));
     
     if ((!safeXN && !safeXP) || (!safeYN && !safeYP))
     {
      out.Get(x,y) = bs::Vert(0.0,0.0,0.0);
      continue;
     }


    // First calculate the bivariate Gaussian on disparity difference from the 
    // input disparity map, handling the boundary cases. Note that this 
    // distribution might not have a positive definite covariance matrix...
    
    
    // Get the 5 disparity difference values - the mean value and 4 more,
    // offsetted from the mean in both directions along each of the major and 
    // minor axis of the ellipsoid so they are at the edge of the region 
    // containing the given probability...
    
    
    // Convert each disparity difference into a surface orientaiton direction,
    // using the pixels disparity to ground the differences...
    
    
    // Construct a Fisher distribution from the angles - direction of the centre
    // disparity difference, concentration to match the probability consumed in
    // the angular range...
    // (We take the maximum angle between the edge surface normals and centre
    // surface normal, to take a pesimistic view of certainty.)
   
   
   }
  }

 prog->Pop();
}

void DispNormFish::Get(svt::Field<bs::Vert> & fish)
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++) fish.Get(x,y) = out.Get(x,y);
 }
}

void DispNormFish::Get(svt::Field<math::Fisher> & fish)
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   for (nat32 i=0;i<3;i++) fish.Get(x,y)[i] = out.Get(x,y)[i];
  }
 }
}

//------------------------------------------------------------------------------
 };
};
