//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/visualize.h"

#include "eos/math/functions.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
EOS_FUNC void RenderDisp(const svt::Field<real32> & disp,svt::Field<bs::ColourL> & out,real32 mult)
{
 if (math::IsZero(mult))
 {
  mult = 0.1; // Divide by zero avoidance.
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++) mult = math::Max(mult,math::Abs(disp.Get(x,y)));
  }
  LogAlways("[eos.stereo.RenderDisp] Maximum absolute disparity" << LogDiv() << mult);
  mult = 1.0/mult;  
 }

 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y).l = math::Clamp(math::Abs(disp.Get(x,y))*mult,real32(0.0),real32(1.0));
  }
 }
}

EOS_FUNC svt::Var * RenderDsiRow(const svt::Field<real32> & dsi,nat32 row)
{
 nat32 outWidth = dsi.Size(0);
 nat32 outHeight = dsi.Size(2);
 svt::Var * ret = new svt::Var(dsi.GetVar()->GetCore());
  ret->Setup2D(outWidth,outHeight);
  bs::ColourL lIni(0.0);
  ret->Add("l",lIni);
 ret->Commit();

 svt::Field<bs::ColourL> l; 
 ret->ByName("l",l);

 real32 minV = 0.0;
 real32 maxV = 0.0;
 for (nat32 y=0;y<outHeight;y++)
 {
  for (nat32 x=0;x<outWidth;x++)
  {
   real32 val = dsi.Get(x,row,y);
   minV = math::Min(minV,val);
   maxV = math::Max(maxV,val);
   l.Get(x,y) = val;
  }
 }

 (*ret)["min"].Set(minV);
 (*ret)["max"].Set(maxV);

 real32 mult = 1.0/(maxV-minV);
 for (nat32 y=0;y<outHeight;y++)
 {
  for (nat32 x=0;x<outWidth;x++)
  {
   l.Get(x,y).l = (l.Get(x,y).l-minV)*mult;
  }
 }

 return ret;
}

//------------------------------------------------------------------------------
 };
};
