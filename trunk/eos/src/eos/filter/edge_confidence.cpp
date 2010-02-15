//-----------------------------------------------------------------------------
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

#include "eos/filter/edge_confidence.h"

#include "eos/math/constants.h"
#include "eos/math/functions.h"
#include "eos/ds/arrays.h"
#include "eos/filter/grad_angle.h"
#include "eos/filter/conversion.h"

namespace eos
{
 namespace filter
 {
//-----------------------------------------------------------------------------
// A structure, used by the below method...
struct RankPos
{
 nat32 x;
 nat32 y;
 real32 grad;
 
 bit operator < (const RankPos & rhs) const {return grad < rhs.grad;}
};

//-----------------------------------------------------------------------------
EOS_FUNC void EdgeConfidence(const svt::Field<real32> & in,const KernelVect & kernel,svt::Field<real32> & eta,svt::Field<real32> & rho)
{
 // Create the data structures we are going to be using...
  nat32 half = kernel.HalfSize();
  nat32 size = half*2 + 1;
  nat32 width = in.Size(0);
  nat32 height = in.Size(1);

  svt::Var * temp = new svt::Var(in.GetVar()->GetCore());
   temp->Setup2D(width,height);
   real32 ini = 0.0;
   temp->Add("quant",ini);
   temp->Add("grad",ini);
   temp->Add("ang",ini);
  temp->Commit(false);

  svt::Field<real32> quant;
  svt::Field<real32> gradiant;
  svt::Field<real32> angle;
   temp->ByName("quant",quant);
   temp->ByName("grad",gradiant);
   temp->ByName("ang",angle);

 // Quantizise the image as the algorithm is rather dependent on it being as such,
 // and produces bad results otherwise... (Yes, that means the algorithm is 
 // essentially broken.)
  Quant(in,quant);

 // First calculate the gradiant and angle for the input...
  GradAngle(quant,kernel,gradiant,angle);
  

 // Now generate eta using the angle...
  KernelMat ik;
  ik.SetSize(half);
   for (nat32 y=0;y<half;y++)
   {
    for (nat32 x=0;x<width;x++) eta.Get(x,y) = 0.0; 
   }
   for (nat32 y=half;y<height-half;y++)
   {
    for (nat32 x=0;x<half;x++) eta.Get(x,y) = 0.0;
    for (nat32 x=half;x<width-half;x++)
    {
     if (!math::Equal(gradiant.Get(x,y),real32(0.0)))
     {
      // Calculate the mean of the window over the image... 
       real32 mean = 0.0;
       for (int32 v=-half;v<=int32(half);v++)
       {
        for (int32 u=-half;u<=int32(half);u++) mean += in.Get(x+u,y+v);
       }
       mean /= math::Sqr(size);
 
      // Calculate the scalar required to make the image windows frobenius norm equal to 1...
       real32 mult = 0.0;
       for (int32 v=-half;v<=int32(half);v++)
       {
        for (int32 u=-half;u<=int32(half);u++)
        {
         mult += math::Sqr(in.Get(x+u,y+v)-mean);
        }
       }
       mult = math::InvSqrt(mult);
      
      // Make the relevent ideal kernel...
       ik.MakeIdeal(angle.Get(x,y));
       ik.ZeroMean();
       ik.FrobNorm(); 

      // Iterate and sum in the contribution to eta from each element in the
      // window...
       real32 o = 0.0;
        for (int32 v=-half;v<=int32(half);v++)
        {
         for (int32 u=-half;u<=int32(half);u++)
         {
          o += ik.Val(u,v)*(in.Get(x+u,y+v)-mean)*mult;
         }
        }
       eta.Get(x,y) = math::Abs(o);
     }
     else eta.Get(x,y) = 0.0;
    }
    for (nat32 x=width-half;x<width;x++) eta.Get(x,y) = 0.0;
   }
   for (nat32 y=height-half;y<height;y++)
   {
    for (nat32 x=0;x<width;x++) eta.Get(x,y) = 0.0;
   }
  
  
 // Sort the gradiant values...
  ds::Array<RankPos> sorted(width*height);
  nat32 i = 0;
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    sorted[i].x = x;
    sorted[i].y = y;    
    sorted[i].grad = gradiant.Get(x,y);
    ++i;
   }
  }
  sorted.Sort< ds::SortOp<RankPos> >();


 // Generate rho, per the papers specification, and after some copying from 
 // the author provided code when my original implimentation didn't do exactly
 // the same dance...
  rho.Get(sorted[0].x,sorted[0].y) = 0.0;
  real64 val = 1.0;
  for (nat32 i=1;i<sorted.Size();i++)
  {
   if (math::Equal(sorted[i].grad,real32(0.0))) rho.Get(sorted[i].x,sorted[i].y) = 0.0;
   else
   {
    rho.Get(sorted[i].x,sorted[i].y) = val;
    if (sorted[i].grad>sorted[i-1].grad) val += 1.0;
   }
  }
  val = 1.0/(val-1.0);
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    rho.Get(x,y) *= val;   
   }   
  }
 
 // Clean up...
  delete temp;
}

EOS_FUNC void EdgeConfidenceKernel(KernelVect & out,nat32 radius)
{
 out.SetSize(radius);

 for (int32 i=-radius;i<=int32(radius);i++)
 {
  real32 w = real32(math::Pow(2.0,-2.0*radius) * math::Factorial(2*radius)) / real32(math::Factorial(radius-i) * math::Factorial(radius+i));
  out.ValH(i) = (2.0*i*w)/real32(radius);
  out.ValV(i) = w;
 }
}

//------------------------------------------------------------------------------
 };
};
