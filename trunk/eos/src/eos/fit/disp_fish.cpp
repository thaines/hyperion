//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/fit/disp_fish.h"

#include "eos/math/functions.h"
#include "eos/ds/arrays.h"
#include "eos/svt/var.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
DispFish::DispFish()
:range(3)
{}

DispFish::~DispFish()
{}

void DispFish::Set(const svt::Field<real32> & di,const stereo::DSC & ds)
{
 disp = di;
 dsc = &ds;
}

void DispFish::SetPair(const cam::CameraPair & p)
{
 pair = p;
}

void DispFish::SetRange(nat32 r)
{
 range = r;
}

void DispFish::Run(time::Progress * prog)
{
 prog->Push();
 
 // First create depth/cost pairs for all pixels covering the range - for each
 // pixel offset the lowest value to 0 to get stability...
  // Make data structure...
   svt::Var temp(disp);
   nat32 scope = range*2 + 1;
   {
    temp.Setup3D(scope,disp.Size(0),disp.Size(1)); // Note order.
    Pixel initPix;
     initPix.pos = bs::Vert(0.0,0.0,0.0);
     initPix.cost = 0.0;
     initPix.weight = 0.0;
    temp.Add("pix",initPix);
    temp.Commit();
   }
   svt::Field<Pixel> pix(&temp,"pix");
  
  // Fill data structure...
   real32 distMult = 1.0/real32(range+1);
   for (nat32 y=0;y<disp.Size(1);y++)
   {
    for (nat32 x=0;x<disp.Size(0);x++)
    {
     real32 bd = disp.Get(x,y);
     int32 base = int32(math::Round(bd)) - int32(range);
     real32 minCost = math::Infinity<real32>();
     for (nat32 s=0;s<scope;s++)
     {
      Pixel & p = pix.Get(s,x,y);
      int32 d = base + int32(s);
      
      pair.Triangulate(x,y,d,p.pos);      
      p.cost = dsc->Cost(x,math::Clamp<int32>(int32(x)+d,0,disp.Size(0)-1),y);
      p.weight = math::Max(1.0 - distMult*math::Abs(real32(d)-bd),0.0);
      
      minCost = math::Min(minCost,p.cost);
     }
     
     for (nat32 s=0;s<scope;s++) pix.Get(s,x,y).cost -= minCost;
    }
   }


 // Create the buffer to store R-contribution/weight pairs...
  ds::Array<Rcont> rb(scope*scope*scope); // This is big.
  
  out.Resize(disp.Size(0),disp.Size(1));
 
 // Iterate the pixels and calculate the distribution for each - this consists 
 // of an easy direction and hard concentration...
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    // Get differential calculation stuff - basically border handling...
     int32 dxI = 1;
     real32 dxM = 1.0;
     if ((x+1)==disp.Size(0)) {dxI = -1; dxM = -1.0;}

     int32 dyI = 1;
     real32 dyM = 1.0;
     if ((y+1)==disp.Size(1)) {dyI = -1; dyM = -1.0;}
     
    // Calculate the direction from most likelly disparity...
     bs::Normal dir;
     {
      // Get locations of disparities...
       bs::Vert b,ix,iy;
       pair.Triangulate(x,y,disp.Get(x,y),b);
       pair.Triangulate(int32(x)+dxI,y,disp.Get(int32(x)+dxI,y),ix);
       pair.Triangulate(x,int32(y)+dyI,disp.Get(x,int32(y)+dyI),iy);
      
      // Get differences...
       bs::Vert dx = ix; dx -= b; dx *= dxM;
       bs::Vert dy = iy; dy -= b; dy *= dyM;
       
      // Cross product and normalise...
       math::CrossProduct(dx,dy,dir);
       dir.Normalise();
     }
    
    // Calculate concentration - this involves a very, very expensive L-estimate...
     real32 k;
     {
      // Fill array with samples...
       nat32 rbInd = 0;
       for (nat32 sb=0;sb<scope;sb++)
       {
        for (nat32 sx=0;sx<scope;sx++)
        {
         for (nat32 sy=0;sy<scope;sy++)
         {
          // Get pointers to the 3 relevant entrys...
           Pixel & base = pix.Get(sb,x,y);
           Pixel & xx = pix.Get(sx,int32(x)+dxI,y);
           Pixel & yy = pix.Get(sy,x,int32(y)+dyI);
          
          // Calculate the surface orientation...
           bs::Vert dx = xx.pos; dx -= base.pos; dx *= dxM;
           bs::Vert dy = yy.pos; dy -= base.pos; dy *= dyM;
           
           bs::Normal sDir;
           math::CrossProduct(dx,dy,sDir);
           sDir.Normalise();
          
          // Dot product with dir to get the r component...
           real32 r = sDir * dir;

          // Calculate the weight...
           real32 weight = math::Exp(-(base.cost + xx.cost + yy.cost));
           weight *= base.weight * xx.weight * yy.weight; // ************* Not sure about this ****************
           
          // Store in the rb array...
           rb[rbInd].r = r;
           rb[rbInd].weight = weight;
           ++rbInd;
         }
        }
       }
       log::Assert(rb.Size()==rbInd);
      
      // Sort array...
       rb.SortNorm();
      
      // Sum total weight...
       real32 weightSum = 0.0;
       for (nat32 i=0;i<rb.Size();i++) weightSum += rb[i].weight;
       
      // Iterate through to calculate inverse k...
       real32 invK = 0.0;
       real32 weI = 0.0;
       real32 a1 = 1.0/weightSum;
       real32 m1 = -2.0/(weightSum*weightSum*(weightSum + 1.0));
       for (nat32 i=0;i<rb.Size();i++)
       {
        invK += (a1 + m1*weI) * (1.0-rb[i].r);
        weI += rb[i].weight;
       }
       
      // Convert to concentration...
       k = 1.0/invK;
     }
    
    // Store it in the output array...
     out.Get(x,y) = dir;
     out.Get(x,y) *= k;
   }
  }  
 
 
 prog->Pop();
}

void DispFish::Get(svt::Field<bs::Vert> & fish)
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++) fish.Get(x,y) = out.Get(x,y);
 }
}

bs::Vert & DispFish::GetFish(nat32 x,nat32 y)
{
 return out.Get(x,y);
}

//------------------------------------------------------------------------------
 };
};
