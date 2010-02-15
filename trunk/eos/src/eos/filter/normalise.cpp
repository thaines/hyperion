//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/filter/normalise.h"

#include "eos/math/functions.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC void Normalise(svt::Field<real32> & f)
{
 // First pass to find the minimum and maximum...
  real32 min = f.Get(0,0);
  real32 max = f.Get(0,0);
  for (nat32 y=0;y<f.Size(1);y++)
  {
   for (nat32 x=0;x<f.Size(0);x++)
   {
    min = math::Min(min,f.Get(x,y));
    max = math::Max(max,f.Get(x,y));
   }	  
  }
 
 // Second pass to adjust...
  for (nat32 y=0;y<f.Size(1);y++)
  {
   for (nat32 x=0;x<f.Size(0);x++)
   {
    f.Get(x,y) = (f.Get(x,y)-min)/(max-min);
   }	  
  }
}

//------------------------------------------------------------------------------
 };
};
