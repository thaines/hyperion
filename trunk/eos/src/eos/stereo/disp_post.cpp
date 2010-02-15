//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/disp_post.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
EOS_FUNC void FillDispInf(svt::Field<real32> & disp,const svt::Field<bit> & valid)
{
 for (nat32 y=0;y<disp.Size(1);y++)
 {
  for (nat32 x=0;x<disp.Size(0);x++)
  {
   if (!valid.Get(x,y))
   {
    disp.Get(x,y) = 0.0;
   }
  }
 }
}

EOS_FUNC void FillDispLeft(svt::Field<real32> & disp,const svt::Field<bit> & valid)
{
 for (nat32 y=0;y<disp.Size(1);y++)
 {
  if (!valid.Get(0,y)) disp.Get(0,y) = 0.0;

  for (nat32 x=1;x<disp.Size(0);x++)
  {
   if (!valid.Get(x,y))
   {
    disp.Get(x,y) = disp.Get(x-1,y);
   }
  }
 }
}

//------------------------------------------------------------------------------
 };
};
