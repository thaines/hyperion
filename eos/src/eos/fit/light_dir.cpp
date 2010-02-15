//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/fit/light_dir.h"

#include "eos/alg/shapes.h"
#include "eos/fit/sphere_sample.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
LightDir::LightDir()
:minAlbedo(0.001),maxAlbedo(1.5),ambient(0.0),maxSegCostPP(0.1),lowAlbErr(8192.0),
segPruneThresh(0.1),subdiv(1),furtherSubdiv(3),recDepth(7),bestLightDir(0.0,0.0,1.0)
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

void LightDir::SetAmbient(real32 a)
{
 ambient = a;
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

void LightDir::SetSampleSubdiv(nat32 sd,nat32 fsd)
{
 subdiv = sd;
 furtherSubdiv = fsd;
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


 // Generate the sampling set of light source directions to sample, for the 
 // initial pass...
  fit::SubDivSphere sds;
  sds.DivideAll(subdiv);
  
  nat32 lightCount = 0;
  for (nat32 i=0;i<sds.VertCount();i++)
  {
   if (!(sds.Vert(i)[2]<0.0)) lightCount += 1;
  }
  lc.Size(lightCount);
  
  lightCount = 0;
  for (nat32 i=0;i<sds.VertCount();i++)
  {
   if (!(sds.Vert(i)[2]<0.0))
   {
    lc[lightCount].dir = sds.Vert(i);
    lc[lightCount].cost = 0.0;
    lc[lightCount].index = i;
    lightCount += 1;
   }
  }


 // Prep for logging...
  nat32 step = 0;
  nat32 steps = 5 + segCount*lightCount + furtherSubdiv*6*3*segCount + 1 + segCount;


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
     if ((!math::IsZero(irr.Get(x,y)))&&(!math::IsZero(dir.Get(x,y).Length())))
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
     if ((!math::IsZero(irr.Get(x,y)))&&(!math::IsZero(dir.Get(x,y).Length())))
     {
      nat32 s = seg.Get(x,y);
      Pixel & targ = pixel[offset[s]+size[s]];
      size[s] += 1;
     
      targ.irr = irr.Get(x,y);
      targ.dir = dir.Get(x,y);
     }
    }
   }



 // Now iterate the light sources and segments and sum up the costs, for the 
 // initial sampling...
  prog->Report(step++,steps);
  ds::Array<real32> segCost(lc.Size());
  ds::Array<real32> minSegCost(segCount);
  ds::Array<PixelAux> tAux(maxSegSize);
  ds::PriorityQueue<CostRange> tWork(recDepth*4);
 
  for (nat32 s=0;s<segCount;s++)
  {
   nat32 segSize = offset[s+1] - offset[s];
   if ((segSize!=0)&&(cor[s]>segPruneThresh))
   {
    for (nat32 l=0;l<lc.Size();l++)
    {
     prog->Report(step++,steps);
     segCost[l] = SegLightCost(lc[l].dir,recDepth,pixel,offset[s],segSize, 
                              tAux,tWork);
    }
   
    minSegCost[s] = math::Infinity<real32>();
    for (nat32 l=0;l<lc.Size();l++) minSegCost[s] = math::Min(minSegCost[s],segCost[l]);

    for (nat32 l=0;l<lc.Size();l++) lc[l].cost += math::Min(segCost[l]-minSegCost[s],maxSegCostPP*segSize);
   }
   else
   {
    step += lc.Size();
   }
  }
  
  
 // Do the refinement passes...
  for (nat32 r=0;r<furtherSubdiv;r++)
  {
   // Find the index of the lowest costed direction...
    nat32 best = 0;
    for (nat32 l=1;l<lc.Size();l++)
    {
     if (lc[l].cost<lc[best].cost) best = l;
    }
    
   // Find all triangles that use the lowest costed direction but don't have
   // children...
    ds::ArrayResize<fit::SubDivSphere::Tri> tris;
    for (nat32 t=0;t<sds.TriCount();t++)
    {
     if ((sds.HasChildren(t)==false)&&((sds.IndA(t)==best)||(sds.IndB(t)==best)||(sds.IndC(t)==best)))
     {
      nat32 ind = tris.Size();
      tris.Size(ind+1);
      tris[ind] = t;
     }
    }
   
    steps = int32(steps) + (int32(tris.Size())-6)*3*int32(segCount);
   
   // Iterate the triangles, subdivide, and make the new samples...
    for (nat32 t=0;t<tris.Size();t++)
    {
     sds.SubDivide(tris[t]);
     fit::SubDivSphere::Tri targ = sds.GetM(tris[t]);
     
     for (nat32 v=0;v<3;v++)
     {
      // Basic setup of storage for the new light cost...
       nat32 ind = lc.Size();
       lc.Size(ind+1);
       lc[ind].dir = sds.Vert(targ,v);
       lc[ind].cost = 0.0;
       lc[ind].index = sds.Ind(targ,v);
       
      // Iterate the segments - cost 'em all and sum it in...
       for (nat32 s=0;s<segCount;s++)
       {
        prog->Report(step++,steps);
        nat32 segSize = offset[s+1] - offset[s];
        if ((segSize!=0)&&(cor[s]>segPruneThresh))
        {
         real32 cost = SegLightCost(lc[ind].dir,recDepth,pixel,offset[s],segSize,tAux,tWork);
         lc[ind].cost += math::Min(cost-minSegCost[s],maxSegCostPP*segSize);
        }
       }
     }
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
  albedo.Size(segCount);
  for (nat32 s=0;s<segCount;s++)
  {
   prog->Report(step++,steps);
   if (offset[s+1]!=offset[s])
   {
    SegLightCost(bestLightDir,recDepth, pixel,offset[s],offset[s+1]-offset[s], 
                 tAux,tWork,&albedo[s]);
   }
   else albedo[s] = 0.0;
  }


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

   tAux[i].s = -k*dot;
   tAux[i].t = -k*math::Sqrt(1.0-math::Sqr(dot));
   tAux[i].irr = data[startInd+i].irr;
   tAux[i].minR = math::Abs(tAux[i].s)/math::Sqrt(math::Sqr(tAux[i].s)+math::Sqr(tAux[i].t));
   tAux[i].minC = tAux[i].s*tAux[i].minR + tAux[i].t*math::Sqrt(1.0-math::Sqr(tAux[i].minR));
   
   //LogDebug("seg {i,k,dot,s,t,irr,minR,minC}" << LogDiv() << i << LogDiv() << k << LogDiv() << dot << LogDiv() << tAux[i].s << LogDiv() << tAux[i].t << LogDiv() << tAux[i].irr << LogDiv() << tAux[i].minR << LogDiv() << tAux[i].minC);
  }


  
 // Before doing the actual work queue stuff we do a single depth first greedy search -
 // this will usually raise the bar for processing other jobs high enough to avoid a load
 // of work - we push the excess off into the work queue...
  tWork.MakeEmpty();
  real32 maxCost = math::Infinity<real32>(); // Any cost higher than this value has already been beaten.
  real32 bestAlb = 0.0;
  {
   CostRange targ;
   targ.minAlbedo = minAlbedo;
   targ.maxAlbedo = maxAlbedo;
   targ.depth = recDepth;
   targ.CalcCost(tAux,length,ambient,lowAlbErr);
  
   while(targ.depth!=0)
   {
    real32 half = 0.5*(targ.minAlbedo+targ.maxAlbedo);
    
    CostRange low;
    low.minAlbedo = targ.minAlbedo;
    low.maxAlbedo = half;
    low.depth = targ.depth - 1;
    low.CalcCost(tAux,length,ambient,lowAlbErr);
    
    CostRange high;
    high.minAlbedo = half;
    high.maxAlbedo = targ.maxAlbedo;
    high.depth = targ.depth - 1;
    high.CalcCost(tAux,length,ambient,lowAlbErr);
    
    maxCost = math::Min(maxCost,low.highMinCost,high.highMinCost);
    
    if (low.lowMinCost<high.lowMinCost)
    {
     targ = low;
     if (high.lowMinCost<maxCost) tWork.Add(high);
    }
    else
    {
     targ = high;
     if (low.lowMinCost<maxCost) tWork.Add(low);   
    }
   }
   
   real32 half = 0.5*(targ.minAlbedo+targ.maxAlbedo);
   real32 cost = CalcCost(half,tAux,length);
   maxCost = math::Min(maxCost,cost);
   bestAlb = half;
  }


 // Keep processing work queue items until done...
  while ((tWork.Size()!=0)&&(tWork.Peek().lowMinCost<maxCost))
  {
   CostRange targ = tWork.Peek();
   tWork.Rem();
   
   real32 half = 0.5*(targ.minAlbedo+targ.maxAlbedo);
   
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
    low.minAlbedo = targ.minAlbedo;
    low.maxAlbedo = half;
    low.depth = targ.depth - 1;
    low.CalcCost(tAux,length,ambient,lowAlbErr);
    
    if (low.lowMinCost<maxCost)
    {
     tWork.Add(low);
     maxCost = math::Min(maxCost,low.highMinCost);
    }
    
    CostRange high;
    high.minAlbedo = half;
    high.maxAlbedo = targ.maxAlbedo;
    high.depth = targ.depth - 1;
    high.CalcCost(tAux,length,ambient,lowAlbErr);   
   
    if (high.lowMinCost<maxCost)
    {
     tWork.Add(high);
     maxCost = math::Min(maxCost,high.highMinCost);
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
 
 for (nat32 i=0;i<length;i++)
 {
  real32 r = (tAux[i].irr/albedo) - ambient;
  
  ret += (r<0.0)?(tAux[i].t + lowAlbErr*(albedo*ambient - tAux[i].irr)):
         ((r>1.0)?(tAux[i].s + lowAlbErr*(tAux[i].irr - albedo*(1.0+ambient))):
         (tAux[i].s*r + tAux[i].t*math::Sqrt(1.0 - math::Sqr(r))));
 }
 
 return ret;
}

//------------------------------------------------------------------------------
void LightDir::CostRange::CalcCost(ds::Array<LightDir::PixelAux> & tAux,nat32 length,
                                   real32 ambient,real32 lowAlbErr)
{
 LogTime("eos::fit::LightDir::CostRange::CalcCost");

 lowMinCost = 0.0;
 highMinCost = 0.0; 
 
 for (nat32 i=0;i<length;i++)
 {
  real32 lowR = (tAux[i].irr/maxAlbedo) - ambient;
  real32 highR = (tAux[i].irr/minAlbedo) - ambient;
  
  // Calculate the cost for lowR and the cost for highR...
   // low...
    real32 lowC = (lowR<0.0)?(tAux[i].t + lowAlbErr*(maxAlbedo*ambient - tAux[i].irr)):
                  ((lowR>1.0)?(tAux[i].s + lowAlbErr*(tAux[i].irr - maxAlbedo*(1.0+ambient))):
                  (tAux[i].s*lowR + tAux[i].t*math::Sqrt(1.0 - math::Sqr(lowR))));

   // high...
    real32 highC = (highR<0.0)?(tAux[i].t + lowAlbErr*(minAlbedo*ambient - tAux[i].irr)):
                   ((highR>1.0)?(tAux[i].s + lowAlbErr*(tAux[i].irr - minAlbedo*(1.0+ambient))):
                   (tAux[i].s*highR + tAux[i].t*math::Sqrt(1.0 - math::Sqr(highR))));
   
  // If the minima is inside the range then we use the minimum, otherwise we use
  // the two bounds already calculated...
   if ((highR<tAux[i].minR)||(lowR>tAux[i].minR))
   {
    lowMinCost += math::Min(lowC,highC);
   }
   else lowMinCost += tAux[i].minC;
   
  // Maximum cost...
   highMinCost += math::Max(lowC,highC);
 }
}

//------------------------------------------------------------------------------
 };
};
