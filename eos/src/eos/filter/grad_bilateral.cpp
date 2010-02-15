//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/filter/grad_bilateral.h"

#include "eos/ds/arrays.h"
#include "eos/math/gaussian_mix.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC void GradBilatGauss(const svt::Field<bs::ColourLuv> & in,svt::Field<real32> & dx,svt::Field<real32> & dy,
                             real32 spatialSd,real32 domainSd,real32 winSdMult,time::Progress * prog)
{
 // Generate windows for the range gaussian, its seperable so two 1D arrays of numbers...
  int32 rad = int32(math::RoundUp(spatialSd*winSdMult));
  ds::Array<real32> rangeDiff(rad*2+1);
  ds::Array<real32> rangeTang(rad*2+1);
  for (int32 i=0;i<(rad*2+1);i++)
  {
   rangeDiff[i] = math::GaussianDiff(spatialSd,real32(i-rad));
   rangeTang[i] = math::Gaussian(spatialSd,real32(i-rad));
  }


 // Iterate all the pixels and calculate each gradiant...
  prog->Push();
  for (int32 y=0;y<int32(in.Size(1));y++)
  {
   prog->Report(y,in.Size(1));
   prog->Push();
   for (int32 x=0;x<int32(in.Size(0));x++)
   {
    prog->Report(x,in.Size(0));

    // Do both direction at the same time...
     real32 sumX = 0.0;
     real32 divX = 0.0;
     real32 sumY = 0.0;
     real32 divY = 0.0;
     for (int32 v=-rad;v<=rad;v++)
     {
      for (int32 u=-rad;u<=rad;u++)
      {
       // Shared...
        int32 ox = math::Clamp(x+u,int32(0),int32(in.Size(0)-1));
        int32 oy = math::Clamp(y+v,int32(0),int32(in.Size(1)-1));
        real32 colDist = math::Sqrt(math::Sqr(in.Get(x,y).l - in.Get(ox,oy).l) +
                                    math::Sqr(in.Get(x,y).u - in.Get(ox,oy).u) +
                                    math::Sqr(in.Get(x,y).v - in.Get(ox,oy).v));
        real32 domWeight = math::UnNormGaussian(domainSd,colDist);

        real32 nc = in.Get(ox,oy).l/100.0;

       // X...
       {
        real32 weight = domWeight * rangeDiff[u+rad] * rangeTang[v+rad];
        sumX += nc * weight;
        divX += math::Abs(weight);
       }

       // Y...
       {
        real32 weight = domWeight * rangeDiff[v+rad] * rangeTang[u+rad];
        sumY += nc * weight;
        divY += math::Abs(weight);
       }
      }
     }
     dx.Get(x,y) = -sumX/divX;
     dy.Get(x,y) = -sumY/divY;
     if (!math::IsFinite(dx.Get(x,y))) dx.Get(x,y) = 0.0;
     if (!math::IsFinite(dy.Get(x,y))) dy.Get(x,y) = 0.0;
   }
   prog->Pop();
  }
  prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
