//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/warp.h"

#include "eos/file/csv.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
EOS_FUNC void ForwardWarp(const svt::Field<bs::ColourRGB> & image,const svt::Field<real32> & disp,svt::Field<bs::ColourRGB> & out,real32 co,real32 t,const svt::Field<bit> * mask)
{
 // Create a tempory depth map... (Actually a disparity map, higher is nearer.)
  svt::Var * temp = new svt::Var(image.GetVar()->GetCore());
   temp->Setup2D(out.Size(0),out.Size(1));
   real32 depthIni = -1.0;
   temp->Add("depth",depthIni);
  temp->Commit();

  svt::Field<real32> depth; temp->ByName("depth",depth);

 // Set all pixels in out to be blue, incase there not written to...
  for (nat32 y=0;y<out.Size(1);y++)
  {
   for (nat32 x=0;x<out.Size(0);x++)
   {
    out.Get(x,y) = bs::ColourRGB(0.0,0.0,1.0);
   }
  }

 // Iterate the image and render each pixel, taking into account the depth map...
  for (nat32 y=0;y<image.Size(1);y++)
  {
   for (nat32 x=0;x<image.Size(0)-1;x++)
   {
    if ((mask==null<svt::Field<bit>*>())||(mask->Get(x,y)&&mask->Get(x+1,y)))
    {
     // Interpolate from this x to x+1, writting to any integers intercepted
     // along the way...
      real32 startX = x + t*disp.Get(x,y);
      real32 endX = x + 1.0 + t*disp.Get(x+1,y);
      real32 mult = 1.0/(endX-startX);
      if (math::Abs(endX-startX)<co)
      {
       int32 si = math::Max<int32>(0,math::RoundUp(int32(math::Min(startX,endX))));
       int32 ei = math::Min<int32>(out.Size(0)-1,math::RoundDown(int32(math::Max(startX,endX))));
       for (int32 i = si;i<=ei;i++)
       {
        real32 t = math::Clamp<real32>((real32(i)-startX)*mult,0.0,1.0);

        real32 d = math::Abs((1.0-t)*disp.Get(x,y) + t*disp.Get(x+1,y));
        if (d>depth.Get(i,y))
        {
         bs::ColourRGB colour;
          colour.r = (1.0-t)*image.Get(x,y).r + t*image.Get(x+1,y).r;
          colour.g = (1.0-t)*image.Get(x,y).g + t*image.Get(x+1,y).g;
          colour.b = (1.0-t)*image.Get(x,y).b + t*image.Get(x+1,y).b;

         out.Get(i,y) = colour;
         depth.Get(i,y) = d;
        }
       }
      }
    }
   }
  }

 // Clean up...
  delete temp;
}

//------------------------------------------------------------------------------
WarpInc::WarpInc(const svt::Field<bs::ColourRGB> & base,nat32 s)
:width(base.Size(0)),height(base.Size(1)),pixel(new Node[base.Size(0)*base.Size(1)]),
segments(s),segs(new Node*[s]),
occCost(0.1),diffSum(0.0),occCount(base.Size(0)*base.Size(1))
{
 Node * targ = pixel;
 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   targ->next = targ;
   targ->last = targ;
   targ->head = null<Node*>();
   targ->colour = base.Get(x,y);
   targ->count = true;

   targ++;
  }
 }

 for (nat32 i=0;i<segments;i++) segs[i] = null<Node*>();
}

WarpInc::~WarpInc()
{
 for (nat32 i=0;i<segments;i++) Remove(i);
 delete[] pixel;
 delete[] segs;
}

void WarpInc::SetMask(const svt::Field<bit> & mask)
{
 Node * targ = pixel;
 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   targ->count = mask.Get(x,y);
   targ++;
  }
 }
}

void WarpInc::Weights(real32 occ)
{
 occCost = occ;
}

nat32 WarpInc::Width()
{
 return width;
}

nat32 WarpInc::Height()
{
 return height;
}

void WarpInc::Add(nat32 x,nat32 y,real32 disp,const bs::ColourRGB & colour,nat32 segment)
{
 disp = math::Abs(disp);

 // Add it in...
  Node * head = &pixel[y*width + x];
  if (!head->count) return; // Break out if its a masked pixel.
  Node * nn = nodeAlloc.Malloc<Node>();

  Node * targ = head;
  while (true)
  {
   if ((targ->next==head)||(targ->next->disp<disp))
   {
    nn->next = targ->next;
    nn->last = targ;

    nn->next->last = nn;
    nn->last->next = nn;
    break;
   }
   targ = targ->next;
  }

  nn->segNext = segs[segment];
  segs[segment] = nn;

  nn->head = head;
  nn->colour = colour;
  nn->disp = disp;

 // Update the scores...
  if (targ==head) // Its the top dog.
  {
   if (nn->next->head==null<Node*>())
   {
    // We have filled in a gap, only slightly hairy...
     // Remove occlusion score...
      --occCount;

     // Add in new score...
      diffSum += math::Abs(nn->colour.r-head->colour.r) + math::Abs(nn->colour.g-head->colour.g) + math::Abs(nn->colour.b-head->colour.b);
   }
   else
   {
    // We have replaced the top node and are now the new king of the hill - very, very hairy...
     // Remove the old king, making him a humble occlusion...
      Node * ot = nn->next;
      ++occCount;
      diffSum -= math::Abs(ot->colour.r-head->colour.r) + math::Abs(ot->colour.g-head->colour.g) + math::Abs(ot->colour.b-head->colour.b);

     // Add in new score...
      diffSum += math::Abs(nn->colour.r-head->colour.r) + math::Abs(nn->colour.g-head->colour.g) + math::Abs(nn->colour.b-head->colour.b);
   }
  }
  else
  {
   // Slipped in behind existing data - just another occlusion...
    ++occCount;
  }
}

void WarpInc::Remove(nat32 segment)
{
 Node * targ = segs[segment];
 segs[segment] = null<Node*>();

 while (targ)
 {
  // Get next victim...
   Node * victim = targ;
   targ = targ->segNext;

  // Adjust score...
   if (victim->head==victim->last)
   {
    // Its the top node - some serious work is required:-(..
     // Remove its effect...
      diffSum -= math::Abs(victim->colour.r-victim->head->colour.r) + math::Abs(victim->colour.g-victim->head->colour.g) + math::Abs(victim->colour.b-victim->head->colour.b);

     // Add in the new kings effect...
      if (victim->next->head!=null<Node*>())
      {
       diffSum += math::Abs(victim->next->colour.r-victim->head->colour.r) + math::Abs(victim->next->colour.g-victim->head->colour.g) + math::Abs(victim->next->colour.b-victim->head->colour.b);
       --occCount;
      }
      else ++occCount;
   }
   else
   {
    // Its not visible - just another occlusion...
     --occCount;
   }

  // Remove from data structure...
   victim->next->last = victim->last;
   victim->last->next = victim->next;

  // Kill the victim...
   nodeAlloc.Free<Node>(victim);
 }
}

real32 WarpInc::Score() const
{
 return diffSum + real32(occCount)*occCost;
}

real32 WarpInc::ScoreDiff() const
{
 return diffSum;
}

real32 WarpInc::ScoreOcc() const
{
 return real32(occCount)*occCost;
}

real32 WarpInc::RawScore() const
{
 real32 ret = 0.0;
 nat32 retCount = 0;
 Node * targ = pixel;
 for (nat32 i=0;i<width*height;i++)
 {
  // If theres nothing in this node we add in an oclusion cost, otherwise we
  // have the cost of the top pixel plus all occluded pixels hiding under it...
   if (targ->count==false) continue;
   if (targ->next->head==null<Node*>())
   {
    ++retCount;
   }
   else
   {
    Node * st = targ->next;
    // Colour difference cost...
     ret += math::Abs(st->colour.r-targ->colour.r) + math::Abs(st->colour.g-targ->colour.g) + math::Abs(st->colour.b-targ->colour.b);
     st = st->next;

    // Occlusion cost...
     while (st->head!=null<Node*>())
     {
      ++retCount;
      st = st->next;
     }
   }
  ++targ;
 }
 return ret + real32(retCount)*occCost;
}

void WarpInc::GetImage(svt::Field<bs::ColourRGB> & out)
{
 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   if (pixel[y*width+x].next->head) out.Get(x,y) = pixel[y*width+x].next->colour;
                               else out.Get(x,y) = bs::ColourRGB(0.0,0.0,0.0);
  }
 }
}

//------------------------------------------------------------------------------
 };
};
