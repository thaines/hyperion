//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/fit/light_ambient.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
LightAmb::LightAmb()
:lightDir(0.0,0.0,1.0),minAlbedo(0.001),maxAlbedo(1.5),minAmbient(0.0),maxAmbient(0.75),
maxSegCostPP(0.1),lowAlbErr(8192.0),segPruneThresh(0.1),
albedoRecDepth(7),ambientRecDepth(7),bestAmbient(0.0)
{}

LightAmb::~LightAmb()
{}

void LightAmb::SetData(svt::Field<nat32> s,svt::Field<real32> i,svt::Field<math::Fisher> d)
{
 seg = s;
 irr = i;
 dir = d;
}

void LightAmb::SetLightDir(const bs::Normal & ld)
{
 lightDir = ld;
}

void LightAmb::SetAlbRange(real32 min,real32 max)
{
 minAlbedo = min;
 maxAlbedo = max;
}

void LightAmb::SetAmbRange(real32 min,real32 max)
{
 minAmbient = min;
 maxAmbient = max;
}

void LightAmb::SetSegCapPP(real32 maxCost)
{
 maxSegCostPP = maxCost;
}

void LightAmb::SetIrrErr(real32 sd)
{
 lowAlbErr = 1.0/(2.0*math::Sqr(sd));
}

void LightAmb::SetPruneThresh(real32 cor)
{
 segPruneThresh = cor;
}

void LightAmb::SetSubdivs(nat32 ambient,nat32 albedo)
{
 albedoRecDepth = albedo;
 ambientRecDepth = ambient;
}

void LightAmb::Run(time::Progress * prog)
{
 prog->Push();
 
 // Count the number of segments...
  nat32 segCount = 1;
  for (nat32 y=0;y<seg.Size(1);y++)
  {
   for (nat32 x=0;x<seg.Size(0);x++) segCount = math::Max(segCount,seg.Get(x,y)+1);
  }


 // Calculate the correlation for each segment...
  {
   // Create array to hold expectation values for every segment...
    ds::Array<SegValue> segExp(segCount);
    for (nat32 s=0;s<segCount;s++)
    {
     segExp[s].div = 0.0;
      
     segExp[s].expI = 0.0;
     segExp[s].expX = 0.0;
     segExp[s].expY = 0.0;
     segExp[s].expZ = 0.0;

     segExp[s].expSqrI = 0.0;
     segExp[s].expSqrX = 0.0;
     segExp[s].expSqrY = 0.0;
     segExp[s].expSqrZ = 0.0;
       
     segExp[s].expIrrX = 0.0;
     segExp[s].expIrrY = 0.0;
     segExp[s].expIrrZ = 0.0;
    }
    
   // Now do a pass over the image and sum up the above for each segment...
    for (nat32 y=0;y<seg.Size(1);y++)
    {
     for (nat32 x=0;x<seg.Size(0);x++)
     {
      real32 l = dir.Get(x,y).Length();
      if (!math::IsZero(l))
      {
       real32 w = l;
       bs::Normal pos;
       for (nat32 i=0;i<3;i++) pos[i] = math::InvCos(dir.Get(x,y)[i]/l); // Makes 'em 0..pi
       nat32 s = seg.Get(x,y);
       real32 ir = irr.Get(x,y);


       segExp[s].div += w;
     
       segExp[s].expI += w*ir;
       segExp[s].expX += w*pos[0];
       segExp[s].expY += w*pos[1];
       segExp[s].expZ += w*pos[2];

       segExp[s].expSqrI += w*math::Sqr(ir);
       segExp[s].expSqrX += w*math::Sqr(pos[0]);
       segExp[s].expSqrY += w*math::Sqr(pos[1]);
       segExp[s].expSqrZ += w*math::Sqr(pos[2]);
      
       segExp[s].expIrrX += w*ir*pos[0];
       segExp[s].expIrrY += w*ir*pos[1];
       segExp[s].expIrrZ += w*ir*pos[2];
      }
     }
    }

   // Now calculate the 0..1 value for every segment, as calculated from correlation...
    cor.Size(segCount);
    for (nat32 s=0;s<segCount;s++)
    {
     if (!math::IsZero(segExp[s].div))
     {
      real32 sqrDivI = (segExp[s].expSqrI/segExp[s].div) - math::Sqr(segExp[s].expI/segExp[s].div);
      real32 sqrDivX = (segExp[s].expSqrX/segExp[s].div) - math::Sqr(segExp[s].expX/segExp[s].div);
      real32 sqrDivY = (segExp[s].expSqrY/segExp[s].div) - math::Sqr(segExp[s].expY/segExp[s].div);
      real32 sqrDivZ = (segExp[s].expSqrZ/segExp[s].div) - math::Sqr(segExp[s].expZ/segExp[s].div);
        
      if ((sqrDivI>0.0)&&(!math::IsZero(sqrDivI)))
      {
       real32 costX = 0.0,costY = 0.0,costZ = 0.0;
        
       if ((sqrDivX>0.0)&&(!math::IsZero(sqrDivX)))
       {
        costX = (segExp[s].expIrrX/segExp[s].div) - (segExp[s].expX*segExp[s].expI/math::Sqr(segExp[s].div));
        costX /= math::Sqrt(sqrDivX) * math::Sqrt(sqrDivI);
       }
         
       if ((sqrDivY>0.0)&&(!math::IsZero(sqrDivY)))
       {
        costY = (segExp[s].expIrrY/segExp[s].div) - (segExp[s].expY*segExp[s].expI/math::Sqr(segExp[s].div));
        costY /= math::Sqrt(sqrDivY) * math::Sqrt(sqrDivI);
       }
         
       if ((sqrDivZ>0.0)&&(!math::IsZero(sqrDivZ)))
       {
        costZ = (segExp[s].expIrrZ/segExp[s].div) - (segExp[s].expZ*segExp[s].expI/math::Sqr(segExp[s].div));
        costZ /= math::Sqrt(sqrDivZ) * math::Sqrt(sqrDivI);
       }
         
       cor[s] = math::Sqrt(math::Sqr(costX) + math::Sqr(costY) + math::Sqr(costZ));
      }
      else cor[s] = -1.0;
     }
     else cor[s] = -1.0;
    }
  }


 // The first real task is to build an array of parameters for the function
 // being optimised, one set of parameters for each pixel. The pixels are
 // arranged by segment, with a supporting structure to get all pixels for a 
 // given segment...
  ds::Array<Pixel> pixel(seg.Size(0)*seg.Size(1));
  ds::Array<nat32> pixelOffset(segCount+1);
  {
   // First work out the size of each segment...
    ds::Array<nat32> segSize(segCount);
    for (nat32 i=0;i<segCount;i++) segSize[i] = 0;
    for (nat32 y=0;y<seg.Size(1);y++)
    {
     for (nat32 x=0;x<seg.Size(0);x++) segSize[seg.Get(x,y)] += 1;
    }
    
   // Now fill in the offset buffer and re-zero the segSize buffer for re-use...
    pixelOffset[0] = 0;
    for (nat32 i=0;i<segCount;i++)
    {
     pixelOffset[i+1] = pixelOffset[i] + segSize[i];
     segSize[i] = 0;
    }
 
   // And finally fill in the pixel buffer...
    for (nat32 y=0;y<seg.Size(1);y++)
    {
     for (nat32 x=0;x<seg.Size(0);x++)
     {
      nat32 s = seg.Get(x,y);
      Pixel & targ = pixel[pixelOffset[s] + segSize[s]];
      
      targ.irr = irr.Get(x,y);
      
      real32 k = dir.Get(x,y).Length();
      if (!math::IsZero(k))
      {
       bs::Normal u = dir.Get(x,y); u /= k;
       real32 dot = u * lightDir;
      
       targ.a = -k * dot;
       targ.b = -k * math::Sqrt(1.0 - math::Sqr(dot));
       targ.min = math::Abs(targ.a)/math::Sqrt(math::Sqr(targ.a) + math::Sqr(targ.b));
      }
      else
      {
       targ.a = 0.0;
       targ.b = 0.0;
       targ.min = -1.0;
      }
      
      segSize[s] += 1;
     }
    }
  }


 // 

 
 prog->Pop();
}

//------------------------------------------------------------------------------
real32 LightAmb::Cost(real32 amb,real32 alb,const ds::Array<Pixel> & pixel,nat32 start,nat32 size)
{
 LogTime("eos::fit::LightAmb::Cost");
 
 real32 out = 0.0;
 
 // Iterate all the pixels and sum...
  for (nat32 i=0;i<size;i++)
  {
   Pixel & targ = pixel[start+i];
   
   real32 r = (targ.irr - amb)/alb;
   if (r<0.0)
   {
    out += targ.b + lowAlbErr * (amb - targ.irr);
   }
   else
   {
    if (r>1.0)
    {
     out += targ.a + lowAlbErr * (targ.irr - amb - alb);
    }
    else
    {
     out += targ.a*r + targ.b*math::Sqrt(1.0-math::Sqr(r));
    }
   }
  }
 
 return out;
}

void LightAmb::CostRange(real32 lowAmb,real32 highAmb,
                         real32 lowAlb,real32 highAlb,
                         real32 & outLow,real32 & outHigh,
                         const ds::Array<Pixel> & pixel,nat32 start,nat32 size)
{
 LogTime("eos::fit::LightAmb::CostRange");

 outLow = 0.0;
 outHigh = 0.0;

 real32 invLowAlb = 1.0/lowAlb; 
 real32 invHighAlb = 1.0/highAlb;

 // Iterate all the pixels and sum...
  for (nat32 i=0;i<size;i++)
  {
   Pixel & targ = pixel[start+i];

   // First calculate the r value range...
    real32 lowR = targ.irr - highAmb;
    if (lowR>0.0) lowR *= invHighAlb;
             else lowR *= invLowAlb;

    real32 highR = targ.irr - lowAmb;
    if (highR>0.0) highR *= invLowAlb;
              else highR *= invHighAlb;
   
   
   // Calculate the minimum value in the range...
   // (Easy-ish as the function only has one minima and an (ignorable) pair of 
   // standing point/discontinuity things.)
    real32 low,high;

    // Calculate the value at lowR...
     real32 valLowR;
     if (lowR<0.0) valLowR = targ.b + lowAlbErr * (highAmb - targ.irr);
     else
     {
      if (lowR>1.0) valLowR = targ.a + lowAlbErr * (targ.irr - highAmb - highAlb);
      else
      {
       // Typical case...
        valLowR = targ.a*lowR + targ.b*math::Sqrt(1.0-math::Sqr(lowR));
      }
     }
     low = valLowR;
     high = valLowR;
   
    // Calculate the value at highR...
     real32 valHighR;
     if (highR<0.0) valHighR = targ.b + lowAlbErr * (lowAmb - targ.irr);
     else
     {
      if (highR>1.0) valHighR = targ.a + lowAlbErr * (targ.irr - lowAmb - lowAlb);
      else
      {
       // Typical case...
        valHighR = targ.a*highR + targ.b*math::Sqrt(1.0-math::Sqr(highR));
      }
     }
     low = math::Min(low,valHighR);
     high = math::Max(high,valHighR);
    
    // See if the minima is in the range, if it is add in its effect...
     if ((targ.min<highR)&&(targ.min>lowR))
     {
      real32 valAtMin = targ.a*targ.min + targ.b*math::Sqrt(1.0-math::Sqr(targ.min));
      low = math::Min(low,valAtMin);
     }
   
   // And sum in the min/max to the score so far...
    outLow += low;
    outHigh += high;
  }
} 

//------------------------------------------------------------------------------
 };
};