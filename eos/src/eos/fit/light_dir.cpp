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
:minAlbedo(0.001),maxAlbedo(3.0),maxSegCost(1.0),subdiv(4),recDepth(8),
bestLightDir(0.0,0.0,1.0)
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
void LightDir::SetSegCap(real32 maxCost)
{
 maxSegCost = maxCost;
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
  nat32 steps = 5 + segCount + 1;
  
   
 // First generate the sampling set of light source directions to sample...
  prog->Report(step++,steps);
  alg::HemiOfNorm hon(subdiv);
  lc.Size(hon.Norms());
  for (nat32 i=0;i<lc.Size();i++) lc[i].dir = hon.Norm(i);



 // Now generate the data structure we actually pass around from the input - 
 // group data points by segment...
  prog->Report(step++,steps);
  ds::Array<Pixel> pixel(seg.Size(0)*seg.Size(1)); // Array of all pixels, grouped by segment.
  ds::Array<nat32> size(segCount); // Number of pixels in each segment.
  ds::Array<nat32> offset(segCount+1); // Array of offsets for each segment, with size on end.
  
  // Count pixels in each segment, setup offset structure...
   prog->Report(step++,steps);
   for (nat32 i=0;i<size.Size();i++) size[i] = 0;
   for (nat32 y=0;y<seg.Size(1);y++)
   {
    for (nat32 x=0;x<seg.Size(0);x++) size[seg.Get(x,y)] += 1;
   }
   
   offset[0] = 0;
   for (nat32 i=0;i<size.Size();i++) offset[i+1] = offset[i] + size[i];
   
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
     nat32 s = seg.Get(x,y);
     Pixel & targ = pixel[offset[s]+size[s]];
     size[s] += 1;
     
     targ.irr = irr.Get(x,y);
     targ.dir = dir.Get(x,y);
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
   prog->Push();
   for (nat32 l=0;l<lc.Size();l++)
   {
    prog->Report(l,lc.Size());
    segCost[l] = SegLightCost(lc[l].dir,recDepth, pixel,offset[s],offset[s+1]-offset[s], 
                              tAux,tWork);
   }
   prog->Pop();
   
   real32 minLightCost = math::Infinity<real32>();
   for (nat32 l=0;l<lc.Size();l++) minLightCost = math::Min(minLightCost,segCost[l]);

   for (nat32 l=0;l<lc.Size();l++) lc[l].cost += math::Min(segCost[l]-minLightCost,maxSegCost);
  }

 
 // Find the lowest costed light source direction and store as the best...
  prog->Report(step++,steps);
  nat32 best = 0;
  for (nat32 l=1;l<lc.Size();l++)
  {
   if (lc[l].cost<lc[best].cost) best = l;
  }
  bestLightDir = lc[best].dir;
 
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
                              ds::Array<PixelAux> & tAux,ds::PriorityQueue<CostRange> & tWork)
{
 if (length==0) return math::Infinity<real32>();
 LogTime("eos::fit::LightDir::SegLightCost");

 // First fill in tAux from the data and lightDir...
  for (nat32 i=0;i<length;i++)
  {
   real32 k = data[startInd+i].dir.Length();
   real32 dot = (lightDir * data[startInd+i].dir) / k;
   if (!math::IsFinite(dot)) dot = 0.0;
   
   tAux[i].mult = -k*math::Sqrt(1.0-math::Sqr(dot));
   tAux[i].irrSqr = math::Sqr(data[startInd+i].irr);
   tAux[i].c = -k*dot*data[startInd+i].irr;
   
   //LogDebug("{i,k,dot,mult,irrSqr,c}" << LogDiv() << i << LogDiv() << k << LogDiv() << dot << LogDiv() << tAux[i].mult << LogDiv() << tAux[i].irrSqr << LogDiv() << tAux[i].c);
  }

  
 // Before doing the actual work queue stuff we do a single depth first greedy search -
 // this will usually raise the bar for processing other jobs high enough to avoid a load
 // of work - we push the excess off into the work queue...
  tWork.MakeEmpty();
  real32 maxCost = math::Infinity<real32>(); // Any cost higher than this value has already been beaten.
  {
   CostRange targ;
   targ.minA = minAlbedo;
   targ.maxA = maxAlbedo;
   targ.depth = recDepth;
   targ.CalcCost(tAux,length);
  
   while(targ.depth!=0)
   {
    real32 half = 0.5*(targ.minA+targ.maxA);
    
    CostRange low;
    low.minA = targ.minA;
    low.maxA = half;
    low.depth = targ.depth - 1;
    low.CalcCost(tAux,length);
    
    CostRange high;
    high.minA = half;
    high.maxA = targ.maxA;
    high.depth = targ.depth - 1;
    high.CalcCost(tAux,length);
    
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
     maxCost = math::Min(maxCost,cost);
   }
   else
   {
    CostRange low;
    low.minA = targ.minA;
    low.maxA = half;
    low.depth = targ.depth - 1;
    low.CalcCost(tAux,length);
    
    if (low.minCost<maxCost)
    {
     tWork.Add(low);
     maxCost = math::Min(maxCost,low.maxCost);
    }
    
    CostRange high;
    high.minA = half;
    high.maxA = targ.maxA;
    high.depth = targ.depth - 1;
    high.CalcCost(tAux,length);   
   
    if (high.minCost<maxCost)
    {
     tWork.Add(high);
     maxCost = math::Min(maxCost,high.maxCost);
    }   
   }
  }
 
 return maxCost;
}

real32 LightDir::CalcCost(real32 albedo,ds::Array<LightDir::PixelAux> & tAux,nat32 length)
{
 LogTime("eos::fit::LightDir::CalcCost");
 real32 ret = 0.0;
 real32 sqrAlb = math::Sqr(albedo);
 for (nat32 i=0;i<length;i++)
 {
  ret += tAux[i].mult*math::Sqrt(math::Max<real32>(sqrAlb - tAux[i].irrSqr,0.0)) + tAux[i].c;
 }
 return ret/albedo;
}

//------------------------------------------------------------------------------
void LightDir::CostRange::CalcCost(ds::Array<LightDir::PixelAux> & tAux,nat32 length)
{
 LogTime("eos::fit::LightDir::CostRange::CalcCost");
 minCost = 0.0;
 maxCost = 0.0;
 
 real32 othMinCost = 0.0;
 real32 othMaxCost = 0.0;
 
 real32 sqrMaxA = math::Sqr(maxA);
 real32 sqrMinA = math::Sqr(minA);
 
 for (nat32 i=0;i<length;i++)
 {  
  real32 minBase = tAux[i].mult*math::Sqrt(math::Max<real32>(sqrMaxA - tAux[i].irrSqr,0.0)) + tAux[i].c;
  real32 maxBase = tAux[i].mult*math::Sqrt(math::Max<real32>(sqrMinA - tAux[i].irrSqr,0.0)) + tAux[i].c;
  if (minBase<0.0) minCost += minBase;
           else othMinCost += minBase;
  if (maxBase<0.0) maxCost += minBase;
           else othMaxCost += minBase;  
 }
 
 minCost = minCost/minA + othMinCost/maxA;
 maxCost = maxCost/maxA + othMaxCost/minA;
}

//------------------------------------------------------------------------------
 };
};
