//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/stereo/dsr.h"

#include "eos/ds/arrays2d.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
DSR::~DSR()
{}

nat32 DSR::Matches()
{
 nat32 ret = 0;
 for (nat32 y=0;y<Height();y++)
 {
  for (nat32 x=0;x<Width();x++)
  {
   for (nat32 i=0;i<Ranges(x,y);i++)
   {
    ret += 1 + End(x,y,i) - Start(x,y,i);
   }
  }
 }
 return ret;
}

//------------------------------------------------------------------------------
RangeDSR::RangeDSR(nat32 w,nat32 h)
:width(w),height(h),start(0),end(0)
{}

RangeDSR::~RangeDSR()
{}

DSR * RangeDSR::Clone() const
{
 RangeDSR * ret = new RangeDSR(width,height);
 
 ret->start = start;
 ret->end = end;
 
 return ret;
}

void RangeDSR::Set(int32 s,int32 e)
{
 start = s;
 end = e;
}

nat32 RangeDSR::Width() const
{
 return width;
}

nat32 RangeDSR::Height() const
{
 return height;
}

nat32 RangeDSR::Ranges(nat32 x,nat32 y) const
{
 return 1;
}

int32 RangeDSR::Start(nat32 x,nat32 y,nat32 i) const
{
 return start;
}

int32 RangeDSR::End(nat32 x,nat32 y,nat32 i) const
{
 return end;
}

cstrconst RangeDSR::TypeString() const
{
 return "eos::stereo::RangeDSR";
}

//------------------------------------------------------------------------------
BasicDSR::BasicDSR(nat32 width,nat32 height)
:data(width,height)
{}

BasicDSR::BasicDSR(const DSR & rhs)
:data(rhs.Width(),rhs.Height())
{
 for (nat32 y=0;y<data.Height();y++)
 {
  for (nat32 x=0;x<data.Width();x++)
  {
   data.Get(x,y).Size(rhs.Ranges(x,y));
   for (nat32 i=0;i<data.Get(x,y).Size();i++)
   {
    data.Get(x,y)[i].start = rhs.Start(x,y,i);
    data.Get(x,y)[i].end   = rhs.End(x,y,i);
   }   
  }
 }
}

BasicDSR::BasicDSR(const DSI & rhs)
:data(rhs.Width(),rhs.Height())
{
 ds::Array<int32> disp(64);

 for (nat32 y=0;y<data.Height();y++)
 {
  for (nat32 x=0;x<data.Width();x++)
  {
   // Calculate the relevant values from the dsi for this pixel, stick them into
   // an array...
    nat32 num = 0;
    for (nat32 i=0;i<rhs.Size(x,y);i++)
    {
     int32 start = int32(math::RoundDown(rhs.Disp(x,y,i) - rhs.DispWidth(x,y,i) + 0.5));
     int32 end = int32(math::RoundUp(rhs.Disp(x,y,i) + rhs.DispWidth(x,y,i) - 0.5));
     
     for (int32 d=start;d<=end;d++)
     {
      if (num==disp.Size()) disp.Size(disp.Size()+32);
      disp[num] = d;
      ++num;
     }
    }

   // Sort the relevant range of the array...
    if (num==0) continue;
    disp.SortRangeNorm(0,num-1);
   
   // First pass to count the number of ranges required...
    nat32 rangeCount = 1;
    {
     int32 prev = disp[0];
     for (nat32 i=1;i<num;i++)
     {
      if (prev+1<disp[i]) ++rangeCount;
      prev = disp[i];
     }
    }

   // Resize the output to store the ranges...
    ds::Array<Range> & targ = data.Get(x,y);
    targ.Size(rangeCount);
   
   // Second pass to extract the ranges...
    nat32 range = 0;
    targ[0].start = disp[0];
    targ[0].end = disp[0];
    for (nat32 i=1;i<num;i++)
    {
     if (targ[range].end+1<disp[i])
     {
      // Onwards to next range...
       ++range;
       targ[range].start = disp[i];
       targ[range].end = disp[i];
     }
     else
     {
      // Extend current range...
       targ[range].end = disp[i];
     }
    }
  }
 }
}

BasicDSR::~BasicDSR()
{}

DSR * BasicDSR::Clone() const
{
 return new BasicDSR(*this);
}

void BasicDSR::Add(nat32 x,nat32 y,int32 start,int32 end)
{
 ds::Array<Range> & targ = data.Get(x,y);

 // Iterate the current array, checking each item against the start and end to 
 // see if they can be merged.
 // Update start/end as we go to include merged ranges.
 // Record how many pass unscathed before and after the final insertion position
 // of the result, ready for the next step...
  nat32 startCount = 0;
  nat32 endCount = 0;
  for (nat32 i=0;i<targ.Size();i++)
  {
   // Check if the current range is before the new range, if so record that it is 
   // to be copied un-harmed and move to the next range...
    if (targ[i].end+1<start) {++startCount; continue;}
   
   // Check if the current range is after the new range, if so break as we are done...
   // (Record that this and all future ranges are to be preserved.)
    if (targ[i].start>end+1) {endCount = targ.Size()-i; break;}
   
   // If we have got this far then a merging is required - apply...
    start = math::Min(start,targ[i].start);
    end   = math::Max(end  ,targ[i].end);
  }
 
 // Two cases - either the new range intercepts with nothing and the array needs
 // to grow, or the new range intercepts and the array needs to shrink, or
 // remain the same size...
  if (startCount+endCount==targ.Size())
  {
   // Grow...
    // Resize...
     targ.Size(targ.Size()+1);
    
    // Shift along to make space...
     for (nat32 i=targ.Size()-1;i>targ.Size()-1-endCount;i--)
     {
      targ[i] = targ[i-1];
     }
     
    // Insert...
     targ[startCount].start = start;
     targ[startCount].end   = end;
  }
  else
  {
   // Shrink, or be stable...
    nat32 ns = startCount+1+endCount;
    if (ns!=targ.Size())
    {
     // Move the end ranges along to get them out of the way of there shrinking universe...
      for (nat32 i=startCount+1;i<ns;i++)
      {
       targ[i] = targ[targ.Size() + i - ns];
      }
    
     // Resize... 
      targ.Size(ns); 
    }
    
    // Insert merged range...
     targ[startCount].start = start;
     targ[startCount].end   = end;
  }
}

void BasicDSR::Grow(nat32 range)
{
 for (nat32 y=0;y<data.Height();y++)
 {
  for (nat32 x=0;x<data.Width();x++)
  {
   ds::Array<Range> & targ = data.Get(x,y);
   if (targ.Size()==0) continue;

   // Iterate the array, expanding each range and checking if it collides with
   // previous, in which case it merges...
    targ[0].start -= int32(range);
    targ[0].end += int32(range);
    nat32 last = 0;
    for (nat32 i=1;i<targ.Size();i++)
    {
     targ[i].start -= int32(range);
     targ[i].end += int32(range);
     
     if (targ[i].start>targ[last].end+1)
     {
      // Not a merge...
       ++last;
       if (last!=i) targ[last] = targ[i];
     }
     else
     {
      // Merge...
       targ[last].end = targ[i].end;
     }
    }

   // If any merge happened then we need to shrink the array accordingly...
    ++last;
    if (last!=targ.Size()) targ.Size(last);
  }
 }
}

nat32 BasicDSR::Width() const
{
 return data.Width();
}

nat32 BasicDSR::Height() const
{
 return data.Height();
}

nat32 BasicDSR::Ranges(nat32 x,nat32 y) const
{
 return data.Get(x,y).Size();
}

int32 BasicDSR::Start(nat32 x,nat32 y,nat32 i) const
{
 return data.Get(x,y)[i].start;
}

int32 BasicDSR::End(nat32 x,nat32 y,nat32 i) const
{
 return data.Get(x,y)[i].end;
}

cstrconst BasicDSR::TypeString() const
{
 return "eos::stereo::BasicDSR";
}

//------------------------------------------------------------------------------
HierachyDSR::HierachyDSR()
:width(0),height(0),dsi(null<DSI*>()),range(0),
cacheX(0xFFFFFFFF),cacheY(0xFFFFFFFF),cacheSize(0),cacheRange(32)
{}

HierachyDSR::~HierachyDSR()
{}

DSR * HierachyDSR::Clone() const
{
 HierachyDSR * ret = new HierachyDSR();
 ret->width = width;
 ret->height = height;
 ret->dsi = dsi;
 ret->range = range;
 return ret;
}

void HierachyDSR::Set(nat32 w,nat32 h)
{
 width = w;
 height = h;
}

void HierachyDSR::Set(const DSI & d,nat32 r)
{
 dsi = &d;
 range = r;
}

void HierachyDSR::Set(const svt::Field<bit> & lMask,const svt::Field<bit> & rMask)
{
 leftMask = lMask;
 rightMask = rMask;
}

nat32 HierachyDSR::Width() const
{
 return width;
}

nat32 HierachyDSR::Height() const
{
 return height;
}

nat32 HierachyDSR::Ranges(nat32 x,nat32 y) const
{
 CalcCache(x,y);
 return cacheSize;
}

int32 HierachyDSR::Start(nat32 x,nat32 y,nat32 i) const
{
 CalcCache(x,y);
 return cacheRange[i].start;
}

int32 HierachyDSR::End(nat32 x,nat32 y,nat32 i) const
{
 CalcCache(x,y);
 return cacheRange[i].end;
}

cstrconst HierachyDSR::TypeString() const
{
 return "eos::stereo::HierachyDSR";
}

void HierachyDSR::CalcCache(nat32 x, nat32 y) const
{
 if ((cacheX==x)&&(cacheY==y)) return;
 cacheX = x;
 cacheY = y;

 nat32 px = x/2;
 nat32 py = y/2;
 
 if ((dsi->Size(px,py)==0)||(leftMask.Valid()&&(leftMask.Get(x,y)==false)))
 {
  cacheSize = 0;
  return;
 }
 
 if (cacheRange.Size()<dsi->Size(px,py)) cacheRange.Size(dsi->Size(px,py));

 cacheSize = 0;
 real32 d = (real32(px) + dsi->Disp(px,py,0))*2.0 - real32(x);
 cacheRange[0].start = int32(math::RoundDown(d - dsi->DispWidth(px,py,0) + 0.5 - real32(range)));
 cacheRange[0].end = int32(math::RoundUp(d + dsi->DispWidth(px,py,0) - 0.5 + real32(range)));
 for (nat32 i=1;i<dsi->Size(px,py);i++)
 {
  real32 d = (real32(px) + dsi->Disp(px,py,i))*2.0 - real32(x);
  int32 start = int32(math::RoundDown(d - dsi->DispWidth(px,py,i) + 0.5 - real32(range)));
  int32 end = int32(math::RoundUp(d + dsi->DispWidth(px,py,i) - 0.5 + real32(range)));
  
  if (start<=cacheRange[cacheSize].end+1)
  {
   // Merge...
    cacheRange[cacheSize].start = math::Min(cacheRange[cacheSize].start,start);
    cacheRange[cacheSize].end = math::Max(cacheRange[cacheSize].end,end);
  }
  else
  {
   // New...
    ++cacheSize;
    cacheRange[cacheSize].start = start;
    cacheRange[cacheSize].end = end;
  }
 }
 ++cacheSize;
}

//------------------------------------------------------------------------------
SpreadDSR::SpreadDSR()
{}

SpreadDSR::~SpreadDSR()
{}

DSR * SpreadDSR::Clone() const
{
 SpreadDSR * ret = new SpreadDSR();
 
 ret->data.Size(data.Size());
 for (nat32 y=0;y<ret->data.Size();y++)
 {
  ret->data[y].index.Size(data[y].index.Size());
  ret->data[y].data.Size(data[y].data.Size());

  for (nat32 x=0;x<ret->data[y].index.Size();x++) ret->data[y].index[x] = data[y].index[x];
  for (nat32 x=0;x<ret->data[y].data.Size();x++) ret->data[y].data[x] = data[y].data[x];
 }
 
 return ret;
}

void SpreadDSR::Set(const DSR & dsr,nat32 radius,const svt::Field<bit> & leftMask)
{
 mem::Packer packer(pack_size);
 
 // Mirror the input dsr into our own data structure...
  // Build empty index...
   ds::Array2D<RanLink> index1(dsr.Width(),dsr.Height());
   for (nat32 y=0;y<index1.Height();y++)
   {
    for (nat32 x=0;x<index1.Width();x++)
    {
     RanLink & targ = index1.Get(x,y);
     targ.ptr = null<Ran*>();
     targ.next = &targ;
     targ.last = &targ;
    }
   }

  // Fill it...
   for (nat32 y=0;y<dsr.Height();y++)
   {
    for (nat32 x=0;x<dsr.Width();x++)
    {
     RanLink & targ = index1.Get(x,y);
     for (nat32 i=0;i<dsr.Ranges(x,y);i++)
     {
      Ran * ran = packer.Malloc<Ran>();
      ran->rl.ptr = ran;
      ran->rl.next = &targ;
      ran->rl.last = targ.last;
      ran->range.start = dsr.Start(x,y,i);
      ran->range.end = dsr.End(x,y,i);
      
      ran->rl.next->last = &ran->rl;
      ran->rl.last->next = &ran->rl;
     }
    }
   }


 // Union the input into an intermediate, applying the horizontal part of the 
 // top-hat as we do so...
  // Build empty index...
   ds::Array2D<RanLink> index2(dsr.Width(),dsr.Height());
   for (nat32 y=0;y<index2.Height();y++)
   {
    for (nat32 x=0;x<index2.Width();x++)
    {
     RanLink & targ = index2.Get(x,y);
     targ.ptr = null<Ran*>();
     targ.next = &targ;
     targ.last = &targ;
    }
   }
   
  // Do the unioning...
   for (int32 y=0;y<int32(index1.Height());y++)
   {
    for (int32 x=0;x<int32(index1.Width());x++)
    {
     for (int32 x2=math::Max(int32(0),x-int32(radius));
                x2<=math::Min(int32(index2.Width()-1),x+int32(radius));
                x2++)
     {
      Union(index2.Get(x2,y),index1.Get(x,y),packer);
     }
    }
   }


 // Union the intermediate back to the dsr copy, applying the vertical part of
 // the top-hat as we do so (Can just leave original data, as it will be part
 // of the output regardless.)...
  for (int32 y=0;y<int32(index2.Height());y++)
  {
   for (int32 x=0;x<int32(index2.Width());x++)
   {
    for (int32 y2=math::Max(int32(0),y-int32(radius));
               y2<=math::Min(int32(index1.Height()-1),y+int32(radius));
               y2++)
    {
     Union(index1.Get(x,y2),index2.Get(x,y),packer);
    }
   }
  } 


 // Take the final data structure and convert it into the scanline structure
 // used for actual storage...
 // (It is here that the mask is considered.)
  data.Size(dsr.Height());
  for (nat32 y=0;y<data.Size();y++)
  {   
   // First pass over scanline to build the index...
    data[y].index.Size(dsr.Width()+1);
    data[y].index[0] = 0;
    for (nat32 x=0;x<dsr.Width();x++)
    {
     nat32 size = 0;
     if ((!leftMask.Valid())||(leftMask.Get(x,y)))
     {
      RanLink * targ = index1.Get(x,y).next;
      while (targ->ptr)
      {
       ++size;
       targ = targ->next;
      }
     }
     data[y].index[x+1] = data[y].index[x] + size;
    }
    
   // Second pass to build the data...
    data[y].data.Size(data[y].index[data[y].index.Size()-1]);
    nat32 ind = 0;
    for (nat32 x=0;x<dsr.Width();x++)
    {
     if ((!leftMask.Valid())||(leftMask.Get(x,y)))
     {
      RanLink * targ = index1.Get(x,y).next;
      while (targ->ptr)
      {
       data[y].data[ind].start = targ->ptr->range.start;
       data[y].data[ind].end = targ->ptr->range.end;
       ++ind;
       targ = targ->next;
      }
     }
    }
    log::Assert(ind==data[y].data.Size());
  }
}

nat32 SpreadDSR::Width() const
{
 return (data.Size()!=0)?(data[0].index.Size()-1):0;
}

nat32 SpreadDSR::Height() const
{
 return data.Size();
}

nat32 SpreadDSR::Ranges(nat32 x,nat32 y) const
{
 return data[y].index[x+1] - data[y].index[x];
}

int32 SpreadDSR::Start(nat32 x,nat32 y,nat32 i) const
{
 return data[y].data[data[y].index[x]+i].start;
}

int32 SpreadDSR::End(nat32 x,nat32 y,nat32 i) const
{
 return data[y].data[data[y].index[x]+i].end;
}

cstrconst SpreadDSR::TypeString() const
{
 return "eos::stereo::SpreadDSR";
}

void SpreadDSR::Union(RanLink & out,const RanLink & in,mem::Packer & packer)
{
 RanLink * outTarg = out.next;
 RanLink * inTarg = in.next; 
 while (inTarg->ptr)
 {
  // Whilst outTarg is less than inTarg move to next, assuming there is a next
  // to move to...
   while ((outTarg->ptr)&&
          (outTarg->ptr->range.end+1<inTarg->ptr->range.start))
   {
    outTarg = outTarg->next;
   }

  // Create a new Ran and insert it before the current outTarg, unless we can 
  // merge with the current outTarg in which case save the malloc...
   if ((outTarg->ptr)&&(outTarg->ptr->range.Intercepts(inTarg->ptr->range)))
   {
    // Merge with existing...
     outTarg->ptr->range += inTarg->ptr->range;
   }
   else
   {
    // Create new, make outTarg point at it...
     Ran * ran = packer.Malloc<Ran>();
     ran->range = inTarg->ptr->range;
     ran->rl.ptr = ran;
     ran->rl.next = outTarg;
     ran->rl.last = outTarg->last;
     
     ran->rl.next->last = &ran->rl;
     ran->rl.last->next = &ran->rl;

     outTarg = &ran->rl;
   }

  // Keep eatting outTarg's that merge with the current outTarg and are after it
  // until no more collisions occur...
   while ((outTarg->next->ptr)&&(outTarg->ptr->range.Intercepts(outTarg->next->ptr->range)))
   {
    RanLink * toDie = outTarg->next;
    outTarg->ptr->range += toDie->ptr->range;
    
    toDie->next->last = toDie->last;
    toDie->last->next = toDie->next;
   }

  inTarg = inTarg->next;
 }
}

//------------------------------------------------------------------------------
 };
};
