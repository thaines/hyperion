//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/plane_seg.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
PlaneSeg::PlaneSeg()
{}

PlaneSeg::~PlaneSeg()
{}

void PlaneSeg::SetDisparity(const svt::Field<real32> & d)
{
 disp = d;
}

void PlaneSeg::SetValidity(const svt::Field<bit> & v)
{
 valid = v;
}

void PlaneSeg::SetSegs(nat32 segCount,const svt::Field<nat32> & s)
{
 sed.Size(segCount);
 segs = s;
}

void PlaneSeg::MakePlanes()
{
 // Create and initialise data...
  ds::Array<SedInt> sedInt(sed.Size());
  for (nat32 i=0;i<sed.Size();i++)
  {
   sed[i].x = 0.0;
   sed[i].y = 0.0;
   sed[i].pix = 0;
   sed[i].area = 0;
   sedInt[i].planeFit.Reset();
   sedInt[i].done = false;
  }

 // Iterate and improve the current fit for all data...
  bit firstPass = true;
  while (true)
  {
   // Iterate all valid pixels and store the values...
    for (nat32 y=0;y<disp.Size(1);y++)
    {
     for (nat32 x=0;x<disp.Size(0);x++)
     {
      if (valid.Get(x,y))
      {
       nat32 s = segs.Get(x,y);       
       if (firstPass)
       {
        sed[s].x += x;
        sed[s].y += y;
        sed[s].pix += 1;
        sed[s].area += 1;
        sedInt[s].planeFit.Add(x,y,disp.Get(x,y));
       }
       else
       {
        if (!sedInt[s].done)
        {
         real32 d = disp.Get(x,y);
         if (math::Abs(d-sed[s].plane.Z(x,y))<=outlier)
         {
          sedInt[s].planeFit.Add(x,y,d);
         }
        }
       }       
      }
      else if (firstPass)
      {
       sed[segs.Get(x,y)].area += 1;       
      }
     }
    }

   // Iterate the segments and integrate the data collected, reset for 
   // next iteration and break if no more work is required...
    bit timeToDie = !firstPass;
    for (nat32 s=0;s<sed.Size();s++)
    {
     if (firstPass)
     {
      if (sedInt[s].planeFit.Valid())
      {
       sed[s].x /= sed[s].pix;
       sed[s].y /= sed[s].pix;
       sedInt[s].planeFit.Get(sed[s].plane);
      }
      else
      {
       sed[s].plane.a = 0.0;
       sed[s].plane.b = 0.0;
       sed[s].plane.c = 0.0;
       sedInt[s].done = true;
      }
     }
     else
     {
      if (sedInt[s].done==false)
      {
       if (sedInt[s].planeFit.Valid())
       {
        bs::PlaneABC np;
        sedInt[s].planeFit.Get(np);
        real32 diff = math::Sqr(np.a-sed[s].plane.a) + math::Sqr(np.b-sed[s].plane.b) + math::Sqr(np.c-sed[s].plane.c);
        sed[s].plane = np;
        if (diff<=convergence) sedInt[s].done = true;
        else
        {
         timeToDie = false;
         sedInt[s].planeFit.Reset();
        }
       }
       else sedInt[s].done = true;
      }
     }
    }
    if (timeToDie) break;
    firstPass = false;
  }
  
 // We now have a problem - various segments will have not had planes fitted, simply because
 // they lacked enough information.
}

void PlaneSeg::Extract(svt::Field<real32> & d)
{
 for (nat32 y=0;y<disp.Size(1);y++)
 {
  for (nat32 x=0;x<disp.Size(0);x++)
  {
   d.Get(x,y) = sed[segs.Get(x,y)].plane.Z(x,y);
  }
 }
}

void PlaneSeg::GetPlane(nat32 seg,bs::PlaneABC & out)
{
 out = sed[seg].plane;
}

void PlaneSeg::SetPlane(nat32 seg,const bs::PlaneABC & in)
{
 sed[seg].plane = in;
}

void PlaneSeg::GetCenter(nat32 seg,real32 & outX,real32 & outY)
{
 outX = sed[seg].x;
 outY = sed[seg].y;
}

void PlaneSeg::GetArea(nat32 seg,nat32 & out)
{
 out = sed[seg].area;
}

void PlaneSeg::PrepLayerSeg(class LayerSeg & out)
{
 out.SetDisparity(disp);
 out.SetValidity(valid);
 out.SetSegs(sed.Size(),segs);
}

//------------------------------------------------------------------------------
LayerSeg::LayerSeg()
{}

LayerSeg::~LayerSeg()
{}

void LayerSeg::SetDisparity(svt::Field<real32> & d)
{
 disp = d;
}

void LayerSeg::SetValidity(svt::Field<bit> & v)
{
 valid = v;
}

void LayerSeg::SetSegs(nat32 segCount, svt::Field<nat32> & s)
{
 gas.Size(segCount);
 for (nat32 i=0;i<gas.Size();i++) gas[i] = false;
 segs = s;
}

void LayerSeg::AddSegment(nat32 seg)
{
 gas[seg] = true;
}

void LayerSeg::RemSegment(nat32 seg)
{
 gas[seg] = false;
}

void LayerSeg::Run()
{
 // Do a first pass to fit the initial plane...
  alg::PlaneFit planeFit;
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    nat32 s = segs.Get(x,y);
    if (gas[s]&&valid.Get(x,y))
    {
     planeFit.Add(x,y,disp.Get(x,y));
    }
   }
  }
  if (planeFit.Valid()) planeFit.Get(plane);
  else
  {
   plane.a = 0.0;
   plane.b = 0.0;
   plane.c = 0.0;
   return;
  }


 // Do further passes to refine it, losing outliers...
  while (true)
  {
   // Fit again, excluding outliers...
    planeFit.Reset();
    for (nat32 y=0;y<disp.Size(1);y++)
    {
     for (nat32 x=0;x<disp.Size(0);x++)
     {
      nat32 s = segs.Get(x,y);
      if (gas[s]&&valid.Get(x,y))
      {
       real32 d = disp.Get(x,y);
       if (math::Abs(d-plane.Z(x,y))<=outlier) planeFit.Add(x,y,d);
      }
     }
    }

   // Update the plane...
    if (!planeFit.Valid()) break;
     bs::PlaneABC tp;
     planeFit.Get(tp);
     real32 diff = math::Sqr(tp.a-plane.a) + math::Sqr(tp.b-plane.b) + math::Sqr(tp.c-plane.c);
     plane = tp;
     if (diff<=convergence) break;
  }
}

void LayerSeg::Result(bs::PlaneABC & p)
{
 p = plane;
}

//------------------------------------------------------------------------------
 };
};
