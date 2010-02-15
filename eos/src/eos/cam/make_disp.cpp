//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/cam/make_disp.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
MakeDisp::MakeDisp()
{}

MakeDisp::MakeDisp(const CameraPair & pair)
{
 Reset(pair);
}

MakeDisp::~MakeDisp()
{}

void MakeDisp::Reset(const CameraPair & p)
{
 pair = p;
 disp.Resize(nat32(pair.leftDim[0]),nat32(pair.leftDim[1]));
 depth.Resize(nat32(pair.leftDim[0]),nat32(pair.leftDim[1]));
 mask.Resize(nat32(pair.leftDim[0]),nat32(pair.leftDim[1]));
 
 {
  math::Mat<3,3,real64> temp;
  rectLeft = pair.unRectLeft;   math::Inverse(rectLeft,temp);
  rectRight = pair.unRectRight; math::Inverse(rectRight,temp);
 }
 
 pair.lp.Centre(centreLeft);
 
 {
  math::Mat<4,3,real64> temp;
  pair.lp.GetInverse(temp);
  math::Mult(temp,pair.unRectLeft,invLP);
 }
 
 math::Vect<3,real64> dirLeftTemp;
 pair.lp.Dir(dirLeftTemp);
 for(nat32 i=0;i<3;i++) dirLeft[i] = dirLeftTemp[i];


 for (nat32 y=0;y<disp.Height();y++)
 {
  for (nat32 x=0;x<disp.Width();x++)
  {
   disp.Get(x,y) = 0.0;
   depth.Get(x,y) = -math::Infinity<real32>();
   mask.Get(x,y) = false;
  }
 }
}

void MakeDisp::Add(const bs::Vert & a,const bs::Vert & b,const bs::Vert & c)
{ 
 // Calculate the plane that intercepts the 3 coordinates...
 // (Simultaneously do backface culling.)
  math::Vect<4,real64> plane;
  {
   bs::Vert tb = b; tb -= a;
   bs::Vert tc = c; tc -= a;
   bs::Vert n;
   math::CrossProduct(tb,tc,n);
   if ((n*dirLeft)<0.0) return;
   plane[0] = n[0];
   plane[1] = n[1];
   plane[2] = n[2];
   plane[3] = -(n*a);
  }


 // Duplicate the 3 input coordinates, project them all to the left image...
  bs::Vert pos[3];
  pos[0] = a;
  pos[1] = b;
  pos[2] = c;
  
  bs::Pnt proj[3];
  Project(pos[0],proj[0]);
  Project(pos[1],proj[1]);
  Project(pos[2],proj[2]);


 // Sort them by y projection coordinates, lowest to highest...
  if (proj[1][1]<proj[0][1]) {math::Swap(pos[0],pos[1]); math::Swap(proj[0],proj[1]);}
  if (proj[2][1]<proj[0][1]) {math::Swap(pos[0],pos[2]); math::Swap(proj[0],proj[2]);}
  if (proj[2][1]<proj[1][1]) {math::Swap(pos[1],pos[2]); math::Swap(proj[1],proj[2]);}


 // Iterate the scanlines covered by the range, that are in the image...
  int32 minY = math::Max(int32(0),int32(math::RoundUp(proj[0][1])));
  int32 maxY = math::Min(int32(disp.Height()-1),int32(math::RoundDown(proj[2][1])));
  for (int32 y=minY;y<=maxY;y++)
  {
   // Determine the bounds on the x axis for this scanline, by intercepting the
   // projected line with the scanline for the two relevant edges of the
   // triangle...
    real32 xi[2];
    // Intercept the long side...
     xi[0] = proj[0][0] + (proj[2][0]-proj[0][0]) * ((real32(y)-proj[0][1])/(proj[2][1]-proj[0][1]));
    
    // Intercept the relevant short side...
     if (real32(y)>proj[1][1])
     {
      xi[1] = proj[1][0] + (proj[2][0]-proj[1][0]) * ((real32(y)-proj[1][1])/(proj[2][1]-proj[1][1]));
     }
     else
     {
      xi[1] = proj[1][0] + (proj[0][0]-proj[1][0]) * ((real32(y)-proj[1][1])/(proj[0][1]-proj[1][1]));
     }
    
    // Sort the two intercepts...
     if (xi[1]<xi[0]) math::Swap(xi[0],xi[1]);
    
    // Quantize...
     int32 minX = math::Max(int32(0),int32(math::RoundUp(xi[0])));
     int32 maxX = math::Min(int32(disp.Width()-1),int32(math::RoundDown(xi[1])));
  
  
   // Iterate the relevant x range, so we cover every pixel that intercepts
   // this triangle...
    for (int32 x=minX;x<=maxX;x++)
    {
     // Calculate the ray for the relevant pixel, as a plucker matrix...
      math::Vect<3,real64> pix;
      pix[0] = x;
      pix[1] = y;
      pix[2] = 1.0;
      
      math::Vect<4,real64> pol;
      math::MultVect(invLP,pix,pol);
      
      math::Mat<4,4,real64> ray;
      math::Plucker(centreLeft,pol,ray);


     // Intercept the ray with the triangles plane to get our final coordinate...
      math::Vect<4,real64> loc;
      math::MultVect(ray,plane,loc);


     // Check the coordinate is in front of the camera...
      real64 d = pair.lp.Depth(loc);
      if (d>=0.0) continue;


     // If a disparity is already assigned to the pixel check if ours is
     // closest...
      if (mask.Get(x,y))
      {
       if (depth.Get(x,y)>d) continue;
      }
      else
      {
       mask.Get(x,y) = true;
      }      
      
     
     // Update the pixel...
      real32 tempX,tempY;
      pair.Project(loc,tempX,tempY,disp.Get(x,y),&rectLeft,&rectRight);
      depth.Get(x,y) = d;
    }
  }
}

nat32 MakeDisp::Width() const
{
 return disp.Width();
}

nat32 MakeDisp::Height() const
{
 return disp.Height();
}

real32 MakeDisp::Disp(nat32 x,nat32 y) const
{
 return disp.Get(x,y);
}

bit MakeDisp::Mask(nat32 x,nat32 y) const
{
 return mask.Get(x,y);
}

void MakeDisp::GetDisp(svt::Field<real32> & out) const
{
 for (nat32 y=0;y<disp.Height();y++)
 {
  for (nat32 x=0;x<disp.Width();x++) out.Get(x,y) = disp.Get(x,y);
 }
}

void MakeDisp::GetMask(svt::Field<bit> & out) const
{
 for (nat32 y=0;y<mask.Height();y++)
 {
  for (nat32 x=0;x<mask.Width();x++) out.Get(x,y) = mask.Get(x,y);
 }
}

void MakeDisp::Project(bs::Vert & loc,bs::Pnt & out,real32 * disp)
{
 math::Vect<4,real64> pos;
 pos[0] = loc[0];
 pos[1] = loc[1];
 pos[2] = loc[2];
 pos[3] = 1.0;
 
 real32 d;
 pair.Project(pos,out[0],out[1],d,&rectLeft,&rectRight);
 if (disp) *disp = d;
}

//------------------------------------------------------------------------------
 };
};
