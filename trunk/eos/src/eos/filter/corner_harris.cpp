//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/filter/corner_harris.h"

#include "eos/math/functions.h"
#include "eos/math/interpolation.h"
#include "eos/filter/kernel.h"
#include "eos/ds/priority_queues.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC void CornerHarris(const svt::Field<real32> & img,nat32 maxSize,ds::Array<Corner> & out,time::Progress * prog)
{
 LogBlock("eos::filter::CornerHarris","-");
 prog->Push();
  // Create tempory data structures...
   prog->Report(0,6);
   svt::Var tmpVar(img);
    real32 realIni = 0.0;
    tmpVar.Add("dxdx",realIni);
    tmpVar.Add("dydy",realIni);
    tmpVar.Add("dxdy",realIni);
    tmpVar.Add("corner",realIni);
   tmpVar.Commit();
   
   svt::Field<real32> dxdx(&tmpVar,"dxdx");
   svt::Field<real32> dydy(&tmpVar,"dydy");
   svt::Field<real32> dxdy(&tmpVar,"dxdy");
   svt::Field<real32> corner(&tmpVar,"corner");


  // Create dx^2,dy^2 and dx*dy fields (Uses central difference)...
   prog->Report(1,6);
   for (nat32 y=1;y<img.Size(1)-1;y++)
   {
    for (nat32 x=1;x<img.Size(0)-1;x++)
    {
     real32 dx = 0.5*(img.Get(x+1,y)-img.Get(x-1,y));
     real32 dy = 0.5*(img.Get(x,y+1)-img.Get(x,y-1));
     
     dxdx.Get(x,y) = math::Sqr(dx);
     dydy.Get(x,y) = math::Sqr(dy);
     dxdy.Get(x,y) = dx*dy;
    }
   }


  // Apply a gaussian blur to each of the 3 fields...
   prog->Report(2,6);
   KernelVect kernel(2);
   kernel.MakeGaussian(0.7);
   
   kernel.Apply(dxdx,dxdx);
   kernel.Apply(dydy,dydy);
   kernel.Apply(dxdy,dxdy);


  // Calculate all the corner responses...
   prog->Report(3,6);
   for (nat32 y=2;y<img.Size(1)-2;y++)
   {
    for (nat32 x=2;x<img.Size(0)-2;x++)
    {
     corner.Get(x,y) = dxdx.Get(x,y)*dydy.Get(x,y) - math::Sqr(dxdy.Get(x,y)) - 
                       0.04*math::Sqr(dxdx.Get(x,y) + dydy.Get(x,y));
    }
   }


  // Find the maximas and store them in a heap...
   prog->Report(4,6);
   ds::PriorityQueue<Corner> heap(math::Max((img.Size(0)*img.Size(1))/50,maxSize));
   for (nat32 y=3;y<img.Size(1)-3;y++)
   {
    for (nat32 x=3;x<img.Size(0)-3;x++)
    {
     if ((corner.Get(x,y)>corner.Get(x+1,y))&&
         (corner.Get(x,y)>corner.Get(x-1,y))&&
         (corner.Get(x,y)>corner.Get(x,y+1))&&
         (corner.Get(x,y)>corner.Get(x,y-1))&&
         (corner.Get(x,y)>corner.Get(x-1,y+1))&&
         (corner.Get(x,y)>corner.Get(x-1,y-1))&&
         (corner.Get(x,y)>corner.Get(x+1,y+1))&&
         (corner.Get(x,y)>corner.Get(x+1,y-1)))
     {
      Corner c;
       c.loc[0] = x;
       c.loc[1] = y;
       c.strength = corner.Get(x,y);
      heap.Add(c);
     }
    }
   }
 
  // Pop off maxSize items from the heap to generate the out array...
   prog->Report(5,6);
   out.Size(math::Min(heap.Size(),maxSize));
   for (nat32 i=0;i<out.Size();i++)
   {
    out[i] = heap.Peek();
    heap.Rem();   
   }
 
 prog->Pop();
}

//------------------------------------------------------------------------------
HarrisSnap::HarrisSnap()
:falloff(0.75)
{}

HarrisSnap::~HarrisSnap()
{}

void HarrisSnap::Set(const svt::Field<real32> & im)
{
 img = im;
}

void HarrisSnap::Set(real32 fo)
{
 falloff = fo;        
}

void HarrisSnap::Run(time::Progress * prog)
{
 prog->Push();
 
 snap.Resize(img.Size(0),img.Size(1));


 // First calculate the harris response function for every pixel...
  // Create tempory data structures...
   svt::Var tmpVar(img);
    real32 realIni = 0.0;
    tmpVar.Add("dxdx",realIni);
    tmpVar.Add("dydy",realIni);
    tmpVar.Add("dxdy",realIni);
    tmpVar.Add("corner",realIni);
    tmpVar.Add("weight",realIni);
   tmpVar.Commit();
   
   svt::Field<real32> dxdx(&tmpVar,"dxdx");
   svt::Field<real32> dydy(&tmpVar,"dydy");
   svt::Field<real32> dxdy(&tmpVar,"dxdy");
   svt::Field<real32> corner(&tmpVar,"corner");
   svt::Field<real32> weight(&tmpVar,"weight");


  // Create dx^2,dy^2 and dx*dy fields (Uses central difference)...
   for (nat32 y=1;y<img.Size(1)-1;y++)
   {
    for (nat32 x=1;x<img.Size(0)-1;x++)
    {
     real32 dx = 0.5*(img.Get(x+1,y)-img.Get(x-1,y));
     real32 dy = 0.5*(img.Get(x,y+1)-img.Get(x,y-1));
     
     dxdx.Get(x,y) = math::Sqr(dx);
     dydy.Get(x,y) = math::Sqr(dy);
     dxdy.Get(x,y) = dx*dy;
    }
   }


  // Apply a gaussian blur to each of the 3 fields...
   KernelVect kernel(2);
   kernel.MakeGaussian(0.7);
   
   kernel.Apply(dxdx,dxdx);
   kernel.Apply(dydy,dydy);
   kernel.Apply(dxdy,dxdy);


  // Calculate all the corner responses...
   for (nat32 y=2;y<img.Size(1)-2;y++)
   {
    for (nat32 x=2;x<img.Size(0)-2;x++)
    {
     corner.Get(x,y) = dxdx.Get(x,y)*dydy.Get(x,y) - math::Sqr(dxdy.Get(x,y)) - 
                       0.04*math::Sqr(dxdx.Get(x,y) + dydy.Get(x,y));
    }
   }



 // For all maximas set the associated position to the sub-pixel coordinate...
 // (Simple 1D parabola fitting - 1D is wrong really, and something more 
 // inteligent must be possible, I just havn't found it.)
  for (nat32 y=3;y<img.Size(1)-3;y++)
  {
   for (nat32 x=3;x<img.Size(0)-3;x++)
   {
    if ((corner.Get(x,y)>corner.Get(x+1,y))&&
        (corner.Get(x,y)>corner.Get(x-1,y))&&
        (corner.Get(x,y)>corner.Get(x,y+1))&&
        (corner.Get(x,y)>corner.Get(x,y-1))&&
        (corner.Get(x,y)>corner.Get(x-1,y+1))&&
        (corner.Get(x,y)>corner.Get(x-1,y-1))&&
        (corner.Get(x,y)>corner.Get(x+1,y+1))&&
        (corner.Get(x,y)>corner.Get(x+1,y-1)))
    {
     weight.Get(x,y) = corner.Get(x,y);
     snap.Get(x,y) = bs::Pnt(x,y);

     snap.Get(x,y)[0] += math::Clamp<real32>(math::ParabolaMax(corner.Get(x-1,y),corner.Get(x,y),corner.Get(x+1,y)),-0.5,0.5);
     snap.Get(x,y)[1] += math::Clamp<real32>(math::ParabolaMax(corner.Get(x,y-1),corner.Get(x,y),corner.Get(x,y+1)),-0.5,0.5);
    }
    else
    {
     snap.Get(x,y) = bs::Pnt(x,y);
    }
   }
  }



 // Propagate in the 4 compass directions to assign the correct position to each and every pixel...
  // +ve x...
   for (int32 y=0;y<int32(img.Size(1));y++)
   {
    for (int32 x=1;x<int32(img.Size(0));x++)
    {
     real32 prev = falloff * weight.Get(x-1,y);
     if (prev>weight.Get(x,y))
     {
      weight.Get(x,y) = prev;
      snap.Get(x,y) = snap.Get(x-1,y);       
     }
    }
   }

  // -ve x...
   for (int32 y=0;y<int32(img.Size(1));y++)
   {
    for (int32 x=img.Size(0)-2;x>=0;x--)
    {
     real32 prev = falloff * weight.Get(x+1,y);
     if (prev>weight.Get(x,y))
     {
      weight.Get(x,y) = prev;
      snap.Get(x,y) = snap.Get(x+1,y);       
     }
    }
   }

  // +ve y...
   for (int32 y=1;y<int32(img.Size(1));y++)
   {
    for (int32 x=0;x<int32(img.Size(0));x++)
    {
     real32 prev = falloff * weight.Get(x,y-1);
     if (prev>weight.Get(x,y))
     {
      weight.Get(x,y) = prev;
      snap.Get(x,y) = snap.Get(x,y-1);       
     }
    }
   }

  // -ve y...
   for (int32 y=img.Size(1)-2;y>=0;y--)
   {
    for (int32 x=0;x<int32(img.Size(0));x++)
    {
     real32 prev = falloff * weight.Get(x,y+1);
     if (prev>weight.Get(x,y))
     {
      weight.Get(x,y) = prev;
      snap.Get(x,y) = snap.Get(x,y+1);       
     }
    }
   }


 prog->Pop();
}

void HarrisSnap::Get(int32 x,int32 y,bs::Pnt & out)
{
 out = snap.Get(math::Clamp<int32>(x,0,snap.Width()-1),math::Clamp<int32>(y,0,snap.Height()-1));        
}

//------------------------------------------------------------------------------
 };
};
