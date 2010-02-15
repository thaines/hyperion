//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/mya/needle_int.h"

#include "eos/ds/arrays2d.h"
#include "eos/ds/priority_queues.h"
#include "eos/file/wavefront.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
struct NeedleNode
{
 NeedleNode * Head()
 {
  if (parent)
  {
   parent = parent->Head();
   return parent;
  } else return this;
 }
 NeedleNode * parent;
 
 nat32 x,y;
 bit link[4]; // Up,right,down,left.  
 bit set; // true if its depth has been decided.
 real64 shift; // valid for heads only, used to calculate the shift to zero mean each tree.
 nat32 sum; // a head thing, number of member nodes for any given head.
};

struct NeedleLink
{
 bit operator < (const NeedleLink & rhs) const {return math::Abs(delta)<math::Abs(rhs.delta);}
 
 nat32 aX,aY,aLink; // *Link is the index of the link to set to true if merging.
 nat32 bX,bY,bLink;
 real32 delta; // b.depth = a.depth+delta
};

//------------------------------------------------------------------------------
EOS_FUNC void IntegrateNeedle(const svt::Field<bs::Normal> & needle,const svt::Field<bit> & mask,svt::Field<real32> & depth)
{
 // Construct a forest with an entry for every normal, with every normal
 // a member of its own little tree...
  ds::Array2D<NeedleNode> forest(needle.Size(0),needle.Size(1));
  for (nat32 y=0;y<forest.Height();y++)
  {
   for (nat32 x=0;x<forest.Width();x++)
   {
    NeedleNode & targ = forest.Get(x,y);
     targ.parent = null<NeedleNode*>();
     targ.x = x;
     targ.y = y;
     for (nat32 i=0;i<4;i++) targ.link[i] = false;
     targ.set = false;
     targ.shift = 0.0;
     targ.sum = 0;
   }
  }

 // Build a priority queue for every pair of 4-way neighbours, sorted by
 // the absolute delta between them...
  ds::PriorityQueue<NeedleLink> queue(forest.Width()*forest.Height()*2);
  for (nat32 y=0;y<forest.Height();y++)
  {
   for (nat32 x=0;x<forest.Width();x++)
   {
    if (mask.Get(x,y))
    {
     if ((x!=forest.Width()-1)&&mask.Get(x+1,y));
     {
      NeedleLink nl;
       nl.aX = x;
       nl.aY = y;
       nl.aLink = 1;
       nl.bX = x+1;
       nl.bY = y;
       nl.bLink = 3;
       nl.delta = 0.5*(needle.Get(x,y)[0]/needle.Get(x,y)[2] + needle.Get(x+1,y)[0]/needle.Get(x+1,y)[2]);
      queue.Add(nl); 
     }

     if ((y!=forest.Height()-1)&&mask.Get(x,y+1))
     {
      NeedleLink nl;
       nl.aX = x;
       nl.aY = y;
       nl.aLink = 2;
       nl.bX = x;
       nl.bY = y+1;
       nl.bLink = 0;
       nl.delta = 0.5*(needle.Get(x,y)[1]/needle.Get(x,y)[2] + needle.Get(x,y+1)[1]/needle.Get(x,y+1)[2]);
      queue.Add(nl);     
     }
    }
   }
  }

 // Go through the priority queue merging seperate trees, till it is empty...
  while (queue.Size()!=0)
  {
   // Get and remove the next item...
    NeedleLink nl = queue.Peek();
    queue.Rem();
    
   // If the two nodes in question have the same head skip...
    NeedleNode * aHead = forest.Get(nl.aX,nl.aY).Head();
    NeedleNode * bHead = forest.Get(nl.bX,nl.bY).Head();
    if (aHead==bHead) continue;
    
   // Arrange for one node to feed off the other...
    forest.Get(nl.aX,nl.aY).link[nl.aLink] = true;
    forest.Get(nl.bX,nl.bY).link[nl.bLink] = true;    
    
   // Merge the two trees, putting the higher one at the head...
    if (nl.delta>0.0) aHead->parent = bHead;
                 else bHead->parent = aHead;
  }


 // Select the head point in each tree, set its depth to 0.0, and then calculate
 // the depth of every other point accordingly, we do this via doing top-left to
 // bottom-right and visa versa passes until every point is set...
  // First pass to set head nodes...
   for (nat32 y=0;y<forest.Height();y++)
   {
    for (nat32 x=0;x<forest.Width();x++)
    {
     if (mask.Get(x,y))
     {
      if (forest.Get(x,y).parent==null<NeedleNode*>())
      {
       forest.Get(x,y).set = true;
       depth.Get(x,y) = 0.0;
      }
     }
     else depth.Get(x,y) = 0.0;
    }
   }

  // Iterate over the forest until every non-masked node is set...
   while (true)
   {
    // Top-left to bottom-right pass...
     bit keepGoing = false;
      for (nat32 y=0;y<forest.Height();y++)
      {
       for (nat32 x=0;x<forest.Width();x++)
       {
        if (mask.Get(x,y))
        {
         NeedleNode & targ = forest.Get(x,y);
         if (targ.set==false)
         {         
          if ((targ.link[0])&&(forest.Get(x,y-1).set))
          {
           depth.Get(x,y) = depth.Get(x,y-1) + 0.5*(needle.Get(x,y-1)[1]/needle.Get(x,y-1)[2] + needle.Get(x,y)[1]/needle.Get(x,y)[2]);
           targ.set = true;
          }
          
          if ((targ.link[3])&&(forest.Get(x-1,y).set))
          {
           depth.Get(x,y) = depth.Get(x-1,y) + 0.5*(needle.Get(x-1,y)[0]/needle.Get(x-1,y)[2] + needle.Get(x,y)[0]/needle.Get(x,y)[2]);
           targ.set = true;
          }
          
          if (targ.set==false) keepGoing = true;
         }
        }
       }
      }
     if (keepGoing==false) break;
    
    // Bottom-right to top-left pass...
     keepGoing = false;
      for (int32 y=forest.Height()-1;y>=0;y--)
      {
       for (int32 x=forest.Width();x>=0;x--)
       {
        if (mask.Get(x,y))
        {
         NeedleNode & targ = forest.Get(x,y);
         if (targ.set==false)
         {         
          if ((targ.link[1])&&(forest.Get(x+1,y).set))
          {
           depth.Get(x,y) = depth.Get(x+1,y) - 0.5*(needle.Get(x+1,y)[0]/needle.Get(x+1,y)[2] + needle.Get(x,y)[0]/needle.Get(x,y)[2]);
           targ.set = true;
          }
          
          if ((targ.link[2])&&(forest.Get(x,y+1).set))
          {
           depth.Get(x,y) = depth.Get(x,y+1) - 0.5*(needle.Get(x,y+1)[1]/needle.Get(x,y+1)[2] + needle.Get(x,y)[1]/needle.Get(x,y)[2]);
           targ.set = true;
          }
          
          if (targ.set==false) keepGoing = true;
         }
        }
       }
      }   
     if (keepGoing==false) break;
   }


 // Zero mean the depths for consistancy, on a tree by tree basis...
  // First pass to calculate depth sum for each tree...
   for (nat32 y=0;y<forest.Height();y++)
   {
    for (nat32 x=0;x<forest.Width();x++)
    {
     if (mask.Get(x,y))
     {
      NeedleNode * head = forest.Get(x,y).Head();
       head->shift += depth.Get(x,y);
       head->sum += 1;
     }
    }
   }
  
  // Second pass to convert depth sum to a shift for each tree...
   for (nat32 y=0;y<forest.Height();y++)
   {
    for (nat32 x=0;x<forest.Width();x++)
    {
     if (mask.Get(x,y)&&(forest.Get(x,y).parent==null<NeedleNode*>()))
     {
      forest.Get(x,y).shift = -forest.Get(x,y).shift/real32(forest.Get(x,y).sum);
     }
    }
   }
     
  // Third pass to apply the shift to each node...
   for (nat32 y=0;y<forest.Height();y++)
   {
    for (nat32 x=0;x<forest.Width();x++)
    {
     if (mask.Get(x,y))
     {
      depth.Get(x,y) += forest.Get(x,y).Head()->shift;
     }
    }
   }
   
   
  // Final smoothing passes, to remove the 'extra-edges' affect that is formed by this techneque...
   /*for (nat32 i=0;i<64;i++)
   {
    // Copy the depths into the shifts...
     for (nat32 y=0;y<forest.Height();y++)
     {
      for (nat32 x=0;x<forest.Width();x++)
      {
       if (mask.Get(x,y))
       {
        forest.Get(x,y).shift = depth.Get(x,y);
       }
      }
     }
    
    // Set the depths as calculated as the average of suposed depth from there
    // valid neighbours...
     for (nat32 y=0;y<forest.Height();y++)
     {
      for (nat32 x=0;x<forest.Width();x++)
      {
       if (mask.Get(x,y))
       {
        depth.Get(x,y) = 0.0;
        nat32 sourceSum = 0;
       
        if (mask.Get(x-1,y))
        {
         depth.Get(x,y) += forest.Get(x-1,y).shift + 0.5*(needle.Get(x-1,y)[0]/needle.Get(x-1,y)[2] + needle.Get(x,y)[0]/needle.Get(x,y)[2]);
         ++sourceSum;
        }
       
        if (mask.Get(x+1,y)) 
        {
         depth.Get(x,y) += forest.Get(x+1,y).shift - 0.5*(needle.Get(x+1,y)[0]/needle.Get(x+1,y)[2] + needle.Get(x,y)[0]/needle.Get(x,y)[2]);
         ++sourceSum;
        }

        if (mask.Get(x,y-1))
        {
         depth.Get(x,y) += forest.Get(x,y-1).shift + 0.5*(needle.Get(x,y-1)[1]/needle.Get(x,y-1)[2] + needle.Get(x,y)[1]/needle.Get(x,y)[2]);
         ++sourceSum;
        }
       
        if (mask.Get(x,y+1)) 
        {
         depth.Get(x,y) += forest.Get(x,y+1).shift - 0.5*(needle.Get(x,y+1)[1]/needle.Get(x,y+1)[2] + needle.Get(x,y)[1]/needle.Get(x,y)[2]);
         ++sourceSum;
        }
       
        if (sourceSum!=0) depth.Get(x,y) /= real32(sourceSum);
       }
      }
     }
   }*/
}

//------------------------------------------------------------------------------
EOS_FUNC bit SaveNeedleModel(const svt::Field<bs::Normal> & needle,const svt::Field<bit> & mask,nat32 freq,cstrconst fn,bit overwrite)
{
 // Tempory storage...
  svt::Var tv(needle);
   real32 depthIni = 0.0;
   tv.Add("depth",depthIni);
  tv.Commit(false);
  
  svt::Field<real32> depth(&tv,"depth");
 
 // Integrate to generate the depth map...
  IntegrateNeedle(needle,mask,depth);
 
 // Write the depth map into a Wavefront object, with the given sampling...
  file::Wavefront wf;
  ds::Array2D<nat32> vertInd(needle.Size(0)/freq,needle.Size(1)/freq);
  ds::Array2D<nat32> normInd(needle.Size(0)/freq,needle.Size(1)/freq);
  
  for (nat32 y=0;y<vertInd.Height();y++)
  {
   for (nat32 x=0;x<vertInd.Width();x++)
   {
    bs::Vert vert;
     vert.X() = (real32(x) - 0.5*real32(vertInd.Width()))*real32(freq);
     vert.Y() = (real32(y) - 0.5*real32(vertInd.Height()))*real32(freq);
     vert.Z() = -depth.Get(x*freq,y*freq);
    vertInd.Get(x,y) = wf.Add(vert);
    
    normInd.Get(x,y) = wf.Add(needle.Get(x*freq,y*freq));
   }
  }
  
  for (nat32 y=0;y<vertInd.Height()-1;y++)
  {
   for (nat32 x=0;x<vertInd.Width()-1;x++)
   {
    if (mask.Get((x)*freq,(y)*freq)&&
        mask.Get((x)*freq,(y+1)*freq)&&
        mask.Get((x+1)*freq,(y+1)*freq)&&
        mask.Get((x+1)*freq,(y)*freq))
    {
     wf.Add(vertInd.Get(x,y),normInd.Get(x,y));
     wf.Add(vertInd.Get(x,y+1),normInd.Get(x,y+1));
     wf.Add(vertInd.Get(x+1,y+1),normInd.Get(x+1,y+1));
     wf.Add(vertInd.Get(x+1,y),normInd.Get(x+1,y));
     wf.Face();
    }
   }
  }
 
 // Save it and return the result...  
  return wf.Save(fn,overwrite);
}

//------------------------------------------------------------------------------
 };
};
