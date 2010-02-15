//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/filter/segmentation.h"

#include "eos/mem/alloc.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
void Segmenter::SetMergeWeight(const svt::Field<real32> & w,real32 c)
{
 doWeight = true;
 weightMax = c;
 weight = w;
}

void Segmenter::AddField(const svt::Field<real32> & in,real32 scale)
{
 nat32 ind = feat.Size();
 feat.Size(ind+1);
 feat[ind].field = in;
 feat[ind].scale = scale;
}

void Segmenter::Run(time::Progress * prog)
{
 prog->Push();
 prog->Report(0,4+avs);

 // We construct a forest, where each node of the forest represents a pixel,
 // and each head of a tree represents a cluster. At each node we store the 
 // relevent feature vector and its weight (Number of pixels in it.). As 
 // nodes are merged together the head node is allways made to contain the 
 // relevent average vector and cluster size.
  width = feat[0].field.Size(0);
  height = feat[0].field.Size(1);
  nodeCount = width * height;
  nodeSize = sizeof(Node) - sizeof(real32) + sizeof(real32)*feat.Size();
  forest = (Node*)mem::Malloc<byte>(nodeCount*nodeSize);

  Node * targ = forest;
  for (nat32 y=0;y<height;y++)
  {
   for (nat32 x=0;x<width;x++)
   {
    targ->parent = null<Node*>();
    targ->newParent = null<Node*>();
    targ->distance = 1e100;
    targ->stop = false;
    targ->segment = 0;
    targ->weight = 1;
    if (doWeight)
    {
     targ->edge = weight.Get(x,y);
     targ->next = null<Node*>();
     targ->xPos = x;
     targ->yPos = y;
    }
    for (nat32 i=0;i<feat.Size();i++)
    {
     targ->fv[i] = feat[i].field.Get(x,y)*feat[i].scale;
    }

    (byte*&)targ += nodeSize;
   }
  }


 // Do the first merge...
  prog->Report(1,4+avs);
   if (doWeight) MergeWeighted();
            else MergeBasic();
  prog->Report(2,4+avs);

 // If needed do the averaging steps...
  for (nat32 i=0;i<avs;i++)
  {   
   AverageFeatures();
   if (doWeight) MergeWeighted();
            else MergeBasic();
   prog->Report(3+i,4+avs);
  }


 // Merge the small once...
  if (minMerge!=0) MergeSmallOnce();


 // Kill the still-too-small once...
  Node * superHead = null<Node*>();
  if (minMerge<minKill) superHead = AggregateSmallOnce();
  prog->Report(3+avs,4+avs);


 // And the final step - assign cluster numbers, and extract them at the same time
 // to produce the output segmentation...
 // (If running in trigger happy mode make segment 0 the segment for the 
 // sufferers of flesh wounds.)
  segments = 0;
  if (minMerge<minKill)
  {
   ++segments;
   if (superHead) superHead->segment = segments;
  }
  
  segs = mem::Malloc<nat32>(nodeCount);
  targ = forest;
  for (nat32 i=0;i<nodeCount;i++)
  {
   Node * head = targ->Head();
   if (head->segment==0) 
   {
    ++segments;
    head->segment = segments;
   }
   segs[i] = head->segment - 1;
   (byte*&)targ += nodeSize;
  }


 // Bring in the Cleaner...
  mem::Free((byte*)forest);

 prog->Report(4+avs,4+avs);
 prog->Pop();
}

void Segmenter::GetOutput(svt::Field<nat32> & out) const
{
 out.CopyFrom(segs,sizeof(nat32));
}

void Segmenter::GetOutput(ds::Array2D<nat32> & out) const
{
 out.Resize(width,height);
 nat32 * targ = segs;
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   out.Get(x,y) = *targ;
   ++targ;
  }
 }
}

//------------------------------------------------------------------------------
void Segmenter::MergeBasic()
{
 // Now iterativly merge - we do this by iterating all Nodes and checking 
 // there neighbours, and merging with the closest neighbour if there is
 // one within the cutoff. When an iteration with no change happens we stop.
  while (true)
  {
   // First pass, select neighbours to cluster with for each node.
    Node * targ = forest;
    for (nat32 y=0;y<height;y++)
    {
     for (nat32 x=0;x<width;x++)
     {
      if (!targ->stop)
      {
       Node * head = targ->Head();

       Node * nearHead = 0;
       Node * nearNode = 0;
       real32 nearDist = 1e100; // distance squared in fact, for speed.

       // Find the minimum distance to the 4 neighbours, ignoring once of the same cluster...
        // Left...
         if (x!=0)
         {
          Node * tempNode = (Node*)((byte*)targ - nodeSize);
          Node * tempHead = tempNode->Head();
          if (tempHead!=head)
          {
           real32 tempDist = 0.0;
           for (nat32 i=0;i<feat.Size();i++)
           {
            tempDist += (targ->fv[i]-tempNode->fv[i])*(targ->fv[i]-tempNode->fv[i]);
           }
           if (tempDist<nearDist)
           {
            nearHead = tempHead;
            nearNode = tempNode;
            nearDist = tempDist;         
           }
          }
         }

        // Right...
         if (x!=(width-1))
         {
          Node * tempNode = (Node*)((byte*)targ + nodeSize);
          Node * tempHead = tempNode->Head();
          if (tempHead!=head)
          {
           real32 tempDist = 0.0;
           for (nat32 i=0;i<feat.Size();i++)
           {
            tempDist += (targ->fv[i]-tempNode->fv[i])*(targ->fv[i]-tempNode->fv[i]);
           }
           if (tempDist<nearDist)
           {
            nearHead = tempHead;
            nearNode = tempNode;
            nearDist = tempDist;         
           }
          }
         }

        // Up...
         if (y!=0)
         {
          Node * tempNode = (Node*)((byte*)targ - nodeSize*width);
          Node * tempHead = tempNode->Head();
          if (tempHead!=head)
          {
           real32 tempDist = 0.0;
           for (nat32 i=0;i<feat.Size();i++)
           {
            tempDist += (targ->fv[i]-tempNode->fv[i])*(targ->fv[i]-tempNode->fv[i]);
           }
           if (tempDist<nearDist)
           {
            nearHead = tempHead;
            nearNode = tempNode;
            nearDist = tempDist;         
           }
          }         
         }
 
        // Down...
         if (y!=(height-1))
         {
          Node * tempNode = (Node*)((byte*)targ + nodeSize*width);
          Node * tempHead = tempNode->Head();
          if (tempHead!=head)
          {
           real32 tempDist = 0.0;
           for (nat32 i=0;i<feat.Size();i++)
           {
            tempDist += (targ->fv[i]-tempNode->fv[i])*(targ->fv[i]-tempNode->fv[i]);
           }
           if (tempDist<nearDist)
           {
            nearHead = tempHead;
            nearNode = tempNode;
            nearDist = tempDist;         
           }
          }
         }
 
       // If it is null then we are surrounded by nodes of the same cluster - stop proccessing this one, permanantly...
        if (nearNode==0) targ->stop = true;
        else
        {
         // If the nearest cluster is within the cutoff then merge...
          if (nearDist<(cutoff*cutoff))
          {
           targ->newParent = nearHead;
          }
        }
       }
      (byte*&)targ += nodeSize;
     }
    }

   // Second pass, merge clusters that have been marked for the merging.
   // Also, detect when its time to finish...
    targ = forest;
    bit wibble = true;
     for (nat32 i=0;i<nodeCount;i++)
     {
      if (targ->newParent)
      {
       Node * head1 = targ->Head();
       Node * head2 = targ->newParent->Head();
       targ->newParent = null<Node*>();
 
       if (head1!=head2)
       {
        head1->weight = head1->weight + head2->weight;
        head2->parent = head1;
        wibble = false;
       }    
      }
      (byte*&)targ += nodeSize; 
     }
    if (wibble) break;
  }
}

void Segmenter::MergeWeighted()
{
 // Now iterativly merge - we do this by iterating all Nodes and checking 
 // there neighbours, and merging with the closest neighbour if there is
 // one within the cutoff. When an iteration with no change happens we stop.
  while (true)
  {
   // First pass, select neighbours to cluster with for each node.
    Node * targ = forest;
    for (nat32 y=0;y<height;y++)
    {
     for (nat32 x=0;x<width;x++)
     {
      if (!targ->stop)
      {
       Node * head = targ->Head();

       Node * nearHead = 0;
       Node * nearNode = 0;
       real32 nearDist = 1e100; // distance squared in fact, for speed.

       // Find the minimum distance to the 4 neighbours, ignoring once of the same cluster...
        // Left...
         if (x!=0)
         {
          Node * tempNode = (Node*)((byte*)targ - nodeSize);
          Node * tempHead = tempNode->Head();
          if (tempHead!=head)
          {
           real32 tempDist = 0.0;
           for (nat32 i=0;i<feat.Size();i++)
           {
            tempDist += (targ->fv[i]-tempNode->fv[i])*(targ->fv[i]-tempNode->fv[i]);
           }
           if (tempDist<nearDist)
           {
            nearHead = tempHead;
            nearNode = tempNode;
            nearDist = tempDist;         
           }
          }
         }

        // Right...
         if (x!=(width-1))
         {
          Node * tempNode = (Node*)((byte*)targ + nodeSize);
          Node * tempHead = tempNode->Head();
          if (tempHead!=head)
          {
           real32 tempDist = 0.0;
           for (nat32 i=0;i<feat.Size();i++)
           {
            tempDist += (targ->fv[i]-tempNode->fv[i])*(targ->fv[i]-tempNode->fv[i]);
           }
           if (tempDist<nearDist)
           {
            nearHead = tempHead;
            nearNode = tempNode;
            nearDist = tempDist;         
           }
          }
         }

        // Up...
         if (y!=0)
         {
          Node * tempNode = (Node*)((byte*)targ - nodeSize*width);
          Node * tempHead = tempNode->Head();
          if (tempHead!=head)
          {
           real32 tempDist = 0.0;
           for (nat32 i=0;i<feat.Size();i++)
           {
            tempDist += (targ->fv[i]-tempNode->fv[i])*(targ->fv[i]-tempNode->fv[i]);
           }
           if (tempDist<nearDist)
           {
            nearHead = tempHead;
            nearNode = tempNode;
            nearDist = tempDist;         
           }
          }         
         }
 
        // Down...
         if (y!=(height-1))
         {
          Node * tempNode = (Node*)((byte*)targ + nodeSize*width);
          Node * tempHead = tempNode->Head();
          if (tempHead!=head)
          {
           real32 tempDist = 0.0;
           for (nat32 i=0;i<feat.Size();i++)
           {
            tempDist += (targ->fv[i]-tempNode->fv[i])*(targ->fv[i]-tempNode->fv[i]);
           }
           if (tempDist<nearDist)
           {
            nearHead = tempHead;
            nearNode = tempNode;
            nearDist = tempDist;         
           }
          }
         }
 
       // If it is null then we are surrounded by nodes of the same cluster - stop proccessing this one, permanantly...
        if (nearNode==0) targ->stop = true;
        else
        {
         // If the nearest cluster is within the cutoff then merge...
          if (nearDist<(cutoff*cutoff))
          {
           targ->newParent = nearHead;
          }
        }
       }
      (byte*&)targ += nodeSize;
     }
    }

   // Second pass, merge clusters that have been marked for the merging
   // that pass the weighting test.
   // Also, detect when its time to finish...
    targ = forest;
    bit wibble = true;
     for (nat32 i=0;i<nodeCount;i++)
     {
      if (targ->newParent)
      {
       Node * head1 = targ->Head();
       Node * head2 = targ->newParent->Head();
       targ->newParent = null<Node*>();
 
       if (head1!=head2)
       {
        // Before continuing we do the weight test, to do this we maintain
        // a linked list in this version of the algorithm of all nodes in 
        // a segment, so we can iterate all members very quckly. The list 
        // allways starts at the head.
         // Iterate the 2 lists to determine the mean weighting...
          real32 ws = 0.0;
          nat32 n = 0;

          Node * t = head1;
          while (t)
          {
           if (!t->stop)
           {
            nat32 ng = 0;
             if ((t->xPos!=0)&&(((Node*)((byte*)t - nodeSize))->Head()==head2)) ++ng;
             if ((t->xPos!=(width-1))&&(((Node*)((byte*)t + nodeSize))->Head()==head2)) ++ng;
             if ((t->yPos!=0)&&(((Node*)((byte*)t - nodeSize*width))->Head()==head2)) ++ng;
             if ((t->yPos!=(height-1))&&(((Node*)((byte*)t + nodeSize*width))->Head()==head2)) ++ng;
            ws += ng*t->edge;
            n += ng;
           }
           t = t->next;
          }

          t = head2;
          while (t)
          {
           if (!t->stop)
           {
            nat32 ng = 0;
             if ((t->xPos!=0)&&(((Node*)((byte*)t - nodeSize))->Head()==head1)) ++ng;
             if ((t->xPos!=(width-1))&&(((Node*)((byte*)t + nodeSize))->Head()==head1)) ++ng;
             if ((t->yPos!=0)&&(((Node*)((byte*)t - nodeSize*width))->Head()==head1)) ++ng;
             if ((t->yPos!=(height-1))&&(((Node*)((byte*)t + nodeSize*width))->Head()==head1)) ++ng;
            ws += ng*t->edge;
            n += ng;
           }
           t = t->next;
          }

          ws /= real32(n);

         // If the weighting is satisfactory we merge, making sure to 
         // maintain the linked list...
          if (ws>weightMax)
          {
           head1->Last() = head2; // Could be optimised by setting the smaller segment as parent, just can't be arsed to code at time of writting.
           head1->weight = head1->weight + head2->weight;
           head2->parent = head1;
           wibble = false;
          }
       }    
      }
      (byte*&)targ += nodeSize; 
     }
    if (wibble) break;
  }
}

void Segmenter::AverageFeatures()
{
 // We need to set every pixel to the average of its segment of pixels. We do this 
 // in 3 passes, first we sum into the head of each cluster all the features, then
 // we do a second pass to divide through by the numbers of pixels in each cluster
 // and a final third pass to do the setting...
  // Pass 1 - summation...
   Node * targ = forest;
   for (nat32 y=0;y<height;y++)
   {
    for (nat32 x=0;x<width;x++)
    {
     Node * head = targ->Head();
     if (head!=targ)
     {
      for (nat32 i=0;i<feat.Size();i++)
      {
       head->fv[i] += targ->fv[i];
      }
     }
     (byte*&)targ += nodeSize;
    }
   }

  // Pass 2 - division...
   targ = forest;
   for (nat32 y=0;y<height;y++)
   {
    for (nat32 x=0;x<width;x++)
    {
     Node * head = targ->Head();
     if (head==targ)
     {
      real32 we = 1.0/head->weight;
      for (nat32 i=0;i<feat.Size();i++)
      {
       head->fv[i] *= we;
      }
     }
     (byte*&)targ += nodeSize;
    }
   }

  // Pass 3 - setting...
   targ = forest;
   for (nat32 y=0;y<height;y++)
   {
    for (nat32 x=0;x<width;x++)
    {
     Node * head = targ->Head();
     if (head!=targ)
     {
      for (nat32 i=0;i<feat.Size();i++)
      {
       targ->fv[i] = head->fv[i];
      }
     }
     (byte*&)targ += nodeSize;
    }
   }
}

void Segmenter::MergeSmallOnce()
{
 // Now iterate and forcefully merge all segments smaller than 'minimum' with there
 // nearest neighbour. We do this by storing the best match at the head of each cluster
 // smaller than the limit in the first pass then merging in the second. We iterate till
 // all clusters are bigger than or equal to the given limit.
 // (This is very similar, though subtly different, to above.)
  while (true)
  {
   // First pass, select neighbours to cluster with for each node, storing the 
   // closest node with the head.
    Node * targ = forest;
    for (nat32 y=0;y<height;y++)
    {
     for (nat32 x=0;x<width;x++)
     {
      if (!targ->stop)
      {
       Node * head = targ->Head();
       if (head->weight<minMerge)
       {
        // Find the minimum distance to the 4 neighbours, ignoring once of the same cluster...
         // Left...
          if (x!=0)
          {
           Node * tempNode = (Node*)((byte*)targ - nodeSize);
           Node * tempHead = tempNode->Head();
           if (tempHead!=head)
           {
            real32 tempDist = 0.0;
            for (nat32 i=0;i<feat.Size();i++)
            {  
             tempDist += (targ->fv[i]-tempNode->fv[i])*(targ->fv[i]-tempNode->fv[i]);
            }
            if (tempDist<head->distance)
            {
             head->newParent = tempHead;
             head->distance = tempDist;         
            }
           }
          }

         // Right...
          if (x!=(width-1))
          {
           Node * tempNode = (Node*)((byte*)targ + nodeSize);
           Node * tempHead = tempNode->Head();
           if (tempHead!=head)
           {
            real32 tempDist = 0.0;
            for (nat32 i=0;i<feat.Size();i++)
            {    
             tempDist += (targ->fv[i]-tempNode->fv[i])*(targ->fv[i]-tempNode->fv[i]);
            }
            if (tempDist<head->distance)
            {
             head->newParent = tempHead;
             head->distance = tempDist;         
            }
           }
          }

         // Up...
          if (y!=0)
          {
           Node * tempNode = (Node*)((byte*)targ - nodeSize*width);
           Node * tempHead = tempNode->Head();
           if (tempHead!=head)
           {
            real32 tempDist = 0.0;
            for (nat32 i=0;i<feat.Size();i++)
            {  
             tempDist += (targ->fv[i]-tempNode->fv[i])*(targ->fv[i]-tempNode->fv[i]);
            }
            if (tempDist<head->distance)
            {
             head->newParent = tempHead;
             head->distance = tempDist;         
            }
           }         
          }
  
         // Down...
          if (y!=(height-1))
          {
           Node * tempNode = (Node*)((byte*)targ + nodeSize*width);
           Node * tempHead = tempNode->Head();
           if (tempHead!=head)
           {
            real32 tempDist = 0.0;
            for (nat32 i=0;i<feat.Size();i++)
            {  
             tempDist += (targ->fv[i]-tempNode->fv[i])*(targ->fv[i]-tempNode->fv[i]);
            }
            if (tempDist<head->distance)
            {
             head->newParent = tempHead;
             head->distance = tempDist;         
            }
           }
          }
       }
      }
      (byte*&)targ += nodeSize;
     }
    }

   // Second pass, merge clusters that have been marked for the merging, and do some prep
   // for the next round. Also, detect when its time to finish...
    targ = forest;
    bit wibble = true;
     for (nat32 i=0;i<nodeCount;i++)
     {
      if (targ->newParent)
      {
       Node * head1 = targ->Head();
       Node * head2 = targ->newParent->Head();
       targ->newParent = null<Node*>();
       targ->distance = 1e100;

       if (head1!=head2)
       {
        nat32 newWeight = head1->weight + head2->weight;
        if (newWeight<minMerge) wibble = false;      
        head1->weight = newWeight;
        head2->parent = head1;
       }    
      }
      (byte*&)targ += nodeSize; 
     }
    if (wibble) break;
  }
}

Segmenter::Node * Segmenter::AggregateSmallOnce()
{
 Node * ret = null<Node*>();
  Node * targ = forest;
  for (nat32 i=0;i<nodeCount;i++)
  {
   if ((targ->parent==null<Node*>())&&(targ->weight<minKill))
   {
    if (ret)
    {
     targ->parent = ret;
    }
    else
    {
     ret = targ;
    }
   }
   
   (byte*&)targ += nodeSize; 
  }
 if (ret) ret = ret->Head();
 return ret;
}

//------------------------------------------------------------------------------
 };
};
