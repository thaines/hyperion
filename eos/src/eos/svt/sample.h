#ifndef EOS_SVT_SAMPLE_H
#define EOS_SVT_SAMPLE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file sample.h
/// Provides sampling capabilities for interpolating etc field objects.

#include "eos/types.h"
#include "eos/math/functions.h"
#include "eos/svt/field.h"
#include "eos/bs/colours.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
/// Uses linear interpolation to obtain a value from a non-integer point on a 
/// 2D field of reals. Handles boundary conditions.
/// Returns true on success, false on failure. On failure out is not set,
/// happens due to out of bounds of being in a totally masked area.
EOS_FUNC inline bit SampleRealLin2D(real32 & out,real32 x,real32 y,const Field<real32> & in,const Field<bit> * mask = null<Field<bit>*>())
{
 int32 xb = int32(math::RoundDown(x));
 int32 yb = int32(math::RoundDown(y));
 
 // Check bounds...
  if ((xb<-1)||(yb<-1)||(xb>=int32(in.Size(0)))||(yb>=int32(in.Size(1)))) return false;
  bit boundX[2];
   boundX[0] = xb!=-1;
   boundX[1] = (xb+1)!=int32(in.Size(0));
  bit boundY[2];
   boundY[0] = yb!=-1;
   boundY[1] = (yb+1)!=int32(in.Size(1));

 // Weight in the 4 corners...
  real32 xw[2];
   xw[1] = x-xb;
   xw[0] = 1.0 - xw[1];
  real32 yw[2];
   yw[1] = y-yb;
   yw[0] = 1.0 - yw[1];  
  
  out = 0.0;
  real32 weight = 0.0;
  if (mask)
  {
   for (nat32 v=0;v<2;v++)
   {
    for (nat32 u=0;u<2;u++)
    {
     if (boundX[u]&&boundY[v]&&mask->Get(xb+u,yb+v))
     {
      real32 w = xw[u]*yw[v];
      out += in.Get(xb+u,yb+v)*w;
      weight += w; 
     }
    }
   }
  }
  else
  {
   for (nat32 v=0;v<2;v++)
   {
    for (nat32 u=0;u<2;u++)
    {
     if (boundX[u]&&boundY[v])
     {
      real32 w = xw[u]*yw[v];
      out += in.Get(xb+u,yb+v)*w;
      weight += w;
     }
    }
   }
  }
  
  if (math::IsZero(weight)) return false;
                       else out /= weight;
  
  return true;
}

/// Uses linear interpolation to obtain a value from a non-integer point on a 
/// 2D field of ColourRGBs. Handles boundary conditions.
/// Returns true on success, false on failure. On failure out is not set,
/// happens due to out of bounds of being in a totally masked area.
EOS_FUNC inline bit SampleColourRGBLin2D(bs::ColourRGB & out,real32 x,real32 y,const Field<bs::ColourRGB> & in,const Field<bit> * mask = null<Field<bit>*>())
{
 int32 xb = int32(math::RoundDown(x));
 int32 yb = int32(math::RoundDown(y));
 
 // Check bounds...
  if ((xb<-1)||(yb<-1)||(xb>=int32(in.Size(0)))||(yb>=int32(in.Size(1)))) return false;
  bit boundX[2];
   boundX[0] = xb!=-1;
   boundX[1] = (xb+1)!=int32(in.Size(0));
  bit boundY[2];
   boundY[0] = yb!=-1;
   boundY[1] = (yb+1)!=int32(in.Size(1));

 // Weight in the 4 corners...
  real32 xw[2];
   xw[1] = x-xb;
   xw[0] = 1.0 - xw[1];
  real32 yw[2];
   yw[1] = y-yb;
   yw[0] = 1.0 - yw[1];  
  
  out.r = 0.0; out.g = 0.0; out.b = 0.0;
  real32 weight = 0.0;
  if (mask)
  {
   for (nat32 v=0;v<2;v++)
   {
    for (nat32 u=0;u<2;u++)
    {
     if (boundX[u]&&boundY[v]&&mask->Get(xb+u,yb+v))
     {
      real32 w = xw[u]*yw[v];
      out.r += in.Get(xb+u,yb+v).r*w;
      out.g += in.Get(xb+u,yb+v).g*w;
      out.b += in.Get(xb+u,yb+v).b*w;
      weight += w; 
     }
    }
   }
  }
  else
  {
   for (nat32 v=0;v<2;v++)
   {
    for (nat32 u=0;u<2;u++)
    {
     if (boundX[u]&&boundY[v])
     {
      real32 w = xw[u]*yw[v];
      out.r += in.Get(xb+u,yb+v).r*w;
      out.g += in.Get(xb+u,yb+v).g*w;
      out.b += in.Get(xb+u,yb+v).b*w;
      weight += w;
     }
    }
   }
  }
  
  if (math::IsZero(weight)) return false;
                       else out /= weight;
  
  return true;
}

//------------------------------------------------------------------------------
 };
};
#endif
