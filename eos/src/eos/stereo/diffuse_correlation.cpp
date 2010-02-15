//------------------------------------------------------------------------------
// Copyright 2009 Tom Haines
#include "eos/stereo/diffuse_correlation.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
DiffusionWeight::DiffusionWeight()
{}
  
DiffusionWeight::~DiffusionWeight()
{}

void DiffusionWeight::Create(const bs::LuvRangeImage & img, const bs::LuvRangeDist & dist, real32 distMult, time::Progress * prog)
{
 prog->Push();
 
 data.Resize(img.Width(),img.Height());
 for (nat32 y=0;y<img.Height();y++)
 {
  prog->Report(y,img.Height());
  for (nat32 x=0;x<img.Width();x++)
  {
   if (img.Valid(x,y))
   {
    Weight & weight = data.Get(x,y);

    // +ve x...
     if ((x+1<img.Width())&&(img.Valid(x+1,y))) weight.dir[0] = dist(img.Get(x,y),img.Get(x+1,y));
                                           else weight.dir[0] = math::Infinity<real32>();

    // +ve y...
     if ((y+1<img.Height())&&(img.Valid(x,y+1))) weight.dir[1] = dist(img.Get(x,y),img.Get(x,y+1));
                                            else weight.dir[1] = math::Infinity<real32>();

    // -ve x...
     if ((x>0)&&(img.Valid(x-1,y))) weight.dir[2] = dist(img.Get(x,y),img.Get(x-1,y));
                               else weight.dir[2] = math::Infinity<real32>();

    // -ve y...
     if ((y>0)&&(img.Valid(x,y-1))) weight.dir[3] = dist(img.Get(x,y),img.Get(x,y-1));
                               else weight.dir[3] = math::Infinity<real32>();

    // Convert and normalise...
     real32 low = math::Infinity<real32>();
     for (nat32 d=0;d<4;d++) low = math::Min(low,weight.dir[d]);
     
     real32 weightSum = 0.0;
     for (nat32 d=0;d<4;d++)
     {
      if (math::IsFinite(weight.dir[d]))
      {
       weight.dir[d] = -math::Exp(distMult * (weight.dir[d]-low));
       weightSum += weight.dir[d];
      }
      else weight.dir[d] = 0.0;
     }
     
     if (!math::IsZero(weightSum))
     {
      for (nat32 d=0;d<4;d++) weight.dir[d] /= weightSum;
     }
   }
   else
   {
    for (nat32 d=0;d<4;d++) data.Get(x,y).dir[d] = 0.0;
   }
  }
 }
 
 prog->Pop();
}

//------------------------------------------------------------------------------
RangeDiffusionSlice::RangeDiffusionSlice()
:steps(0)
{}

RangeDiffusionSlice::~RangeDiffusionSlice()
{}

void RangeDiffusionSlice::Create(nat32 yy, nat32 stps, const bs::LuvRangeImage & img, const DiffusionWeight & dw, time::Progress * prog)
{
 prog->Push();
 
 // First we need to calculate the offset array, and how large we need the
 // result storage to be...
  prog->Report(0,img.Width()+1);
  y = yy;
  steps = stps;
  offset.Resize(steps*2+1,steps*2+1);
  nat32 valueCount = 0;
  for (int32 v=0;v<int32(offset.Height());v++)
  {
   for (int32 u=0;u<int32(offset.Width());u++)
   {
    if (math::Abs(u-int32(steps)) + math::Abs(v-int32(steps)) <= int32(steps))
    {
     offset.Get(u,v) = valueCount;
     valueCount += 1;
    }
   }
  }
 
 // We need some tempory arrays for doing the diffusion in...
  ds::Array2D<real32> bufA(offset.Width(),offset.Height());
  ds::Array2D<real32> bufB(offset.Width(),offset.Height());

 // Helpful little arrays...
  int32 du[4] = {1,0,-1, 0};
  int32 dv[4] = {0,1, 0,-1};


 // Now iterate the pixels and calculate and store the diffusion weights for
 // each...
  if ((data.Width()!=img.Width())||(valueCount!=data.Height()))
  {
   data.Resize(img.Width(),valueCount);
  }
  
  for (nat32 x=0;x<img.Width();x++)
  {
   prog->Report(x+1,img.Width()+1);
   
   if (!img.Valid(x,y))
   {
    for (nat32 v=0;v<valueCount;v++)  data.Get(x,v) = 0.0;
    continue;
   }
   
   // Do the diffusion - we have to bounce between two buffers; when zeroing
   // the old buffer only clear the range to be used next time...
    ds::Array2D<real32> * from = &bufA;
    ds::Array2D<real32> * to = &bufB;
    
    from->Get(steps,steps) = 1.0;
    
    for (nat32 s=0;s<steps;s++)
    {
     // Zero out relevant range of to buffer...
      for (int32 v=-int32(s)-1;v<=int32(s)+1;v++)
      {
       for (int32 u=-int32(s)-1;u<=int32(s)+1;u++) to->Get(u+steps,v+steps) = 0.0;
      }
      
     // Do diffusion step...
      for (int32 v=-int32(s);v<=int32(s);v++)
      {
       for (int32 u=-int32(s);u<=int32(s);u++)
       {
        int32 ax = int32(x) + u;
        int32 ay = int32(y) + v;
        
        if (((math::Abs(v)+math::Abs(u))<=int32(s))&&
            (ax>=0)&&(ay>=0)&&(ax<int32(img.Width()))&&(ay<int32(img.Height())))
        {
         real32 val = from->Get(u+steps,v+steps);
         for (nat32 d=0;d<4;d++)
         {
          to->Get(u+steps+du[d],v+steps+dv[d]) += val * dw.Get(ax,ay,d);
         }
        }
       }
      }
     
     // Swap buffers...
      math::Swap(from,to);
    }


   // Copy result into storage - result is in from...
    for (nat32 v=0;v<from->Height();v++)
    {
     for (nat32 u=0;u<from->Width();u++)
     {
      int32 ru = int32(u) - int32(steps);
      int32 rv = int32(v) - int32(steps);
      if (math::Abs(ru) + math::Abs(rv) <= int32(steps))
      {
       data.Get(x,offset.Get(u,v)) = from->Get(u,v);
      }
     }
    }
    
   // Normalise - it should be already, but numerical error will creep in...
    real32 sum = 0.0;
    for (nat32 v=0;v<data.Height();v++) sum += data.Get(x,v);
    
    if (sum>0.5) // sum could be zero in the case of a pixel surrounded by masked pixels.
    {
     for (nat32 v=0;v<data.Height();v++) data.Get(x,v) /= sum;
    }
  }

 
 prog->Pop();
}

nat32 RangeDiffusionSlice::Width() const
{
 return data.Width();
}

nat32 RangeDiffusionSlice::Steps() const
{
 return steps;
}

real32 RangeDiffusionSlice::Get(nat32 x,int32 u,int32 v) const
{
 nat32 man = math::Abs(u) + math::Abs(v);
 if (man>steps) return 0.0;
 
 nat32 os = offset.Get(u+int32(steps),v+int32(steps));
 return data.Get(x,os);
}

//------------------------------------------------------------------------------
DiffuseCorrelation::DiffuseCorrelation()
:dist(null<bs::LuvRangeDist*>()),img1(null<bs::LuvRangeImage*>()),dif1(null<RangeDiffusionSlice*>()),img2(null<bs::LuvRangeImage*>()),dif2(null<RangeDiffusionSlice*>())
{}

DiffuseCorrelation::~DiffuseCorrelation()
{}

void DiffuseCorrelation::Setup(const bs::LuvRangeDist & d, real32 dc, const bs::LuvRangeImage & i1, const RangeDiffusionSlice & f1, const bs::LuvRangeImage & i2, const RangeDiffusionSlice & f2)
{
 log::Assert(f1.Steps()==f2.Steps());

 dist = &d;
 distCap = dc;
 
 img1 = &i1;
 dif1 = &f1;
 img2 = &i2;
 dif2 = &f2;
}

nat32 DiffuseCorrelation::Width1() const
{
 return img1->Width();
}

nat32 DiffuseCorrelation::Width2() const
{
 return img2->Width();
}

real32 DiffuseCorrelation::Cost(nat32 xx1,nat32 xx2) const
{
 real32 ret = 0.0;
 
 int32 steps = dif1->Steps();
 for (int32 v=-steps;v<=steps;v++)
 {
  for (int32 u=-steps;u<=steps;u++)
  {
   int32 x1 = int32(xx1) + u;
   int32 y1 = int32(dif1->Y()) + v;
   int32 x2 = int32(xx2) + u;
   int32 y2 = int32(dif2->Y()) + v;
   
   real32 weight = dif1->Get(xx1,u,v) + dif2->Get(xx2,u,v);
   
   if (img1->ValidExt(x1,y1)&&img2->ValidExt(x2,y2))
   {
    ret += weight * math::Min((*dist)(img1->Get(x1,y1),img2->Get(x2,y2)),distCap);
   }
   else
   {
    ret += weight * distCap;
   }
  }
 }

 return ret/2.0;
}

real32 DiffuseCorrelation::DistanceCap() const
{
 return distCap;
}

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
 };
};
