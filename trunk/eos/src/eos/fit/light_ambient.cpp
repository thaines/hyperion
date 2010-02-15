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
lowAlbErr(8192.0),segPruneThresh(0.1),
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

 // For the progress bar...
  nat32 step = 0;
  nat32 steps = 7 + DivSteps(ambientRecDepth);

 // Count the number of segments...
  prog->Report(step++,steps);
  nat32 segCount = 1;
  for (nat32 y=0;y<seg.Size(1);y++)
  {
   for (nat32 x=0;x<seg.Size(0);x++) segCount = math::Max(segCount,seg.Get(x,y)+1);
  }

 // Calculate the correlation for each segment...
  {
   // Create array to hold expectation values for every segment...
    prog->Report(step++,steps);
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
    prog->Report(step++,steps);
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
    prog->Report(step++,steps);
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
  ds::Array<nat32> pixelOffset(segCount+1);
  ds::Array<PixelAux> pixel;
  {
   // First work out the size of each segment...
    prog->Report(step++,steps);
    ds::Array<nat32> segSize(segCount);
    for (nat32 i=0;i<segCount;i++) segSize[i] = 0;
    for (nat32 y=0;y<seg.Size(1);y++)
    {
     for (nat32 x=0;x<seg.Size(0);x++)
     {
      if ((!math::IsZero(irr.Get(x,y)))&&(!math::IsZero(dir.Get(x,y).Length())))
      {
       segSize[seg.Get(x,y)] += 1;
      }
     }
    }
    
   // Now fill in the offset buffer and re-zero the segSize buffer for re-use...
    prog->Report(step++,steps);
    pixelOffset[0] = 0;
    for (nat32 i=0;i<segCount;i++)
    {
     if (cor[i]>segPruneThresh) pixelOffset[i+1] = pixelOffset[i] + segSize[i];
                           else pixelOffset[i+1] = pixelOffset[i];
     segSize[i] = 0;
    }
 
    pixel.Size(pixelOffset[segCount]);
  
   // And finally fill in the pixel buffer...
    prog->Report(step++,steps);
    for (nat32 y=0;y<seg.Size(1);y++)
    {
     for (nat32 x=0;x<seg.Size(0);x++)
     {
      if ((!math::IsZero(irr.Get(x,y)))&&(!math::IsZero(dir.Get(x,y).Length())))
      {
       nat32 s = seg.Get(x,y);
       PixelAux & targ = pixel[pixelOffset[s] + segSize[s]];
       segSize[s] += 1;

       real32 k = dir.Get(x,y).Length();
       real32 dot = (dir.Get(x,y) * lightDir) / k;
      
       targ.s = -k * dot;
       targ.t = -k * math::Sqrt(1.0 - math::Sqr(dot));
       targ.irr = irr.Get(x,y);
       targ.minR = math::Abs(targ.s)/math::Sqrt(math::Sqr(targ.s) + math::Sqr(targ.t));      
       targ.minC = targ.s*targ.minR + targ.t*math::Sqrt(1.0 - math::Sqr(targ.minR));
      }
     }
    }
  }



 // Now use branch and bound to find the optimum ambient term, keeping note of
 // albedo assignments each time we find a better albedo...
  // Setup output data structures and tempory storage...
   albedo.Size(segCount);
   ds::Array<real32> tempAlbedo(segCount);
   real32 maxMinCost = math::Infinity<real32>();
   
   ds::PriorityQueue<AlbRange> albWork;
   ds::PriorityQueue<AmbRange> ambWork;


  // Create the initial search range...
   AmbRange ini;
   ini.minAmbient = minAmbient;
   ini.maxAmbient = maxAmbient;
   ini.depth = ambientRecDepth;
   ini.lowMinCost = 0.0;
   ini.highMinCost = math::Infinity<real32>();
   
   ambWork.Add(ini);


  // Do the work by eatting the work queue - take a greedy approach to try and 
  // accelerate tying down the possible values...
   while ((ambWork.Size()!=0)&&(ambWork.Peek().lowMinCost<maxMinCost))
   {
    AmbRange targ = ambWork.Peek();
    ambWork.Rem();
    
    // Greedy on down whilst depth isn't 0...
     while (targ.depth!=0)
     {
      // Divide to make a low and high...
       real32 half = (targ.minAmbient + targ.maxAmbient) * 0.5;
       
       AmbRange low;
       low.minAmbient = targ.minAmbient;
       low.maxAmbient = half;
       low.depth = targ.depth-1;
       prog->Report(step++,steps);
       AmbRangeCost(low,pixel,pixelOffset,albWork);
      
       AmbRange high;
       high.minAmbient = half;
       high.maxAmbient = targ.maxAmbient;
       high.depth = targ.depth-1;
       prog->Report(step++,steps);
       AmbRangeCost(high,pixel,pixelOffset,albWork);
       
       maxMinCost = math::Min(maxMinCost,low.highMinCost,high.highMinCost);


      // Set the best as targ, store the other in the work queue if needed...
       if (low.lowMinCost<high.lowMinCost)
       {
        targ = low;
        if (high.lowMinCost<maxMinCost) ambWork.Add(high);
                                   else steps -= DivSteps(high.depth);
       }
       else
       {
        targ = high;
        if (low.lowMinCost<maxMinCost) ambWork.Add(low);
                                  else steps -= DivSteps(low.depth);
       }
     }
     
    // At the bottom take an actual sample at the half way position of the range...
     real32 half = (targ.minAmbient + targ.maxAmbient) * 0.5;
     prog->Report(step++,steps);
     real32 c = AmbCost(half,tempAlbedo,pixel,pixelOffset,albWork);
     
     if (c<maxMinCost)
     {
      maxMinCost = c;
      bestAmbient = half;
      for (nat32 i=0;i<segCount;i++) albedo[i] = tempAlbedo[i];
     }
   }

 
 prog->Pop();
}

//------------------------------------------------------------------------------
void LightAmb::AmbRangeCost(AmbRange & amb,const ds::Array<PixelAux> & pixel,
                            const ds::Array<nat32> & pixelOffset,ds::PriorityQueue<AlbRange> & work)
{
 LogTime("eos::fit::LightAmb::AmbRangeCost");
 
 amb.lowMinCost = 0.0;
 amb.highMinCost = 0.0;
 
 for (nat32 i=0;i<pixelOffset.Size()-1;i++)
 {
  nat32 size = pixelOffset[i+1] - pixelOffset[i];
  if (size!=0)
  {
   real32 outLow,outHigh;
   SegCostRange(amb.minAmbient,amb.maxAmbient,outLow,outHigh,pixel,pixelOffset[i],size,work);
   amb.lowMinCost += outLow;
   amb.highMinCost += outHigh;
  }
 }
}

real32 LightAmb::AmbCost(real32 amb,ds::Array<real32> & albedo,const ds::Array<PixelAux> & pixel,
                 const ds::Array<nat32> & pixelOffset,ds::PriorityQueue<AlbRange> & work)
{
 LogTime("eos::fit::LightAmb::AmbCost");

 real32 cost = 0.0;

 for (nat32 i=0;i<pixelOffset.Size()-1;i++)
 {
  nat32 size = pixelOffset[i+1] - pixelOffset[i];
  if (size!=0)
  {
   cost += SegCost(amb,pixel,pixelOffset[i],size,work,&albedo[i]);
  }
  else
  {
   albedo[i] = 0.0;
  }
 }

 return cost;
}

void LightAmb::SegCostRange(real32 lowAmb,real32 highAmb,
                            real32 & outLow,real32 & outHigh,
                            const ds::Array<PixelAux> & pixel,nat32 start,nat32 size,
                            ds::PriorityQueue<AlbRange> & work)
{
 LogTime("eos::fit::LightAmb::SegCostRange");

 // Empty work list...
  work.MakeEmpty();
   
 // Create initial work item...
  AlbRange ini;
  ini.minAlbedo = minAlbedo;
  ini.maxAlbedo = maxAlbedo;
  ini.depth = albedoRecDepth;
    
  ini.lowMinCost = 0.0; // Doesn't matter for this first entry.
  ini.highMinCost = math::Infinity<real32>(); // Provides no constraint - let the divisions do so.
  
  work.Add(ini);


 // Initialise output variables...
  outHigh = math::Infinity<real32>();
  outLow = math::Infinity<real32>();
   
   
 // Keep eatting work items until no more remain - take a greedy sub-pass 
 // approach to quickly find tight bounds...
  while ((work.Size()!=0)&&(work.Peek().lowMinCost<outHigh))
  {
   AlbRange targ = work.Peek();
   work.Rem();
   
   // Do a greedy search - first whitle down our target to a depth of 0 by subdivision...
    while (targ.depth!=0)
    {
     real32 halfAlb = (targ.minAlbedo + targ.maxAlbedo) * 0.5;
    
     AlbRange low;
     low.minAlbedo = targ.minAlbedo;
     low.maxAlbedo = halfAlb;
     low.depth = targ.depth - 1;
     CostDualRange(lowAmb,highAmb,low.minAlbedo,low.maxAlbedo,
                   low.lowMinCost,low.highMinCost,pixel,start,size);
     
     AlbRange high;
     high.minAlbedo = halfAlb;
     high.maxAlbedo = targ.maxAlbedo;
     high.depth = targ.depth - 1;
     CostDualRange(lowAmb,highAmb,high.minAlbedo,high.maxAlbedo,
                   high.lowMinCost,high.highMinCost,pixel,start,size);
     
     outHigh = math::Min(outHigh,low.highMinCost,high.highMinCost);
     
     if (low.lowMinCost<high.lowMinCost)
     {
      targ = low;
      if (high.lowMinCost<outHigh) work.Add(high);
      else outLow = math::Min(outLow,high.lowMinCost);
     }
     else
     {
      targ = high;
      if (low.lowMinCost<outHigh) work.Add(low);
      else outLow = math::Min(outLow,low.lowMinCost);
     }
    }
  
   // And now deal with the remaining non-range before moving on to a further target...
   {
    real32 alb = (targ.minAlbedo + targ.maxAlbedo) * 0.5;
    real32 oLow, oHigh;
    CostRangeAmb(lowAmb,highAmb,alb,oLow,oHigh,pixel,start,size);

    outHigh = math::Min(outHigh,oHigh);
    outLow = math::Min(outLow,oLow);
   }
  }


 // Eat remaining work items to make sure outLow is correct...
  while (work.Size()!=0)
  {
   outLow = math::Min(outLow,work.Peek().lowMinCost);
   work.Rem();
  }
}

real32 LightAmb::SegCost(real32 amb,const ds::Array<PixelAux> & pixel,nat32 start,nat32 size,
                         ds::PriorityQueue<AlbRange> & work,real32 * albedo)
{
 LogTime("eos::fit::LightAmb::SegCost");
 
 // Empty work list...
  work.MakeEmpty();
   
 // Create initial work item...
  AlbRange ini;
  ini.minAlbedo = minAlbedo;
  ini.maxAlbedo = maxAlbedo;
  ini.depth = albedoRecDepth;
    
  ini.lowMinCost = 0.0; // Doesn't matter for this first entry.
  ini.highMinCost = math::Infinity<real32>(); // "
  
  work.Add(ini);


 // Initialise pruning variable and associated albedo...
  real32 maxMinCost = math::Infinity<real32>();
  real32 bestAlbedo = 0.0;
   
   
 // Keep eatting work items until no more remain - take a greedy sub-pass 
 // approach to quickly find tight bounds...
  while ((work.Size()!=0)&&(work.Peek().lowMinCost<maxMinCost))
  {
   AlbRange targ = work.Peek();
   work.Rem();
   
   // Do a greedy search - first whitle down our target to a depth of 0 by subdivision...
    while (targ.depth!=0)
    {
     real32 halfAlb = (targ.minAlbedo + targ.maxAlbedo) * 0.5;
    
     AlbRange low;
     low.minAlbedo = targ.minAlbedo;
     low.maxAlbedo = halfAlb;
     low.depth = targ.depth - 1;
     CostRangeAlb(amb,low.minAlbedo,low.maxAlbedo,
                  low.lowMinCost,low.highMinCost,pixel,start,size);
     
     AlbRange high;
     high.minAlbedo = halfAlb;
     high.maxAlbedo = targ.maxAlbedo;
     high.depth = targ.depth - 1;
     CostRangeAlb(amb,high.minAlbedo,high.maxAlbedo,
                  high.lowMinCost,high.highMinCost,pixel,start,size);
     
     maxMinCost = math::Min(maxMinCost,low.highMinCost,high.highMinCost);
     
     if (low.lowMinCost<high.lowMinCost)
     {
      targ = low;
      if (high.lowMinCost<maxMinCost) work.Add(high);
     }
     else
     {
      targ = high;
      if (low.lowMinCost<maxMinCost) work.Add(low);
     }
    }
  
   // And now deal with the remaining non-range before moving on to a further target...
   {
    real32 alb = (targ.minAlbedo + targ.maxAlbedo) * 0.5;
    real32 c = Cost(amb,alb,pixel,start,size);
    
    if (c<maxMinCost)
    {
     maxMinCost = c;
     bestAlbedo = alb;
    }
   }
  }


 // Finish with the outputs...
  if (albedo) *albedo = bestAlbedo;
  return maxMinCost;
}

real32 LightAmb::Cost(real32 amb,real32 alb,const ds::Array<PixelAux> & pixel,nat32 start,nat32 size)
{
 LogTime("eos::fit::LightAmb::Cost");
 
 real32 out = 0.0;
 
 // Iterate all the pixels and sum...
  for (nat32 i=0;i<size;i++)
  {
   PixelAux & targ = pixel[start+i];
   
   real32 r = targ.irr/alb - amb;
   if (r<0.0)  out += targ.t + lowAlbErr * (amb*alb - targ.irr);
   else
   {
    if (r>1.0) out += targ.s + lowAlbErr * (targ.irr - alb*(1.0+amb));
          else out += targ.s*r + targ.t*math::Sqrt(1.0-math::Sqr(r));
   }
  }
 
 return out;
}

void LightAmb::CostRangeAlb(real32 amb,real32 lowAlb,real32 highAlb,
                            real32 & outLow,real32 & outHigh,
                            const ds::Array<PixelAux> & pixel,nat32 start,nat32 size)
{
 LogTime("eos::fit::LightAmb::CostRangeAlb");
 
 outLow = 0.0;
 outHigh = 0.0; 
 
 for (nat32 i=0;i<size;i++)
 {
  PixelAux & pix = pixel[start+i];
 
  real32 lowR = (pix.irr/highAlb) - amb;
  real32 highR = (pix.irr/lowAlb) - amb;
  
  // Calculate the cost for lowR and the cost for highR...
   // low...
    real32 lowC = (lowR<0.0)?(pix.t + lowAlbErr*(highAlb*amb - pix.irr)):
                  ((lowR>1.0)?(pix.s + lowAlbErr*(pix.irr - highAlb*(1.0+amb))):
                  (pix.s*lowR + pix.t*math::Sqrt(1.0 - math::Sqr(lowR))));

   // high...
    real32 highC = (highR<0.0)?(pix.t + lowAlbErr*(lowAlb*amb - pix.irr)):
                   ((highR>1.0)?(pix.s + lowAlbErr*(pix.irr - lowAlb*(1.0+amb))):
                   (pix.s*highR + pix.t*math::Sqrt(1.0 - math::Sqr(highR))));
   
  // If the minima is inside the range then we use the minimum, otherwise we use
  // the two bounds already calculated...
   if ((highR<pix.minR)||(lowR>pix.minR))
   {
    outLow += math::Min(lowC,highC);
   }
   else outLow += pix.minC;
   
  // Maximum cost...
   outHigh += math::Max(lowC,highC);
 }
}

void LightAmb::CostRangeAmb(real32 lowAmb,real32 highAmb,real32 alb,
                            real32 & outLow,real32 & outHigh,
                            const ds::Array<PixelAux> & pixel,nat32 start,nat32 size)
{
 LogTime("eos::fit::LightAmb::CostRangeAmb");
 
 outLow = 0.0;
 outHigh = 0.0; 
 
 for (nat32 i=0;i<size;i++)
 {
  PixelAux & pix = pixel[start+i];
 
  real32 lowR = (pix.irr/alb) - highAmb;
  real32 highR = (pix.irr/alb) - lowAmb;
  
  // Calculate the cost for lowR and the cost for highR...
   // low...
    real32 lowC = (lowR<0.0)?(pix.t + lowAlbErr*(alb*highAmb - pix.irr)):
                  ((lowR>1.0)?(pix.s + lowAlbErr*(pix.irr - alb*(1.0+highAmb))):
                  (pix.s*lowR + pix.t*math::Sqrt(1.0 - math::Sqr(lowR))));

   // high...
    real32 highC = (highR<0.0)?(pix.t + lowAlbErr*(alb*lowAmb - pix.irr)):
                   ((highR>1.0)?(pix.s + lowAlbErr*(pix.irr - alb*(1.0+lowAmb))):
                   (pix.s*highR + pix.t*math::Sqrt(1.0 - math::Sqr(highR))));
   
  // If the minima is inside the range then we use the minimum, otherwise we use
  // the two bounds already calculated...
   if ((highR<pix.minR)||(lowR>pix.minR))
   {
    outLow += math::Min(lowC,highC);
   }
   else outLow += pix.minC;
   
  // Maximum cost...
   outHigh += math::Max(lowC,highC);
 }
}

void LightAmb::CostDualRange(real32 lowAmb,real32 highAmb,
                             real32 lowAlb,real32 highAlb,
                             real32 & outLow,real32 & outHigh,
                             const ds::Array<PixelAux> & pixel,nat32 start,nat32 size)
{
 LogTime("eos::fit::LightAmb::CostDualRange");
 
 outLow = 0.0;
 outHigh = 0.0; 
 
 for (nat32 i=0;i<size;i++)
 {
  PixelAux & pix = pixel[start+i];
 
  real32 lowR = (pix.irr/highAlb) - highAmb;
  real32 highR = (pix.irr/lowAlb) - lowAmb;
  
  // Calculate the cost for lowR and the cost for highR...
   // low...
    real32 lowC = (lowR<0.0)?(pix.t + lowAlbErr*(highAlb*highAmb - pix.irr)):
                  ((lowR>1.0)?(pix.s + lowAlbErr*(pix.irr - highAlb*(1.0+highAmb))):
                  (pix.s*lowR + pix.t*math::Sqrt(1.0 - math::Sqr(lowR))));

   // high...
    real32 highC = (highR<0.0)?(pix.t + lowAlbErr*(lowAlb*lowAmb - pix.irr)):
                   ((highR>1.0)?(pix.s + lowAlbErr*(pix.irr - lowAlb*(1.0+lowAmb))):
                   (pix.s*highR + pix.t*math::Sqrt(1.0 - math::Sqr(highR))));
   
  // If the minima is inside the range then we use the minimum, otherwise we use
  // the two bounds already calculated...
   if ((highR<pix.minR)||(lowR>pix.minR))
   {
    outLow += math::Min(lowC,highC);
   }
   else outLow += pix.minC;
   
  // Maximum cost...
   outHigh += math::Max(lowC,highC);
 }
} 

//------------------------------------------------------------------------------
 };
};
