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

#include "eos/filter/kernel.h"

#include "eos/math/constants.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace filter
 {
//-----------------------------------------------------------------------------
KernelMat & KernelMat::operator = (const KernelMat & rhs)
{
 SetSize(rhs.half);
  nat32 total = math::Sqr(half*2 + 1);
 for (nat32 i=0;i<total;i++) data[i] = rhs.data[i];
 return *this;
}

void KernelMat::SetSize(nat32 h)
{
 if (h!=half)
 {
  delete[] data;
  half = h;

  nat32 total = math::Sqr(half*2 + 1);
  data = new real32[total];
  for (nat32 i=0;i<total;i++) data[i] = 0.0;
 }
 else
 {
  nat32 total = math::Sqr(half*2 + 1);
  for (nat32 i=0;i<total;i++) data[i] = 0.0;
 }
}

real32 & KernelMat::Val(int32 x,int32 y)
{
 int32 width = half*2 + 1;
 real32 * centre = data + (width+1)*half;
 return centre[x + y*width];
}

void KernelMat::Apply(const svt::Field<real32> & in,svt::Field<real32> & out) const
{
 // First do the borders, set them all to zero...
  for (nat32 y=0;y<half;y++)
  {
   for (nat32 x=0;x<out.Size(0);x++) out.Get(x,y) = 0.0;
  }
  for (nat32 y=out.Size(1)-half;y<out.Size(1);y++)
  {
   for (nat32 x=0;x<out.Size(0);x++) out.Get(x,y) = 0.0;
  }
  for (nat32 y=half;y<out.Size(1)-half;y++)
  {
   for (nat32 x=0;x<half;x++) out.Get(x,y) = 0.0;
   for (nat32 x=out.Size(0)-half;x<out.Size(0);x++) out.Get(x,y) = 0.0;
  }

 // Some values...
  int32 width = half*2 + 1;
  real32 * centre = data + (width+1)*half;

 // Then the hard work, calculate with brute force the solution for the soft
 // juicy centre...
  for (nat32 y=half;y<out.Size(1)-half;y++)
  {
   for (nat32 x=half;x<out.Size(0)-half;x++)
   {
    real32 o = 0.0;
     for (int32 v = -half;v<=int32(half);v++)
     {
      for (int32 u = -half;u<=int32(half);u++)
      {
       o += centre[width*v + u]*in.Get(x+u,y+v);
      }
     }
    out.Get(x,y) = o;
   }
  }
}

void KernelMat::MakeIdeal(real32 angle)
{
 // Fast indexing helps...
  int32 width = half*2 + 1;
  real32 * centre = data + (width+1)*half;

 // Make a normal to the slicing line, this makes it very
 // easy to test which side any point is on, and how far away they are...
 real32 vec[2];
  vec[0] = math::Cos(angle);
  vec[1] = math::Sin(angle);

 // Iterate each point, try to classify fully one side or the other quickly,
 // when this can't be done things get painful...
  centre[0] = 0.5;
  for (int32 y=-half;y<=int32(half);y++)
  {
   for (int32 x=-half;x<=int32(half);x++)
   {
    if ((x==0)&&(y==0)) continue;
    real32 dist = x*vec[0] + y*vec[1]; // Shoudl be negative, everything is simply reversed.
    if (math::Abs(dist)>(0.5*math::Sqrt(2.0)))
    {
     // The line can't possible intercept - sign of dist tells us its value...
      centre[x + y*width] = (dist>0.0)?1.0:0.0;
      continue;
    }
    
    // Ok the line *probably* intercepts the square, time to get fiddly.
    // We work out the distance for each corner anc check signs, putting them
    // into the lower 4 bits of a byte. This then acts as a lookup to the 
    // action to take, in a big fat switch statement.
    // When a line is intecepted we work out how far along it we need to go
    // using the distances calculated and using areas of triangles etc calculate
    // a perfect value...
     real32 c[4];
      c[0] = (x-0.5)*vec[0] + (y-0.5)*vec[1]; // Bottom left.
      c[1] = (x+0.5)*vec[0] + (y-0.5)*vec[1]; // Bottom right.
      c[2] = (x+0.5)*vec[0] + (y+0.5)*vec[1]; // Top right.
      c[3] = (x-0.5)*vec[0] + (y+0.5)*vec[1]; // Top left.

     byte flags = 0;
      if (c[0]>0.0) flags |= 1;
      if (c[1]>0.0) flags |= 2;
      if (c[2]>0.0) flags |= 4;
      if (c[3]>0.0) flags |= 8;
     
     // Some useful functions for below...
      struct UF
      {
       // Returns the area of a triangle going from a to b to c, with b as the right angle.
       static real32 TriArea(real32 a,real32 b,real32 c)
       {
        real32 p1 = 1.0 - (math::Abs(a)/(math::Abs(a)+math::Abs(b)));
        real32 p2 = 1.0 - (math::Abs(c)/(math::Abs(c)+math::Abs(b)));
        return 0.5*p1*p2;
       }
       
       static real32 SqrArea(real32 a,real32 b,real32 c,real32 d) // b and c are on the side of the line we want the area for.
       {
        real32 p1 = 1.0 - (math::Abs(a)/(math::Abs(a)+math::Abs(b)));
        real32 p2 = 1.0 - (math::Abs(d)/(math::Abs(d)+math::Abs(c)));
        return 0.5*math::Abs(p1-p2) + math::Min(p1,p2);
       }
      };
     
     // The mother of all switches... 
     switch (flags)
     {
      case  0: centre[x + y*width] = 0.0; break;
      case  1: centre[x + y*width] = UF::TriArea(c[3],c[0],c[1]); break;
      case  2: centre[x + y*width] = UF::TriArea(c[0],c[1],c[2]); break;
      case  3: centre[x + y*width] = UF::SqrArea(c[3],c[0],c[1],c[2]); break;
      case  4: centre[x + y*width] = UF::TriArea(c[1],c[2],c[3]); break;
      case  5: centre[x + y*width] = 0.0; break; // Impossible
      case  6: centre[x + y*width] = UF::SqrArea(c[0],c[1],c[2],c[3]); break;
      case  7: centre[x + y*width] = 1.0 - UF::TriArea(c[2],c[3],c[0]); break;
      case  8: centre[x + y*width] = UF::TriArea(c[2],c[3],c[0]); break;
      case  9: centre[x + y*width] = UF::SqrArea(c[2],c[3],c[0],c[1]); break;
      case 10: centre[x + y*width] = 0.0; break; // Impossible
      case 11: centre[x + y*width] = 1.0 - UF::TriArea(c[1],c[2],c[3]); break;
      case 12: centre[x + y*width] = UF::SqrArea(c[1],c[2],c[3],c[0]); break;
      case 13: centre[x + y*width] = 1.0 - UF::TriArea(c[0],c[1],c[2]); break;
      case 14: centre[x + y*width] = 1.0 - UF::TriArea(c[3],c[0],c[1]); break;
      case 15: centre[x + y*width] = 1.0; break;
     }
   }	 
  }
}
   
void KernelMat::ZeroMean()
{
 nat32 size = math::Sqr(half*2 + 1);
 
 real32 sum = 0.0;
 for (nat32 i=0;i<size;i++) sum += data[i];
 sum /= real32(size);
 
 for (nat32 i=0;i<size;i++) data[i] -= sum;
}

void KernelMat::FrobNorm()
{
 nat32 size = math::Sqr(half*2 + 1);
 
 real32 sum = 0.0;
 for (nat32 i=0;i<size;i++) sum += math::Sqr(data[i]);
 sum = math::InvSqrt(sum);
 
 for (nat32 i=0;i<size;i++) data[i] *= sum;
}

//-----------------------------------------------------------------------------
KernelVect & KernelVect::operator = (const KernelVect & rhs)
{
 SetSize(rhs.half);
 nat32 size = half*2 + 1;
 for (nat32 i=0;i<size;i++) {a[i] = rhs.a[i]; b[i] = rhs.b[i];}
 return *this;
}

void KernelVect::SetSize(nat32 h)
{
 if (half!=h)
 {
  delete[] a;
  delete[] b;
  half = h;
  nat32 size = half*2 + 1;
  a = new real32[size];
  b = new real32[size];
  for (nat32 i=0;i<size;i++) {a[i] = 0.0; b[i] = 0.0;}
 }
 else
 {
  nat32 size = half*2 + 1;
  for (nat32 i=0;i<size;i++) {a[i] = 0.0; b[i] = 0.0;}
 }
}

void KernelVect::ConvertTo(KernelMat & out) const
{
 out.SetSize(half);
 for (int32 v=-half;v<=int32(half);v++)
 {
  for (int32 u=-half;u<=int32(half);u++)
  {
   out.Val(u,v) = ValH(u)*ValV(v);	  
  }	 
 }
}

void KernelVect::Apply(const svt::Field<real32> & in,svt::Field<real32> & out,bit transpose) const
{
 nat32 width = out.Size(0);
 nat32 height = out.Size(1); // Now create the intermediate data structure...
 // We transpose the data when writting out to it, reading back from it, as this
 // creates for better cache access behaviour.
  real32 * im = new real32[width*height];

 // Do the first pass - calculate on the x...
  real32 * t = a;
  if (transpose) t = b;
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<half;x++) im[x*height + y] = 0.0;
   for (nat32 x=half;x<width-half;x++)
   {
    real32 o = 0.0;
    for (int32 i=-half;i<=int32(half);i++)
    {
     o += in.Get(x+i,y)*t[i+half];
    }
    im[x*height + y] = o;
   }
   for (nat32 x=width-half;x<width;x++) im[x*height + y] = 0.0;
  }

 // Do the second pass - calculate on the y, except its the x because of the transpose...
  t = b;
  if (transpose) t = a;
  for (nat32 y=0;y<half;y++)
  {
   for (nat32 x=0;x<width;x++) out.Get(x,y) = 0.0;
  }
  for (nat32 y=half;y<height-half;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    real32 o = 0.0;
    for (int32 i=-half;i<=int32(half);i++)
    {
     o += im[x*height + y+i]*t[i+half];
    }
    out.Get(x,y) = o;
   }
  }
  for (nat32 y=height-half;y<height;y++)
  {
   for (nat32 x=0;x<width;x++) out.Get(x,y) = 0.0;
  }  

 // Clean up...
  delete[] im;
}

void KernelVect::ApplyRepeat(const svt::Field<real32> & in,svt::Field<real32> & out,bit transpose) const
{
 int32 width = out.Size(0);
 int32 height = out.Size(1); // Now create the intermediate data structure...
 // We transpose the data when writting out to it, reading back from it, as this
 // creates for better cache access behaviour.
  real32 * im = new real32[width*height];

 // Do the first pass - calculate on the x...
  real32 * t = a;
  if (transpose) t = b;
  for (int32 y=0;y<height;y++)
  {
   for (int32 x=0;x<width;x++)
   {
    real32 o = 0.0;
    for (int32 i=-half;i<=int32(half);i++)
    {
     o += in.Get(math::Clamp<int32>(x+i,0,width),y)*t[i+half];
    }
    im[x*height + y] = o;
   }
  }

 // Do the second pass - calculate on the y, except its the x because of the transpose...
  t = b;
  if (transpose) t = a;
  for (int32 y=0;y<height;y++)
  {
   for (int32 x=0;x<width;x++)
   {
    real32 o = 0.0;
    for (int32 i=-half;i<=int32(half);i++)
    {
     o += im[x*height + math::Clamp<int32>(y+i,0,height)]*t[i+half];
    }
    out.Get(x,y) = o;
   }
  }

 // Clean up...
  delete[] im;
}

void KernelVect::MakeGaussian(real32 sd)
{
 real32 sum = 0.0;
 for (int32 i=-half;i<=int32(half);i++)
 {
  ValH(i) = math::Exp(-math::Sqr(i)/(2.0*math::Sqr(sd)));
  sum += ValH(i);
 }
 sum = 1.0/sum;
 
 for (int32 i=-half;i<=int32(half);i++)
 {
  ValH(i) *= sum;
  ValV(i) = ValH(i);
 } 
}

//------------------------------------------------------------------------------
 };
};
