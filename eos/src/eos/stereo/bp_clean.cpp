//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/stereo/bp_clean.h"

#include "eos/inf/gauss_integration.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
CleanDSI::CleanDSI()
:dsi(null<DSI*>()),strength(1.0),cutoff(16.0),width(2.0),iters(100)
{}

CleanDSI::~CleanDSI()
{}

void CleanDSI::Set(const svt::Field<bs::ColourLuv> & i)
{
 image = i;
}

void CleanDSI::Set(const DSI & d)
{
 dsi = &d;
}

void CleanDSI::SetMask(const svt::Field<bit> & m)
{
 mask = m;
}

void CleanDSI::SetSD(const svt::Field<real32> & s)
{
 sdOverride = s;
}

void CleanDSI::Set(real32 s,real32 c,real32 w,nat32 i)
{
 strength = s;
 cutoff = c;
 width = w;
 iters = i;
}

void CleanDSI::Run(time::Progress * prog)
{
 prog->Push();

 // Create the solver, resize the output...
  out.Resize(dsi->Width(),dsi->Height());

  inf::IntegrateBP ibp(dsi->Width(),dsi->Height());
  ibp.SetIters(iters);


 // Fill in the disparity estimates...
  for (nat32 y=0;y<ibp.Height();y++)
  {
   for (nat32 x=0;x<ibp.Width();x++)
   {
    if ((dsi->Size(x,y)!=0)&&((mask.Valid()==false)||mask.Get(x,y)))
    {
     // Calculate mean...
      real32 mean = 0.0;
      for (nat32 i=0;i<dsi->Size(x,y);i++) mean += dsi->Disp(x,y,i);
      mean /= real32(dsi->Size(x,y));

     // Calculate sd, have to consider that the disparities represent ranges,
     // which kinda causes a mild headache, unless an override has been provided...
      real32 sd = 0.0;
      if (sdOverride.Valid())
      {
       sd = sdOverride.Get(x,y);
      }
      else
      {
       for (nat32 i=0;i<dsi->Size(x,y);i++)
       {
        sd += 0.5*(math::Abs(dsi->Disp(x,y,i)+dsi->DispWidth(x,y,i) - mean) +
                   math::Abs(dsi->Disp(x,y,i)-dsi->DispWidth(x,y,i) - mean));
       }
       sd /= real32(dsi->Size(x,y));
      }

     // Store the relevant output...
      ibp.SetVal(x,y,mean,1.0/sd);
    }
   }
  }


 // Fill in the smoothing terms...
  for (nat32 y=0;y<ibp.Height();y++)
  {
   for (nat32 x=0;x<ibp.Width();x++)
   {
    if ((mask.Valid()==false)||mask.Get(x,y))
    {
     // Horizontal...
      if ((x+1<ibp.Width())&&((mask.Valid()==false)||mask.Get(x+1,y)))
      {
       real32 diff = math::Abs(image.Get(x,y).l-image.Get(x+1,y).l) +
                     math::Abs(image.Get(x,y).u-image.Get(x+1,y).u) +
                     math::Abs(image.Get(x,y).v-image.Get(x+1,y).v);
       real32 invSd = strength*math::SigmoidCutoff(diff,cutoff,width);
       ibp.SetRel(x,y,0,1.0,0.0,invSd);
       ibp.SetRel(x+1,y,2,1.0,0.0,invSd);
      }

     // Vertical...
      if ((y+1<ibp.Height())&&((mask.Valid()==false)||mask.Get(x,y+1)))
      {
       real32 diff = math::Abs(image.Get(x,y).l-image.Get(x,y+1).l) +
                     math::Abs(image.Get(x,y).u-image.Get(x,y+1).u) +
                     math::Abs(image.Get(x,y).v-image.Get(x,y+1).v);
       real32 invSd = strength*math::SigmoidCutoff(diff,cutoff,width);
       ibp.SetRel(x,y,1,1.0,0.0,invSd);
       ibp.SetRel(x,y+1,3,1.0,0.0,invSd);
      }
    }
   }
  }

 // Run the solver...
  ibp.Run(prog);


 // Extract the result...
  for (nat32 y=0;y<out.Height();y++)
  {
   for (nat32 x=0;x<out.Width();x++)
   {
    if (ibp.Defined(x,y)&&((mask.Valid()==false)||mask.Get(x,y)))
    {
     out.Get(x,y) = ibp.Expectation(x,y);
    }
    else
    {
     out.Get(x,y) = math::Infinity<real32>();
    }
   }
  }

 prog->Pop();
}

nat32 CleanDSI::Width() const
{
 return out.Width();
}

nat32 CleanDSI::Height() const
{
 return out.Height();
}

real32 CleanDSI::Get(nat32 x,nat32 y) const
{
 return out.Get(x,y);
}

void CleanDSI::GetMap(svt::Field<real32> & o)
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++) o.Get(x,y) = out.Get(x,y);
 }
}

//------------------------------------------------------------------------------
 };
};
