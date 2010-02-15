//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/filter/render_segs.h"

#include "eos/mem/alloc.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC void RenderSegsLines(const svt::Field<nat32> & in,svt::Field<bs::ColourRGB> & out,const bs::ColourRGB & lc)
{
 for (nat32 y=1;y<in.Size(1);y++)
 {
  for (nat32 x=1;x<in.Size(0);x++)
  {
   if ((in.Get(x,y)!=in.Get(x-1,y))||(in.Get(x,y)!=in.Get(x,y-1))) out.Get(x,y) = lc;
  }
 }
}

EOS_FUNC void RenderSegsColour(const svt::Field<nat32> & in,svt::Field<bs::ColourRGB> & out)
{
 for (nat32 y=0;y<in.Size(1);y++)
 {
  for (nat32 x=0;x<in.Size(0);x++)
  {
   nat32 seg = in.Get(x,y)+1; // +1 to avoid black.
   nat8 r,g,b;
    r = (seg&0x01)<<7 | (seg&0x08)<<3 | (seg&0x0040)>>1 | (seg&0x0200)>>5;
    g = (seg&0x02)<<6 | (seg&0x10)<<2 | (seg&0x0080)>>2 | (seg&0x0400)>>6;
    b = (seg&0x04)<<5 | (seg&0x20)<<1 | (seg&0x0100)>>3 | (seg&0x0800)>>7;
   out.Get(x,y) = bs::ColourRGB(real32(r)/255.0,real32(g)/255.0,real32(b)/255.0);
  }
 }
}

EOS_FUNC void RenderSegsMean(const svt::Field<nat32> & segs,const svt::Field<bs::ColourRGB> & col,svt::Field<bs::ColourRGB> & out)
{
 // First discover the largest segment number...
  nat32 segments = 0;
  for (nat32 y=0;y<segs.Size(1);y++)
  {
   for (nat32 x=0;x<segs.Size(0);x++)
   {
    segments = math::Max(segments,segs.Get(x,y)+1);
   }
  }

 // Create an array of segment colours and sums...
  nat32 * sum = new nat32[segments];
  bs::ColourRGB * ac = new bs::ColourRGB[segments];
  for (nat32 i=0;i<segments;i++)
  {
   sum[i] = 0;
   ac[i] = bs::ColourRGB(0.0,0.0,0.0);
  }

 // Second pass to fill in the segment colours array...
  for (nat32 y=0;y<segs.Size(1);y++)
  {
   for (nat32 x=0;x<segs.Size(0);x++)
   {
    ++sum[segs.Get(x,y)];
    ac[segs.Get(x,y)] += col.Get(x,y);
   }
  }

 // Divide through by the sums...
  for (nat32 i=0;i<segments;i++)
  {
   if (sum[i]!=0)
   {
    ac[i] /= real32(sum[i]);
   }
  }

 // Third pass to output the now averaged colours...
  for (nat32 y=0;y<segs.Size(1);y++)
  {
   for (nat32 x=0;x<segs.Size(0);x++)
   {
    out.Get(x,y) = ac[segs.Get(x,y)]; 
   }
  }

 // Clean up...
  delete[] sum;
  delete[] ac;
}

//------------------------------------------------------------------------------
 };
};
