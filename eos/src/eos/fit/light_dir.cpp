//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/fit/light_dir.h"

#include "eos/alg/shapes.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
LightDir::LightDir()
:minAlbedo(0.001),maxAlbedo(3.0),maxSegCostPP(0.1),lowAlbErr(8192.0),segPruneThresh(0.3),
subdiv(4),recDepth(8),bestLightDir(0.0,0.0,1.0)
{}

LightDir::~LightDir()
{}

void LightDir::SetData(svt::Field<nat32> s,svt::Field<real32> i,svt::Field<math::Fisher> d)
{
 seg = s;
 irr = i;
 dir = d;
}

void LightDir::SetAlbRange(real32 min,real32 max)
{
 minAlbedo = min;
 maxAlbedo = max;
}
void LightDir::SetSegCapPP(real32 maxCost)
{
 maxSegCostPP = maxCost;
}

void LightDir::SetIrrErr(real32 sd)
{
 lowAlbErr = 1.0/(2.0*math::Sqr(sd));
}

void LightDir::SetPruneThresh(real32 cor)
{
 segPruneThresh = cor;
}

void LightDir::SetSampleSubdiv(nat32 sd)
{
 subdiv = sd;
}

void LightDir::SetRecursion(nat32 depth)
{
 recDepth = depth;
}

void LightDir::Run(time::Progress * prog)
{
 LogTime("eos::fit::LightDir::Run");
 prog->Push();

 // Count how many segments there are...
  nat32 segCount = 1;
  for (nat32 y=0;y<seg.Size(1);y++)
  {
   for (nat32 x=0;x<seg.Size(0);x++) segCount = math::Max(segCount,seg.Get(x,y)+1);
  }

 // Prep for logging...
  nat32 step = 0;
  nat32 steps = 5 + segCount + 3;
  
 
 // Calculate the correlation for each segment...
  prog->Report(step++,steps);
  prog->Push();
  {
   // Create array to hold expectation values for every segment...
    prog->Report(0,2+seg.Size(1));
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
     prog->Report(1+y,2+seg.Size(1));
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

   // Now calculate the 0..1 value for every segment...
    prog->Report(1+seg.Size(1),2+seg.Size(1));
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
  prog->Pop();
 
 
 
 // Generate the sampling set of light source directions to sample...
  prog->Report(step++,steps);
  alg::HemiOfNorm hon(subdiv);
  lc.Size(hon.Norms());
  for (nat32 i=0;i<lc.Size();i++) lc[i].dir = hon.Norm(i);



 // Now generate the data structure we actually pass around from the input - 
 // group data points by segment...
  prog->Report(step++,steps);  
  ds::Array<nat32> size(segCount); // Number of pixels in each segment.
  ds::Array<nat32> offset(segCount+1); // Array of offsets for each segment, with size on end.
  
  // Count pixels in each segment, setup offset structure...
   prog->Report(step++,steps);
   for (nat32 i=0;i<size.Size();i++) size[i] = 0;
   for (nat32 y=0;y<seg.Size(1);y++)
   {
    for (nat32 x=0;x<seg.Size(0);x++)
    {
     if (!math::IsZero(irr.Get(x,y)))
     {
      size[seg.Get(x,y)] += 1;
     }
    }
   }
   
   offset[0] = 0;
   for (nat32 i=0;i<size.Size();i++) offset[i+1] = offset[i] + size[i];
   ds::Array<Pixel> pixel(offset[size.Size()]); // Array of all pixels, grouped by segment.
   
  // Copy pixel information into the pixel buffer...
   prog->Report(step++,steps);
   nat32 maxSegSize = 1;
   for (nat32 i=0;i<size.Size();i++)
   {
    maxSegSize = math::Max(maxSegSize,size[i]);
    size[i] = 0;
   }
   
   for (nat32 y=0;y<seg.Size(1);y++)
   {
    for (nat32 x=0;x<seg.Size(0);x++)
    {
     if (!math::IsZero(irr.Get(x,y)))
     {
      nat32 s = seg.Get(x,y);
      Pixel & targ = pixel[offset[s]+size[s]];
      size[s] += 1;
     
      targ.irr = irr.Get(x,y);
      targ.dir = dir.Get(x,y);
     }
    }
   }


 // Now iterate the light sources and segments and sum up the costs...
  prog->Report(step++,steps);
  ds::Array<real32> segCost(lc.Size());
  ds::Array<PixelAux> tAux(maxSegSize);
  ds::PriorityQueue<CostRange> tWork(recDepth*4);
 
  for (nat32 l=0;l<lc.Size();l++) lc[l].cost = 0.0;

  for (nat32 s=0;s<segCount;s++)
  {
   prog->Report(step++,steps);
   nat32 segSize = offset[s+1] - offset[s];
   if ((segSize!=0)&&(cor[s]>segPruneThresh))
   {
    prog->Push();
    for (nat32 l=0;l<lc.Size();l++)
    {
     prog->Report(l,lc.Size());
     segCost[l] = SegLightCost(lc[l].dir,recDepth,pixel,offset[s],segSize, 
                              tAux,tWork);
    }
    prog->Pop();
   
    real32 minLightCost = math::Infinity<real32>();
    for (nat32 l=0;l<lc.Size();l++) minLightCost = math::Min(minLightCost,segCost[l]);

    for (nat32 l=0;l<lc.Size();l++) lc[l].cost += math::Min(segCost[l]-minLightCost,maxSegCostPP*segSize);
   }
  }

 
 // Find the lowest costed light source direction and store as the best...
  prog->Report(step++,steps);
  nat32 best = 0;
  for (nat32 l=1;l<lc.Size();l++)
  {
   if (lc[l].cost<lc[best].cost) best = l;
  }
  bestLightDir = lc[best].dir;


 // Calculate an albedo map using the choosen light source direction...
  prog->Report(step++,steps);
  albedo.Size(segCount);
  prog->Push();
  for (nat32 s=0;s<segCount;s++)
  {
   prog->Report(s,segCount);
   if (offset[s+1]!=offset[s])
   {
    SegLightCost(bestLightDir,recDepth, pixel,offset[s],offset[s+1]-offset[s], 
                 tAux,tWork,&albedo[s]);
   }
   else albedo[s] = 0.0;
  }
  prog->Pop();


 prog->Pop();
}

const bs::Normal & LightDir::BestLightDir() const
{
 return bestLightDir;
}

nat32 LightDir::SampleSize() const
{
 return lc.Size();
}

const bs::Normal & LightDir::SampleDir(nat32 i) const
{
 return lc[i].dir;
}

real32 LightDir::SampleCost(nat32 i) const
{
 return lc[i].cost;
}

//------------------------------------------------------------------------------
real32 LightDir::SegLightCost(const bs::Normal & lightDir,nat32 recDepth,
                              const ds::Array<Pixel> & data,nat32 startInd,nat32 length,
                              ds::Array<PixelAux> & tAux,ds::PriorityQueue<CostRange> & tWork,
                              real32 * bestAlbedo)
{
 if (length==0)
 {
  if (bestAlbedo) *bestAlbedo = 0.0;
  return math::Infinity<real32>();
 }
 LogTime("eos::fit::LightDir::SegLightCost");

 // First fill in tAux from the data and lightDir...
  for (nat32 i=0;i<length;i++)
  {
   real32 k = data[startInd+i].dir.Length();
   real32 dot = (lightDir * data[startInd+i].dir) / k;
   if (!math::IsFinite(dot)) dot = 1.0;
   
   tAux[i].mult = -k*math::Sqrt(1.0-math::Sqr(dot));
   tAux[i].irr = data[startInd+i].irr;
   tAux[i].irrSqr = math::Sqr(data[startInd+i].irr);
   tAux[i].c = -k*dot*data[startInd+i].irr;
   tAux[i].lowAlbCost = -k*dot;
  }


  
 // Before doing the actual work queue stuff we do a single depth first greedy search -
 // this will usually raise the bar for processing other jobs high enough to avoid a load
 // of work - we push the excess off into the work queue...
  tWork.MakeEmpty();
  real32 maxCost = math::Infinity<real32>(); // Any cost higher than this value has already been beaten.
  real32 bestAlb = 0.0;
  {
   CostRange targ;
   targ.minA = minAlbedo;
   targ.maxA = maxAlbedo;
   targ.depth = recDepth;
   targ.CalcCost(tAux,length,lowAlbErr);
  
   while(targ.depth!=0)
   {
    real32 half = 0.5*(targ.minA+targ.maxA);
    
    CostRange low;
    low.minA = targ.minA;
    low.maxA = half;
    low.depth = targ.depth - 1;
    low.CalcCost(tAux,length,lowAlbErr);
    
    CostRange high;
    high.minA = half;
    high.maxA = targ.maxA;
    high.depth = targ.depth - 1;
    high.CalcCost(tAux,length,lowAlbErr);
    
    maxCost = math::Min(maxCost,low.maxCost,high.maxCost);
    
    if (low.minCost<high.minCost)
    {
     targ = low;
     if (high.minCost<maxCost) tWork.Add(high);
    }
    else
    {
     targ = high;
     if (low.minCost<maxCost) tWork.Add(low);   
    }
   }
   
   real32 half = 0.5*(targ.minA+targ.maxA);
   real32 cost = CalcCost(half,tAux,length);
   maxCost = math::Min(maxCost,cost);
   bestAlb = half;
  }


 // Keep processing work queue items until done...
  while ((tWork.Size()!=0)&&(tWork.Peek().minCost<maxCost))
  {
   CostRange targ = tWork.Peek();
   tWork.Rem();
   
   real32 half = 0.5*(targ.minA+targ.maxA);
   
   if (targ.depth==0)
   {
    // Take the average, calculate the cost at the average...
     real32 cost = CalcCost(half,tAux,length);
     if (cost<maxCost)
     {
      maxCost = cost;
      bestAlb = half;
     }
   }
   else
   {
    CostRange low;
    low.minA = targ.minA;
    low.maxA = half;
    low.depth = targ.depth - 1;
    low.CalcCost(tAux,length,lowAlbErr);
    
    if (low.minCost<maxCost)
    {
     tWork.Add(low);
     maxCost = math::Min(maxCost,low.maxCost);
    }
    
    CostRange high;
    high.minA = half;
    high.maxA = targ.maxA;
    high.depth = targ.depth - 1;
    high.CalcCost(tAux,length,lowAlbErr);   
   
    if (high.minCost<maxCost)
    {
     tWork.Add(high);
     maxCost = math::Min(maxCost,high.maxCost);
    }   
   }
  }
 
 if (bestAlbedo) *bestAlbedo = bestAlb;
 return maxCost;
}

real32 LightDir::CalcCost(real32 albedo,ds::Array<LightDir::PixelAux> & tAux,nat32 length)
{
 LogTime("eos::fit::LightDir::CalcCost");
 
 real32 ret = 0.0;
 real32 lowRet = 0.0;
 real32 sqrAlb = math::Sqr(albedo);
 
 for (nat32 i=0;i<length;i++)
 {
  if (albedo>tAux[i].irr) ret += tAux[i].mult*math::Sqrt(sqrAlb - tAux[i].irrSqr) + tAux[i].c;
                     else lowRet += tAux[i].lowAlbCost + lowAlbErr*math::Sqr(tAux[i].irr - albedo);
 }
 
 return ret/albedo + lowRet;
}

//------------------------------------------------------------------------------
void LightDir::CostRange::CalcCost(ds::Array<LightDir::PixelAux> & tAux,nat32 length,real32 lowAlbErr)
{
 LogTime("eos::fit::LightDir::CostRange::CalcCost");
 minCost = 0.0;
 maxCost = 0.0;
 
 real32 othMinCost = 0.0; 
 real32 othMaxCost = 0.0;
 
 real32 furMinCost = 0.0; 
 real32 furMaxCost = 0.0;
 
 real32 sqrMaxA = math::Sqr(maxA);
 real32 sqrMinA = math::Sqr(minA);
 
 for (nat32 i=0;i<length;i++)
 {
  if (sqrMaxA>tAux[i].irrSqr)
  {
   real32 minBase = tAux[i].mult*math::Sqrt(sqrMaxA - tAux[i].irrSqr) + tAux[i].c;
   if (minBase<0.0) minCost += minBase;
            else othMinCost += minBase;
            
   if (sqrMinA>tAux[i].irrSqr)
   {
    real32 maxBase = tAux[i].mult*math::Sqrt(sqrMinA - tAux[i].irrSqr) + tAux[i].c;
    if (maxBase<0.0) maxCost += maxBase;
            else othMaxCost += maxBase;
   }
   else
   {
    furMaxCost += tAux[i].lowAlbCost + lowAlbErr*math::Sqr(tAux[i].irr - minA);
    // This branch is potentially wrong in certain cases, i.e. it could be larger, but this shall do.
   }
  }
  else
  {
   furMinCost += tAux[i].lowAlbCost + lowAlbErr*math::Sqr(tAux[i].irr - maxA);
   furMaxCost += tAux[i].lowAlbCost + lowAlbErr*math::Sqr(tAux[i].irr - minA);
  }
 }
 
 minCost = minCost/minA + othMinCost/maxA + furMinCost;
 maxCost = maxCost/maxA + othMaxCost/minA + furMaxCost;
}

//------------------------------------------------------------------------------
 };
};
