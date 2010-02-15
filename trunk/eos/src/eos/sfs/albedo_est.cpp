//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/sfs/albedo_est.h"

#include "eos/ds/arrays.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
EOS_FUNC real32 AlbedoEstimate(const svt::Field<real32> & l,
                              const svt::Field<bs::Normal> & needle,
                              const bs::Normal & toLight,
                              real32 minL,real32 maxL)
{
 LogTime("eos::sfs::AlbedoEstimate");
 
 // Iterate the entire image and count how many suitable pixels exist.
  nat32 sampleCount = 0;
  for (nat32 y=0;y<l.Size(1);y++)
  {
   for (nat32 x=0;x<l.Size(0);x++)
   {
    real32 dot = needle.Get(x,y) * toLight;
    if ((l.Get(x,y)>minL)&&(l.Get(x,y)<maxL)&&(dot>0.0)) ++sampleCount;
   }
  }
  
 // Create array to store all the estimates in...
  ds::Array<real32> samples(sampleCount);
 
 // Fill in the array...
  nat32 index = 0;
  for (nat32 y=0;y<l.Size(1);y++)
  {
   for (nat32 x=0;x<l.Size(0);x++)
   {
    real32 dot = needle.Get(x,y) * toLight;
    if ((l.Get(x,y)>minL)&&(l.Get(x,y)<maxL)&&(dot>0.0))
    {
     samples[index] = l.Get(x,y)/dot;
     ++index;
    }
   }
  }
  log::Assert(index==sampleCount);

 // Sort...
  samples.SortNorm();
  
 // Return middle value...
  return samples[sampleCount/2];
}

//------------------------------------------------------------------------------
 };
};
