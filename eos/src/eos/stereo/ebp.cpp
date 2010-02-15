//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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

#include "eos/stereo/ebp.h"

#include "eos/file/csv.h"
#include "eos/math/functions.h"
#include "eos/mem/functions.h"
#include "eos/ds/priority_queues.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
EBP::EBP()
:occCostBase(1.0),occCostMult(-0.1),occLimMult(2.0),iters(8),outCount(1),
dsr(null<DSR*>()),dsc(null<DSC*>()),dscOcc(null<DSC*>())
{}

EBP::~EBP()
{
 delete dsc;
 if (dsc!=dscOcc) delete dscOcc;
}

void EBP::Set(real32 ocb,real32 ocm,real32 olm,nat32 i,nat32 outC)
{
 occCostBase = ocb;
 occCostMult = ocm;
 occLimMult = olm;
 iters = i;
 outCount = outC;
}

void EBP::Set(DSR * d)
{
 delete dsr;
 dsr = d->Clone();
}

void EBP::Set(const DSC * d)
{
 delete dsc;
 if (dscOcc==dsc) dscOcc = null<DSC*>();
 dsc = d->Clone();
 if (dscOcc==null<DSC*>()) dscOcc = dsc;
}

void EBP::SetOcc(const DSC * d)
{
 if (dscOcc!=dsc) delete dscOcc;
 dscOcc = d->Clone();
}

void EBP::Run(time::Progress * prog)
{
 LogBlock("eos::stereo::EBP::Run","");
 prog->Push();

 // Asserts...
  log::Assert(dsc->HeightLeft()==dsc->HeightRight());
  log::Assert((dsr->Width()==dsc->WidthLeft())&&(dsr->Height()==dsc->HeightLeft()));



 // Calculate how many levels the hierachy will contain...
   int32 levels = math::Max(math::TopBit(dsc->WidthLeft()),
                            math::TopBit(dsc->HeightLeft()),
                            math::TopBit(dsc->WidthRight()));

   LogDebug("[ebp] {width,height,levels,matches}" << LogDiv()
            << dsc->WidthLeft() << LogDiv() << dsc->HeightLeft() << LogDiv()
            << levels << LogDiv() << dsr->Matches());



 // Phase 1 - construct the hierachy of data structures in which messages will be
 // passed...
  // Create the indexes and memory allocator...
   prog->Report(0,levels*2 + 3);
   ds::ArrayDel< ds::Array2D<Pixel*> > index(levels);
   mem::Packer mem(blockSize);

   index[0].Resize(dsc->WidthLeft(),dsc->HeightLeft());
   for (int32 l=1;l<levels;l++)
   {
    nat32 width = (index[l-1].Width()/2)   + ((index[l-1].Width()&1)?1:0);
    nat32 height = (index[l-1].Height()/2) + ((index[l-1].Height()&1)?1:0);
    index[l].Resize(width,height);
    LogDebug("[ebp] Level Size {level,width,height}" << LogDiv()
             << l << LogDiv() << width << LogDiv() << height);
   }


  // Build the base layer, with costs...
   mem::StackPtr<byte,mem::KillDelArray<byte> > pixA = new byte[dscOcc->Bytes()];
   mem::StackPtr<byte,mem::KillDelArray<byte> > pixB = new byte[dscOcc->Bytes()];

   prog->Report(1,levels*2 + 3);
   prog->Push();
   for (nat32 y=0;y<index[0].Height();y++)
   {
    prog->Report(y,index[0].Height());
    for (nat32 x=0;x<index[0].Width();x++)
    {
     // Check for it being masked...
      if (dsr->Ranges(x,y)==0)
      {
       index[0].Get(x,y) = null<Pixel*>();
       continue;
      }

     // Go through the dsr and count the number of disparities...
      nat32 msgSize = 0;
      for (nat32 i=0;i<dsr->Ranges(x,y);i++)
      {
       msgSize += 1 + dsr->End(x,y,i) - dsr->Start(x,y,i);
      }

     // Construct the relevant Pixel structure...
      nat32 pixSize = sizeof(Pixel) + dsr->Ranges(x,y) * sizeof(Section) + 5 * msgSize * sizeof(real32);
      Pixel * pix = (Pixel*)(void*)mem.Malloc<byte>(pixSize);
      index[0].Get(x,y) = pix;

      pix->passFlags = 0xFFFFFFFF;
      pix->msgSize = msgSize;
      pix->sections = dsr->Ranges(x,y);

     // Fill in the occCost array...
      dscOcc->Left(x,y,pixA.Ptr());
      for (nat32 i=0;i<4;i++)
      {
       pix->occCost[i] = occCostBase;

       nat32 xp = x;
       nat32 yp = y;
       bit done = false;
       switch (i)
       {
        case 0: ++xp; if (xp==index[0].Width()) done = true; break;
        case 1: ++yp; if (yp==index[0].Height()) done = true; break;
        case 2: if (xp==0) done = true; else --xp; break;
        case 3: if (yp==0) done = true; else --yp; break;
       }
       if (done) continue;

       dscOcc->Left(xp,yp,pixB.Ptr());
       real32 diff = dscOcc->Cost(pixA.Ptr(),pixB.Ptr());
       pix->occCost[i] += diff * occCostMult;
      }


     // Fill in the sections...
      for (nat32 i=0;i<dsr->Ranges(x,y);i++)
      {
       pix->GetSec(i)->startDisp = dsr->Start(x,y,i);
       pix->GetSec(i)->runLength = 1 + dsr->End(x,y,i) - dsr->Start(x,y,i);
      }

     // Fill in the matching costs taken from the DSC...
     // (Because we allow matches outside the image range we have to do bound checking.)
      nat32 offset = 0;
      for (nat32 i=0;i<pix->sections;i++)
      {
       for (int32 d=dsr->Start(x,y,i);d<=dsr->End(x,y,i);d++)
       {
        int32 ux2 = int32(x) + d;
        int32 x2 = math::Clamp(ux2,int32(0),int32(dsc->WidthRight())-1);
        pix->Start()[offset] = dsc->Cost(x,x2,y) + occCostBase*math::Abs(ux2-x2);
        ++offset;
       }
      }

     // Null the messages...
      for (nat32 i=pix->msgSize;i<pix->msgSize*5;i++) pix->Start()[i] = 0.0;
    }
   }
   prog->Pop();


  // Create further layers...
   for (int32 l=1;l<levels;l++)
   {
    prog->Report(l+1,levels*2 + 3);
    prog->Push();
    for (nat32 y=0;y<index[l].Height();y++)
    {
     prog->Report(y,index[l].Height());
     for (nat32 x=0;x<index[l].Width();x++)
     {
      nat32 fromX = x*2;
      nat32 fromY = y*2;

      bit okX = (fromX+1)!=index[l-1].Width();
      bit okY = (fromY+1)!=index[l-1].Height();

      Pixel * pix[4];

      pix[0] = index[l-1].Get(fromX,fromY);
      if (okX) pix[1] = index[l-1].Get(fromX+1,fromY);
          else pix[1] = null<Pixel*>();
      if (okY) pix[2] = index[l-1].Get(fromX,fromY+1);
          else pix[2] = null<Pixel*>();
      if (okX&&okY) pix[3] = index[l-1].Get(fromX+1,fromY+1);
               else pix[3] = null<Pixel*>();

      index[l].Get(x,y) = Union(pix,&mem);
     }
    }
    prog->Pop();
   }


  // Iterate all levels and set the pass flags to cover boundary conditions and
  // masked regions...
   prog->Report(levels+1,levels*2 + 3);
   prog->Push();
   for (int32 l=0;l<levels;l++)
   {
    prog->Report(l,levels);
    // Borders...
     // x...
      for (nat32 x=0;x<index[l].Width();x++)
      {
       {
        Pixel * targ = index[l].Get(x,0);
        if (targ) targ->Pass(3) = 0;
       }
       {
        Pixel * targ = index[l].Get(x,index[l].Height()-1);
        if (targ) targ->Pass(1) = 0;
       }
      }

     // y...
      for (nat32 y=0;y<index[l].Height();y++)
      {
       {
        Pixel * targ = index[l].Get(0,y);
        if (targ) targ->Pass(2) = 0;
       }
       {
        Pixel * targ = index[l].Get(index[l].Width()-1,y);
        if (targ) targ->Pass(0) = 0;
       }
      }


    // Nulled pixels...
     for (nat32 y=0;y<index[l].Height();y++)
     {
      for (nat32 x=0;x<index[l].Width();x++)
      {
       Pixel * targ = index[l].Get(x,y);
       if (targ)
       {
        for (nat32 i=0;i<4;i++)
        {
         if (targ->Pass(i))
         {
          nat32 nx = x;
          nat32 ny = y;
          switch (i)
          {
           case 0: ++nx; break;
           case 1: ++ny; break;
           case 2: --nx; break;
           case 3: --ny; break;
          }

          if (index[l].Get(nx,ny)==null<Pixel*>()) targ->Pass(i) = 0;
         }
        }
       }
      }
     }
   }
   prog->Pop();



 // Phase 2 - pass messages, starting at the smallest level of the hierachy
  // Do top level seperatly...
   prog->Report(levels+2,levels*2 + 3);
   prog->Push();
   nat32 topIters = 2*(index[levels-1].Width()+index[levels-1].Height());
   for (nat32 i=0;i<topIters;i++)
   {
    prog->Report(i,topIters);
    Iter(index[levels-1],i);
   }
   prog->Pop();


  // Do all further levels...
   for (int32 l=levels-2;l>=0;l--)
   {
    prog->Report(levels*2 + 1 - l,levels*2 + 3);
    prog->Push();

    // Transfer from above to current level of hierachy...
     prog->Report(0,iters+1);
     for (nat32 y=0;y<index[l].Height();y++)
     {
      for (nat32 x=0;x<index[l].Width();x++)
      {
       if (index[l].Get(x,y)) GetMessages(index[l].Get(x,y),index[l+1].Get(x/2,y/2));
      }
     }


    // Do the iterations...
     for (int32 i=0;i<iters;i++)
     {
      prog->Report(i+1,iters+1);
      Iter(index[l],i);
     }

    prog->Pop();
   }



 // Auxilary pass - extract the results...
  prog->Report(levels*2 + 2,levels*2 + 3);
  prog->Push();

  ds::PriorityQueue<Match> dispHeap(outCount);
  ds::Array<Match> dispArray(outCount);

  disp.Size(index[0].Height());
  for (nat32 y=0;y<disp.Size();y++)
  {
   prog->Report(y,disp.Size());

   // First pass to count how many disparities we will actually be outputting,
   // for this scanline. Index is filled in during this process...
    disp[y].index.Size(index[0].Width()+1);
    disp[y].index[0] = 0;
    for (nat32 x=0;x<index[0].Width();x++)
    {
     if (index[0].Get(x,y)) disp[y].index[x+1] = disp[y].index[x] + math::Min(index[0].Get(x,y)->msgSize,outCount);
                       else disp[y].index[x+1] = disp[y].index[x];
    }


   // Second pass to fill in the data structure with the actual disparities
   // and costs...
    disp[y].data.Size(disp[y].index[disp[y].index.Size()-1]);
    for (nat32 x=0;x<index[0].Width();x++)
    {
     Pixel * targ = index[0].Get(x,y);
     if ((targ!=null<Pixel*>())&&(targ->sections!=0))
     {
      // Find the best matches...
       dispHeap.MakeEmpty();
       DispIter di;
       targ->MakeIter(di);

       for (nat32 i=0;i<targ->msgSize;i++)
       {
        real32 cost = di.Value(0) + di.Value(1) + di.Value(2) + di.Value(3) + di.Value(4);
        if (dispHeap.Size()<outCount)
        {
         // No competition - just store it...
          Match m;
          m.disp = di.Disparity();
          m.cost = cost;
          dispHeap.Add(m);
        }
        else
        {
         // Check if its good enough to be included, and include it if so, removing the member it superceded...
          if (cost<dispHeap.Peek().cost)
          {
           dispHeap.Rem();
           Match m;
           m.disp = di.Disparity();
           m.cost = cost;
           dispHeap.Add(m);
          }
        }
        di.ToNext();
       }


      // Sort them by disparity...
       nat32 num = 0;
       while (dispHeap.Size()!=0)
       {
        dispArray[num] = dispHeap.Peek();
        ++num;
        dispHeap.Rem();
       }

       dispArray.SortRange<DispSort>(0,num-1);


      // Write them into the correct scanline positions...
       for (nat32 i=0;i<num;i++)
       {
        disp[y].data[disp[y].index[x]+i] = dispArray[i];
       }
     }
    }
  }
  prog->Pop();

 prog->Pop();
}

nat32 EBP::Width() const
{
 return (disp.Size()!=0)?(disp[0].index.Size()-1):0;
}

nat32 EBP::Height() const
{
 return disp.Size();
}

nat32 EBP::Size(nat32 x, nat32 y) const
{
 return disp[y].index[x+1] - disp[y].index[x];
}

real32 EBP::Disp(nat32 x, nat32 y, nat32 i) const
{
 return disp[y].data[disp[y].index[x]+i].disp;
}

real32 EBP::Cost(nat32 x, nat32 y, nat32 i) const
{
 return disp[y].data[disp[y].index[x]+i].cost;
}

real32 EBP::DispWidth(nat32 x, nat32 y, nat32 i) const
{
 return 0.5;
}

cstrconst EBP::TypeString() const
{
 return "eos::stereo::EBP";
}

//------------------------------------------------------------------------------
void EBP::Iter(ds::Array2D<Pixel*> & index,nat32 iter)
{
 LogTime("eos::stereo::EBP::Iter");
 for (nat32 y=0;y<index.Height();y++)
 {
  for (nat32 x=((y+iter)%2);x<index.Width();x+=2)
  {
   // Iterate every other pixel with a checkboard pattern, for each pixel
   // calculate and pass the messages, except when the flags say otherwise...
    Pixel * targ = index.Get(x,y);
    if (targ==null<Pixel*>()) continue;

    for (nat32 md=0;md<4;md++)
    {
     if (targ->Pass(md))
     {
      // Get iterators for destination message and self, with destination index...
       DispIter self;
       targ->MakeIter(self);

       DispIter out;
       switch (md)
       {
        case 0: index.Get(x+1,y)->MakeIter(out); break;
        case 1: index.Get(x,y+1)->MakeIter(out); break;
        case 2: index.Get(x-1,y)->MakeIter(out); break;
        default: index.Get(x,y-1)->MakeIter(out); break;
       }
       if (out.msgSize==0) continue; // This shouldn't be required. Bug, presumably, means it is however.

       nat32 outInd = 1 + ((md+2)%4);


      // Forward pass - calculate the minimum cost, fill in the output message
      // with minimums including previous entrys...
       real32 lastCost = math::Infinity<real32>();
       int32 lastDisp = 0; // Above infinity makes this value irrelevant, zero to make compiler shut up.

       real32 minCost = lastCost;
       nat32 selfRem = self.msgSize;

       for (nat32 i=0;;i++)
       {
        // Move self forward until it matches or excedes the disparity of out,
        // maintainning some variables...
         if (selfRem!=0)
         {
          while (self.Disparity()<=out.Disparity())
          {
           real32 cost = self.Value(0);
           for (nat32 j=0;j<md;j++) cost += self.Value(j+1);
           for (nat32 j=md+1;j<4;j++) cost += self.Value(j+1);
           minCost = math::Min(minCost,cost);

           lastCost = math::Min(cost,lastCost + targ->occCost[md]*real32(self.Disparity()-lastDisp));
           lastDisp = self.Disparity();

           --selfRem;
           if (selfRem==0) break;
           self.ToNext();
          }
         }

        // Update the out message and support variables, move to next...
         out.Value(outInd) = lastCost + targ->occCost[md]*real32(out.Disparity()-lastDisp);
         if (i+1==out.msgSize) break;
         out.ToNext();
       }


      // Continue onto the end of self, ready for the backward pass...
       if (selfRem!=0)
       {
        while (true)
        {
         real32 cost = self.Value(0);
         for (nat32 j=0;j<md;j++) cost += self.Value(j+1);
         for (nat32 j=md+1;j<4;j++) cost += self.Value(j+1);
         minCost = math::Min(minCost,cost);

         --selfRem;
         if (selfRem==0) break;
         self.ToNext();
        }
       }


      // Backward pass - propagate the costs and apply the cost cap...
       lastCost = math::Infinity<real32>();
       lastDisp = 0;

       selfRem = self.msgSize;
       real32 occLim = targ->occCost[md] * occLimMult;

       for (nat32 i=0;;i++)
       {
        // Move self forward until it matches or excedes the disparity of out,
        // maintainning some variables...
         if (selfRem!=0)
         {
          while (self.Disparity()>=out.Disparity())
          {
           real32 cost = self.Value(0);
           for (nat32 j=0;j<md;j++) cost += self.Value(j+1);
           for (nat32 j=md+1;j<4;j++) cost += self.Value(j+1);

           lastCost = math::Min(cost,lastCost + targ->occCost[md]*real32(lastDisp-self.Disparity()));
           lastDisp = self.Disparity();

           --selfRem;
           if (selfRem==0) break;
           self.ToPrev();
          }
         }

        // Update the out message and support variables, move to next...
         real32 byLast = lastCost + targ->occCost[md]*real32(lastDisp-out.Disparity());
         out.Value(outInd) = math::Min(out.Value(outInd),byLast);
         out.Value(outInd) = math::Min(occLim,out.Value(outInd)-minCost);

         if (i+1==out.msgSize) break;
         out.ToPrev();
       }
     }
    }
  }
 }
}

EBP::Pixel * EBP::Union(Pixel * pix[4],mem::Packer * memAlloc)
{
 LogTime("eos::stereo::EBP::Union");

 // First pass to count how many disparities and sections are required...
  nat32 secCount = 0;
  nat32 msgSize = 0;
  {
   // Get iterators...
    DispIter iter[4];
    nat32 remainder[4];
    for (nat32 i=0;i<4;i++)
    {
     if (pix[i])
     {
      remainder[i] = pix[i]->msgSize;
      pix[i]->MakeIter(iter[i]);
     }
     else remainder[i] = 0;
    }

   // Iterate and count...
    int32 lastDisp = 0;
    while (true)
    {
     // Find the lowest disparity...
      int32 lowest = math::max_int_32;
      for (int32 i=0;i<4;i++)
      {
       if (remainder[i]!=0) lowest = math::Min(lowest,iter[i].Disparity());
      }

     // Break if all remainders are zero...
      if (lowest==math::max_int_32) break;

     // Update section and message count...
      if ((secCount==0)||(lastDisp+1!=lowest)) secCount += 1;
      msgSize += 1;
      lastDisp = lowest;

     // Advance relevant iterators, decriment relevant remainders...
      for (int32 i=0;i<4;i++)
      {
       if ((remainder[i]!=0)&&(lowest==iter[i].Disparity()))
       {
        iter[i].ToNext();
        remainder[i] -= 1;
       }
      }
    }
  }


 // Return null if there is nothing to output (All null.)...
  if (secCount==0) return null<Pixel*>();


 // Create the returned pixel...
  nat32 pixSize = sizeof(Pixel) + secCount * sizeof(Section) + 5 * msgSize * sizeof(real32);
  Pixel * ret = (Pixel*)(void*)memAlloc->Malloc<byte>(pixSize);

  ret->passFlags = 0xFFFFFFFF;
  ret->msgSize = msgSize;
  ret->sections = secCount;


 // Fill in the occCost array...
 {
  for (nat32 i=0;i<4;i++) ret->occCost[i] = 0.0;
  int32 div = 0;
  for (nat32 i=0;i<4;i++)
  {
   if (pix[i])
   {
    for (nat32 j=0;j<4;j++) ret->occCost[j] += pix[i]->occCost[j];
    div += 1;
   }
  }
  if (div!=0)
  {
   for (nat32 i=0;i<4;i++) ret->occCost[i] /= real32(div);
  }
 }


 // Second pass to extract the disparities and fill in the matching costs
 // and messages...
  {
   // Get iterators...
    DispIter iter[4];
    nat32 remainder[4];
    for (nat32 i=0;i<4;i++)
    {
     if (pix[i])
     {
      remainder[i] = pix[i]->msgSize;
      pix[i]->MakeIter(iter[i]);
     }
     else remainder[i] = 0;
    }

   // Iterate and copy...
    int32 section = -1;
    int32 offset = -1;
    int32 lastDisp = 0;
    while (true)
    {
     // Find the lowest disparity...
      int32 lowest = math::max_int_32;
      for (int32 i=0;i<4;i++)
      {
       if (remainder[i]!=0) lowest = math::Min(lowest,iter[i].Disparity());
      }

     // Break if all remainders are zero...
      if (lowest==math::max_int_32) break;

     // Update section and message count...
      if ((section==-1)||(lastDisp+1!=lowest))
      {
       section += 1;
       ret->GetSec(section)->startDisp = lowest;
       ret->GetSec(section)->runLength = 0;
      }
      offset += 1;
      ret->GetSec(section)->runLength += 1;
      lastDisp = lowest;

     // Advance relevant iterators, decriment relevant remainders, store
     // matching cost sum...
      real32 sum = 0.0;
      for (int32 i=0;i<4;i++)
      {
       if ((remainder[i]!=0)&&(lowest==iter[i].Disparity()))
       {
        sum += iter[i].Value(0);

        iter[i].ToNext();
        remainder[i] -= 1;
       }
      }

     // Update the pixels matching cost and messages...
      ret->Start()[offset] = sum;
      for (nat32 i=1;i<5;i++) ret->Start()[ret->msgSize * i + offset] = 0.0;
    }
  }


 return ret;
}

void EBP::GetMessages(Pixel * to,Pixel * from)
{
 LogTime("eos::stereo::EBP::GetMessages");
 log::Assert((to!=null<Pixel*>())&&(from!=null<Pixel*>()),"eos::stereo::EBP::GetMessages");

 // Get iterators for both...
  DispIter out;
  DispIter in;

  to->MakeIter(out);
  from->MakeIter(in);


 // Iterate and extract...
  for (nat32 i=0;i<to->msgSize;i++)
  {
   // Get to matched disparity on input...
    while (in.Disparity()<out.Disparity()) in.ToNext();

   // Transfer message across...
    for (nat32 j=1;j<5;j++) out.Value(j) = in.Value(j);

   out.ToNext();
  }
}

//------------------------------------------------------------------------------
 };
};
