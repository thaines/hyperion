//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/fit/light_dir.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
real32 LightDir::SegLightCost(const bs::Normal & lightDir,nat32 recDepth,
                              const ds::Array<Pixel> & data,nat32 startInd,nat32 length,
                              ds::Array<PixelAux> & tAux,ds::PriorityQueue<CostRange> & tWork)
{
 // First fill in tAux from the data and lightDir...
  for (nat32 i=0;i<length;i++)
  {
   real32 k = data[startInd+i].dir.Length();
   real32 dot = (lightDir * data[startInd+i].dir) / k;
   
   tAux[i].mult = -k*math::Sqrt(1.0-math::Sqr(dot));
   tAux[i].irrSqr = math::Sqr(data[startInd+i].irr);
   tAux[i].c = -k*dot*data[startInd+i].irr;
  }

 // Prep the work queue with the first task...
  {
   CostRange start;
   start.minA = minAlbedo;
   start.maxA = maxAlbedo;
   start.depth = recDepth;
   start.CalcCost(tAux,length);
   
   tWork.MakeEmpty();
   tWork.Add(start);
  }
  
 // Keep processing work queue items until done...
  real32 maxCost = math::Infinity<real32>(); // Any cost higher than this value has already been exceded. 
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
 real32 ret = 0.0;
 for (nat32 i=0;i<length;i++)
 {
  ret += (tAux[i].mult*math::Sqrt(math::Max<real32>(math::Sqr(albedo) - tAux[i].irrSqr,0.0)) + tAux[i].c)
         /
         albedo;
 }
 return ret;
}

//------------------------------------------------------------------------------
void LightDir::CostRange::CalcCost(ds::Array<LightDir::PixelAux> & tAux,nat32 length)
{
 minCost = 0.0;
 maxCost = 0.0;
 
 for (nat32 i=0;i<length;i++)
 {
  real32 minBase = tAux[i].mult*math::Sqrt(math::Max<real32>(math::Sqr(maxA) - tAux[i].irrSqr,0.0)) + tAux[i].c;
  real32 maxBase = tAux[i].mult*math::Sqrt(math::Max<real32>(math::Sqr(minA) - tAux[i].irrSqr,0.0)) + tAux[i].c;
  minCost += (minBase<0.0)?(minBase/minA):(minBase/maxA);
  maxCost += (maxBase<0.0)?(maxBase/maxA):(maxBase/minA);
 }
}

//------------------------------------------------------------------------------
 };
};
