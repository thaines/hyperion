//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/filter/mscr.h"

#include "eos/ds/arrays2d.h"
#include "eos/math/functions.h"
#include "eos/math/eigen.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC real32 DistMSCR(bs::ColourRGB & a,bs::ColourRGB & b)
{
 real32 dsr = math::Sqr(a.r-b.r);
 real32 dsg = math::Sqr(a.g-b.g);
 real32 dsb = math::Sqr(a.b-b.b);

 real32 sr = a.r + b.r;
 real32 sg = a.g + b.g;
 real32 sb = a.b + b.b;
 
 if (!math::IsZero(sr)) dsr /= sr;
 if (!math::IsZero(sg)) dsg /= sg;
 if (!math::IsZero(sb)) dsb /= sb;
 
 return math::Sqrt(dsr + dsg + dsb);
}

//------------------------------------------------------------------------------
MSCR::MSCR()
:minSize(60),areaCap(2.0),minDim(1.5)
{}
   
MSCR::~MSCR()
{}

void MSCR::Input(const svt::Field<bs::ColourRGB> & input)
{
 image = input;
}

void MSCR::Run(time::Progress * prog)
{
 LogTime("eos::filter::MSCR::Run");
 prog->Push();
 
 // Some helper stuff...
  static const int32 secX[4] = {1,0,1,-1};
  static const int32 secY[4] = {0,1,1, 1};  


 // Create a sorted array of crack edges, this being the order to do the merges in...
 // (All crack edges are valid, so boundary checking is not needed.)
  int32 width = int32(image.Size(0));
  int32 height = int32(image.Size(1));
  ds::Array<Crack> merge(4*width*height - 3*width - 3*height + 2);
  {
   real32 mult[4];
   for (nat32 dir=0;dir<4;dir++)
   {
    mult[dir] = 1.0/math::Sqrt(math::Abs(secX[dir]) + math::Abs(secY[dir]));
   }
  
   nat32 ind = 0;
   prog->Report(0,5);
   prog->Push();
   for (int32 y=0;y<height;y++)
   {
    prog->Report(y,height);
    for (int32 x=0;x<width;x++)
    {
     // Generate all 4 cracks for this pixel as boundary conditions allow...
      for (byte dir=0;dir<4;dir++)
      {
       int32 x2 = x + secX[dir];
       int32 y2 = y + secY[dir];
       if ((x2<0)||(x2>=width)) continue;
       if ((y2<0)||(y2>=height)) continue;
       
       merge[ind].x = x;
       merge[ind].y = y;
       merge[ind].dist = DistMSCR(image.Get(x,y),image.Get(x2,y2)) * mult[dir];
       merge[ind].dir = dir;
       
       ++ind;
      }
    }
   }
   prog->Pop();
   log::Assert(ind==merge.Size());
   
   prog->Report(1,5);
   merge.SortNorm();
  }



 // Build the forest...
  prog->Report(2,5);
  ds::Array2D<Region> forest(width,height);
  {
   prog->Push();
   for (int32 y=0;y<height;y++)
   {
    prog->Report(y,height);
    for (int32 x=0;x<width;x++)
    {
     Region & targ = forest.Get(x,y);
    
     targ.parent = null<Region*>();
    
     targ.past = null<Region*>();
     targ.dist = 0.0;
    
     targ.size = 1;
     targ.eX = real32(x);
     targ.eY = real32(y);
     targ.eXX = math::Sqr(targ.eX);
     targ.eYY = math::Sqr(targ.eY);
     targ.eXY = targ.eX * targ.eY;
    }
   }
   prog->Pop();
  }



 // Iterate through the merge list of cracks - merge each crack as needed.
 // When cracks are merged handle all scenarios, including storing found regions
 // as needed...
  ds::Stack<StableRegion> data;
  prog->Report(3,5);
  prog->Push();
  for (nat32 i=0;i<merge.Size();i++)
  {
   prog->Report(i,merge.Size());
   // Get the parents of the pixels to merge...
    Region * par1 = forest.Get(merge[i].x,merge[i].y).Parent();
    Region * par2 = forest.Get(merge[i].x + secX[merge[i].dir],merge[i].y + secY[merge[i].dir]).Parent();
    
   // If different then merge...
    if (par1!=par2)
    {
     // Make par1 the larger region...
      if (par2->size>par1->size) math::Swap(par1,par2);
     
     // Create a tempory Region that represents the merge...
      Region temp;
      {
       temp.parent = null<Region*>();
       temp.past = par2;
       temp.dist = merge[i].dist;
       
       real32 w1 = real32(par1->size) / real32(par1->size + par2->size);
       real32 w2 = 1.0 - w1;
       temp.size = par1->size + par2->size;
      
       temp.eX  = w1*par1->eX  + w2*par2->eX;
       temp.eY  = w1*par1->eY  + w2*par2->eY;
       temp.eXX = w1*par1->eXX + w2*par2->eXX;
       temp.eYY = w1*par1->eYY + w2*par2->eYY;
       temp.eXY = w1*par1->eXY + w2*par2->eXY;
      }
     
     // par2 is going to die and its merge log is going to die with it - 
     // check it for regions before it takes its last gasp...
     // (The check saves on going hunting when no possible stable regions exist.)
      if (par2->size>=minSize) FindStableRegions(&temp,data,&forest.Get(0,0));
      
     
     // Do the actual merge - largest into smallest for reasons of the merge
     // log. Update stored stats and maintain merge log...
     {
      par1->parent = par2;
      
      *par2 = temp;
      par2->past = par1;
     }
    }
  }
  prog->Pop();
  
  // Some final steps - check its actually finished and handle the final merge
  // log...
  {
   Region * final = forest.Get(0,0).Parent();
   FindStableRegions(final,data,&forest.Get(0,0));
   log::Assert(nat32(width*height)==final->size);
  }



 // Move from stack to array for actual output...
  prog->Report(4,5);
  out.Size(data.Size());
  for (nat32 i=0;i<out.Size();i++)
  {
   out[i] = data.Peek();
   data.Pop();
  }
  log::Assert(data.Size()==0);

 prog->Pop();
}

void MSCR::FindStableRegions(Region * ml,ds::Stack<StableRegion> & output,Region * baseRegion)
{
 LogTime("eos::filter::MSCR::FindStableRegions");
 
 LogDebug("Start Search");
 
 // First pass down the merge log to remove merges that are the same dist as a
 // larger merge, so that there is always a dist change in the resulting log...
 // Calculate distDiff values at the same time.
 {
  ml->distDiff = 0.0;
  Region * prev = ml;
  Region * targ = ml->past;
  while (targ)
  {
   if (!math::Equal(prev->dist,targ->dist))
   {
    targ->distDiff = (prev->dist - targ->dist)/real32(prev->size - targ->size);
    LogDebug("Region considered" << LogDiv() << targ->dist << LogDiv() << targ->distDiff);
    
    prev->past = targ;
    prev = targ;
   }
   targ = targ->past;
  }
  prev->past = null<Region*>();
 }
 
 // Safety check, just incase...
  if (ml->past==null<Region*>()) return;



 // Do a pass over the list to leave it with only its minima dist differences,
 // delete the rest...
 // (Have code to automatically consider it lower if area difference
 // is large enough, needed for noise-less cartoon input.)
 Region * base = null<Region*>();
 {
  Region * future = ml;
  Region * targ = ml->past;
  while (targ)
  {
   bit minima = true;
   
   // Check against future region...
    if ((real32(future->size)/real32(targ->size))<areaCap)
    {
     if (future->distDiff<targ->distDiff) minima = false;
    }
   
   // Check against past region...
    if (targ->past)
    {
     if ((real32(targ->size)/real32(targ->past->size))<areaCap)
     {
      if (targ->past->distDiff<targ->distDiff) minima = false;
     }
    }
  
   // Move to next region...
    future = targ;
    targ = targ->past;
    
   // If its a maxima add it to the maxima list - this happens to destroy the
   // past list, and put them in reverse order...
    if (minima)
    {
     future->past = base;
     base = future;
    }
  }
 }



 // Do a pass over the list finding groups of minima. Groups are defined by 
 // neighbours being close enough, with chainning of relationship. Within each
 // group a best maxima will exist, this is choosen and, subject to stability
 // tests, accepted and stored in the stable maxima stack...
 {
  Region * best = base;
  while (base)
  {
   LogDebug("Beep");
   // Check if the next region is glued to this one, assuming it exists...
    if (base->past)
    {
     LogDebug("partner" << LogDiv() << (real32(base->past->size)/real32(base->size)));
     if ((real32(base->past->size)/real32(base->size))<areaCap)
     {
      // Connected - continue the chain and see if its the best member...
       if (base->past->distDiff>best->distDiff) best = base->past;
     }
     else
     {
      // Its not connected - store best of chain being finished and prep for next chain...
       StoreStableRegion(best,output,baseRegion);
       best = base->past;
     }
    }
    else
    {
     // End of the line - store the best of the last chain...
      StoreStableRegion(best,output,baseRegion);
    }

   base = base->past;
  }
 }
}

void MSCR::StoreStableRegion(Region * r,ds::Stack<StableRegion> & output,Region * baseRegion)
{
 LogTime("eos::filter::MSCR::StoreStableRegion");
  
 // Puny regions need not apply...
  if (r->size<minSize) return;

 // Create the stable region object, even though we might soon bin it...
  StableRegion sr;
  nat32 offset = r - baseRegion;
  sr.pixel[0] = offset%image.Size(0);
  sr.pixel[1] = offset/image.Size(0);

  sr.dist = r->dist + 0.5*r->distDiff;
  sr.area = r->size;

  sr.mean[0] = r->eX;
  sr.mean[1] = r->eY;

  sr.covar[0][0] = r->eXX - math::Sqr(r->eX);
  sr.covar[0][1] = r->eXY - r->eX*r->eY;
  sr.covar[1][0] = sr.covar[0][1];
  sr.covar[1][1] = r->eYY - math::Sqr(r->eY);


 // Check that the ellipsoid associated with the new region isn't too
 // thin - if it is drop it for being unstable...
  math::Mat<2,2> trash = sr.covar;
  math::Mat<2,2> q;
  math::Vect<2> d;
  math::SymEigen(trash,q,d);
  if ((d[0]>minDim)&&(d[1]>minDim))
  {
   output.Push(sr);
  }
}

/*void MSCR::FindStableRegions(Region * ml,ds::Stack<StableRegion> & output,Region * baseRegion)
{
 LogTime("eos::filter::MSCR::FindStableRegions");
  
 // Do a single reverse pass over the merge log, with a trailing pointer to match the
 // distInc value - at each node decide if its a cliff (Changes a lot) or a flat.
 // (Changes little.) At the start of each list of flats start recording the 
 // flatest length - when the flats stop the best length is then recorded as a
 // stable region, assuming that it passes various tests of goodness...
  Region * tail = ml;
  Region * targ = ml;
  bit prevFlat = false;
  
  nat32 chainStartSize = 0;
  real32 chainStartDist = 0.0;
  Region * chainBest = null<Region*>();
  real32 chainBestSlope = 0.0;
  
  while (true)
  {
   // Update the tail to be pointing at the correct node...
    while (tail->dist>targ->distInc) tail = tail->past;


   // Calculate the area change percentage and determine from that if its a 
   // flat or cliff (Have to correct for the fact distInc won't be correct.)...
    real32 areaChange = real32(tail->size)/real32(targ->size);
    areaChange *= (targ->distInc-targ->dist)/(tail->dist-targ->dist);
    bit flat = math::IsFinite(areaChange)?(areaChange<areaInc):false;
    
    LogDebug("targ" << LogDiv() << targ->size << LogDiv() << targ->dist << LogDiv() << targ->distInc);
    LogDebug("tail" << LogDiv() << tail->size << LogDiv() << tail->dist << LogDiv() << tail->distInc);
    LogDebug("State" << LogDiv() << flat << LogDiv() << areaChange);


   // Depending on its state and whats happened before handle the scenario...
    if (flat)
    {
     if (prevFlat)
     {
      //LogDebug("Chain cont");
      // Continuation of a chain - check if this is the best found so far...
       //if ((chainStartDist - targ->dist)>changeThreshold) // Don't bother if its not a big enough range.
       // **************************************************************************************************
       {
        real32 slope = real32(chainStartSize - targ->size)/(chainStartDist - targ->dist);
        LogDebug("Chain slope = " << slope);
        if (slope<chainBestSlope)
        {
         chainBest = targ;
         chainBestSlope = slope;
        }
       }
     }
     else
     {
      LogDebug("Chain start");
      // Start of a chain - initialise...
       chainStartSize = targ->size;
       chainStartDist = targ->dist;
       chainBest = null<Region*>();
       chainBestSlope = math::Infinity<real32>();
     }
    }
    else
    {
     if (prevFlat)
     {
      // Chain just ended - store its best region, if it exists and passes final checks...
       if (chainBest)
       {
        LogDebug("Chain found");
        // Fill in a stable region object ready for storage...
         StableRegion sr;
         nat32 offset = chainBest - baseRegion;
         sr.pixel[0] = offset%image.Size(0);
         sr.pixel[1] = offset/image.Size(0);
         
         sr.dist = chainBest->dist;
         sr.area = chainBest->size;
         
         sr.mean[0] = chainBest->eX;
         sr.mean[1] = chainBest->eY;
         
         sr.covar[0][0] = chainBest->eXX - math::Sqr(chainBest->eX);
         sr.covar[0][1] = chainBest->eXY - chainBest->eX*chainBest->eY;
         sr.covar[1][0] = sr.covar[0][1];
         sr.covar[1][1] = chainBest->eYY - math::Sqr(chainBest->eY);

         
        // Check that the ellipsoid associated with the new region isn't too
        // thin - if it is drop it for being unstable...
         math::Mat<2,2> trash = sr.covar;
         math::Mat<2,2> q;
         math::Vect<2> d;
         math::SymEigen(trash,q,d);
         if ((d[0]>minDim)&&(d[1]>minDim))
         {
          LogDebug("Region found!");
          // Store the stable region...
           output.Push(sr);
         }
       }
     }
     if (targ->size<minSize) break; // Little point in continuing the analysis if so.
    }


   // Move to next...
    targ = targ->past;
    if (targ==null<Region*>()) break;
    prevFlat = flat;
  }
}*/

//------------------------------------------------------------------------------
 };
};
