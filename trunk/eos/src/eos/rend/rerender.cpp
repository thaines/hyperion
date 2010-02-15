//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/rend/rerender.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
EOS_FUNC void LambertianNeedleRender(const svt::Field<real32> & albedo,
                                     const svt::Field<bs::Normal> & needle,
                                     const bs::Normal & toLight,
                                     svt::Field<real32> & out)
{
 for (nat32 y=0;y<albedo.Size(1);y++)
 {
  for (nat32 x=0;x<albedo.Size(0);x++)
  {
   out.Get(x,y) = math::Clamp<real32>(albedo.Get(x,y) * (toLight * needle.Get(x,y)),0.0,1.0);
  }
 }
}

//------------------------------------------------------------------------------
 };
};
