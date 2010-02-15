//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "eos/cam/disparity_converter.h"

#include "eos/cam/triangulation.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
DispConvPlane::DispConvPlane()
:rectValid(false)
{}

DispConvPlane::~DispConvPlane()
{}

DispConv * DispConvPlane::Clone() const
{
 DispConvPlane * ret = new DispConvPlane();
  ret->pair = pair;
  ret->leftMask = leftMask;
  ret->rightMask = rightMask;
  ret->rectValid = rectValid;
  ret->rectLeft = rectLeft;
  ret->rectRight = rectRight;
 return ret;
}

void DispConvPlane::Set(const cam::CameraPair & p,
                        nat32 leftWidth,nat32 leftHeight,
                        nat32 rightWidth,nat32 rightHeight)
{
 pair = p;
 pair.ScaleLeft(leftWidth,leftHeight);
 pair.ScaleRight(rightWidth,rightHeight);
 rectValid = false;
}

void DispConvPlane::Set(const svt::Field<bit> & left,const svt::Field<bit> & right)
{
 leftMask = left;
 rightMask = right;
}

bit DispConvPlane::PosInd() const
{
 return true;
}
  
real32 DispConvPlane::DispToDepth(real32 x,real32 y,real32 disp) const
{
 math::Vect<2,real64> rlp;
  rlp[0] = x; rlp[1] = y;
 math::Vect<2,real64> rrp = rlp;
  rrp[0] += disp;
     
 // Convert to true image coordinates...
  math::Vect<2,real64> lp;
  math::MultVectEH(pair.unRectLeft,rlp,lp);
  
  math::Vect<2,real64> rp;
  math::MultVectEH(pair.unRectRight,rrp,rp);

 // Calculate depth...
  real64 o;
  if (Depth(lp,rp,pair.lp,pair.rp,o)) return real32(o);
                                 else return 0.0; // Most likely reason for failure is intersection.
}

void DispConvPlane::DispToPos(real32 x,real32 y,real32 disp,bs::Vertex & pos) const
{
 math::Vect<2,real64> rlp;
  rlp[0] = x; rlp[1] = y;
 math::Vect<2,real64> rrp = rlp;
  rrp[0] += disp;
     
 // Convert to true image coordinates...
  math::Vect<2,real64> lp;
  math::MultVectEH(pair.unRectLeft,rlp,lp);
  
  math::Vect<2,real64> rp;
  math::MultVectEH(pair.unRectRight,rrp,rp);
  
 // Calculate pos...
  math::Vect<4,real64> out;
  if (cam::Triangulate(lp,rp,pair.lp,pair.rp,out)==false)
  {
   pos[0] = 0.0; pos[1] = 0.0; pos[2] = -1.0; pos[3] = 0.0;
   return;
  }
  pos = out;
}

real32 DispConvPlane::DepthToDisp(real32 x,real32 y,real32 depth) const
{
 math::Vect<2,real64> rlp; rlp[0] = x; rlp[1] = y;

 math::Vect<2,real64> lp;
 math::MultVectEH(pair.unRectLeft,rlp,lp);

 math::Vect<4,real64> xx;
 if (DepthToPos(lp,pair.lp,depth,xx)==false) return 0.0;

 math::Vect<3,real64> rp;
 math::MultVect(pair.rp,xx,rp);

 math::Mat<3,3,real64> temp[2];
 temp[0] = pair.unRectRight;
 math::Inverse(temp[0],temp[1]);

 math::Vect<3,real64> rrp;
 math::MultVect(temp[0],rp,rrp);
 rrp /= rrp[2];

 return rrp[0]-rlp[0];
}

void DispConvPlane::PosToDisp(const bs::Vertex & pos,real32 & x,real32 & y,real32 & disp) const
{
 // Project the position onto both camera planes...
  math::Vect<3,real64> lp;
  math::Vect<3,real64> rp;

  math::MultVect(pair.lp,pos,lp);
  math::MultVect(pair.rp,pos,rp);


 // Move to rectified coordinates...  
  if (!rectValid)
  {
   rectValid = true;
   math::Mat<3,3,real64> temp;

   rectLeft = pair.unRectLeft;
   math::Inverse(rectLeft,temp);
  
   rectRight = pair.unRectRight;
   math::Inverse(rectRight,temp);
  }
  
  math::Vect<3,real64> rlp;
  math::Vect<3,real64> rrp;
  math::MultVect(rectLeft,lp,rlp);
  math::MultVect(rectRight,rp,rrp);
  rlp /= rlp[2];
  rrp /= rrp[2];


 // Extract the numbers we require...
  x = rlp[0];
  y = rlp[1];
  disp = rrp[0]-rlp[0];  
}

void DispConvPlane::Ray(real32 x,real32 y,bs::Vertex & start,bs::Vertex & offset) const
{
 // Get the start...
  math::Vect<4,real64> centre;
  pair.lp.Centre(centre);
  start = centre;
 
 // Get the offset...
  math::Mat<4,3,real64> lpInv;
  math::PseudoInverse(pair.lp,lpInv);
  math::Vect<3,real64> p;
  p[0] = x; p[1] = y; p[2] = 1.0;
  math::MultVect(lpInv,p,offset);

  offset -= start;
}

void DispConvPlane::Convert(svt::Field<real32> & disp,svt::Field<real32> & depth) const
{
 for (nat32 y=0;y<disp.Size(1);y++)
 {
  for (nat32 x=0;x<disp.Size(0);x++)
  {
   if ((!leftMask.Valid())||(leftMask.Get(x,y)))
   {
    // Get rectified image coordinates...
     math::Vect<3,real64> rlp;
      rlp[0] = x; rlp[1] = y; rlp[2] = 1.0;
     math::Vect<3,real64> rrp = rlp;
      rrp[0] += disp.Get(x,y);
     
    // Convert to true image coordinates...
     math::Vect<3,real64> tlp;
      math::MultVect(pair.unRectLeft,rlp,tlp);
     math::Vect<3,real64> trp;
      math::MultVect(pair.unRectLeft,rrp,trp);
    
    // Make non-homogenous...
     math::Vect<2,real64> lp;
      tlp /= tlp[2]; lp[0] = tlp[0]; lp[1] = tlp[1];
     math::Vect<2,real64> rp;
      trp /= trp[2]; rp[0] = trp[0]; rp[1] = trp[1];
    
    // Calculate depth...
     real64 o;
     if (Depth(lp,rp,pair.lp,pair.rp,o)) depth.Get(x,y) = real32(o);
                                    else depth.Get(x,y) = math::Infinity<real32>();
   }
   else
   {
    depth.Get(x,y) = 0.0;
   }
  }
 }
}

void DispConvPlane::Convert(svt::Field<real32> & disp,svt::Field<bs::Vertex> & pos) const
{
 for (nat32 y=0;y<disp.Size(1);y++)
 {
  for (nat32 x=0;x<disp.Size(0);x++)
  {
   if ((!leftMask.Valid())||(leftMask.Get(x,y)))
   {
    // Get rectified image coordinates...
     math::Vect<3,real64> rlp;
      rlp[0] = x; rlp[1] = y; rlp[2] = 1.0;
     math::Vect<3,real64> rrp = rlp;
      rrp[0] += disp.Get(x,y);
     
    // Convert to true image coordinates...
     math::Vect<3,real64> tlp;
      math::MultVect(pair.unRectLeft,rlp,tlp);
     math::Vect<3,real64> trp;
      math::MultVect(pair.unRectLeft,rrp,trp);
    
    // Make non-homogenous...
     math::Vect<2,real64> lp;
      tlp /= tlp[2]; lp[0] = tlp[0]; lp[1] = tlp[1];
     math::Vect<2,real64> rp;
      trp /= trp[2]; rp[0] = trp[0]; rp[1] = trp[1];
    
    // Calculate position...
     math::Vect<4,real64> v;
     if (Triangulate(lp,rp,pair.lp,pair.rp,v)==false) pos.Get(x,y) = bs::Vertex(0.0,0.0,-1.0,0.0);
                                                 else pos.Get(x,y) = v;
   }
   else
   {
    math::Vect<4,real64> v;
    pair.lp.Centre(v);
    pos.Get(x,y) = v;
   }
  }
 }
}

void DispConvPlane::Convert(svt::Field<real32> & disp,svt::Field<bs::Normal> & needle) const
{
 // Get the surface normal pointing directly at the camera...
  Intrinsic intr;
  math::Mat<3,3,real64> rot;
  pair.lp.Decompose(intr,rot);
  bs::Normal toCam;
   toCam[0] = -rot[0][2];
   toCam[1] = -rot[1][2];
   toCam[2] = -rot[2][2];
  //LogDebug("toCam" << LogDiv() << toCam);

 // Iterate it all, bar the borders...
  for (nat32 y=1;y<disp.Size(1)-1;y++)
  {
   for (nat32 x=1;x<disp.Size(0)-1;x++)
   {
    // Check the 4 neighbours for being valid, rather than the output location...
     if ((!leftMask.Valid())||
         (leftMask.Get(x-1,y)&&leftMask.Get(x+1,y)&&leftMask.Get(x,y-1)&&leftMask.Get(x,y+1)))
     {
      bs::Vert v[4];
      bit ok = true;
       ok &= CalcPos(x-1,y,disp.Get(x-1,y),disp.Size(0),disp.Size(1),v[0]);
       ok &= CalcPos(x+1,y,disp.Get(x+1,y),disp.Size(0),disp.Size(1),v[1]);
       ok &= CalcPos(x,y-1,disp.Get(x,y-1),disp.Size(0),disp.Size(1),v[2]);
       ok &= CalcPos(x,y+1,disp.Get(x,y+1),disp.Size(0),disp.Size(1),v[3]);
      
      if (ok)
      {
       v[1] -= v[0];
       v[3] -= v[2];
      
       math::CrossProduct(v[1],v[3],needle.Get(x,y));
       needle.Get(x,y).Normalise();
      //LogDebug("{x,y,norm}" << LogDiv() << x << LogDiv() << y << LogDiv() << needle.Get(x,y));
      }
      else
      {
       needle.Get(x,y) = toCam;
      }
     }
     else
     {
      needle.Get(x,y) = toCam;
     }
   }
  }
  
 // Copy neighbouring orientations to the borders...
  // Corners...
   needle.Get(0,0) = needle.Get(1,1);
   needle.Get(disp.Size(0)-1,0) = needle.Get(disp.Size(0)-2,1);
   needle.Get(0,disp.Size(1)-1) = needle.Get(1,disp.Size(1)-2);
   needle.Get(disp.Size(0)-1,disp.Size(1)-1) = needle.Get(disp.Size(0)-2,disp.Size(1)-2);

  // Edges...
   for (nat32 x=1;x<disp.Size(0)-1;x++)
   {
    needle.Get(x,0) = needle.Get(x,1);   
    needle.Get(x,disp.Size(1)-1) = needle.Get(x,disp.Size(1)-2);
   }
   
   for (nat32 y=1;y<disp.Size(1)-1;y++)
   {
    needle.Get(0,y) = needle.Get(1,y);   
    needle.Get(disp.Size(0)-1,y) = needle.Get(disp.Size(0)-2,y);
   }
}

cstrconst DispConvPlane::TypeString() const
{
 return "eos::cam::DispConvPlane";
}

bit DispConvPlane::CalcPos(nat32 x,nat32 y,real32 disp,nat32 width,nat32 height,bs::Vert & out) const
{
 // Get rectified image coordinates...
  math::Vect<3,real64> rlp;
   rlp[0] = x; rlp[1] = y; rlp[2] = 1.0;
  math::Vect<3,real64> rrp = rlp;
   rrp[0] += disp;

 // Adjust for scale...
  real64 mult[2];
   mult[0] = pair.leftDim[0]/real64(width);
   mult[1] = pair.leftDim[1]/real64(height);
  
  rlp[0] *= mult[0];
  rlp[1] *= mult[1];
  rrp[0] *= mult[0]; // We assume the same scaling was applied to both left and right images, certainly has to be true in y.
  rrp[1] *= mult[1];

 // Convert to true image coordinates...
  math::Vect<3,real64> tlp;
   math::MultVect(pair.unRectLeft,rlp,tlp);
  math::Vect<3,real64> trp;
   math::MultVect(pair.unRectRight,rrp,trp);
    
 // Make non-homogenous...
  math::Vect<2,real64> lp;
   tlp /= tlp[2]; lp[0] = tlp[0]; lp[1] = tlp[1];
  math::Vect<2,real64> rp;
   trp /= trp[2]; rp[0] = trp[0]; rp[1] = trp[1];
    
 // Calculate depth...
  math::Vect<4,real64> pos;
  if (Triangulate(lp,rp,pair.lp,pair.rp,pos)==false) return false;
  if (math::Equal(pos[3],0.0)) return false;

  pos /= pos[3];
  out[0] = pos[0];
  out[1] = pos[1];
  out[2] = pos[2];    
  return true;
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
 };
};
