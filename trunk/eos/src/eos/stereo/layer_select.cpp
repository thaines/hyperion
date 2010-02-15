//------------------------------------------------------------------------------
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

#include "eos/stereo/layer_select.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
LayerSelect::LayerSelect()
:layerMaker(null<LayerMaker*>()),
segCount(0),occCost(0.1),disCost(0.1),
checkRuns(12)
{}

LayerSelect::~LayerSelect()
{}

void LayerSelect::SetMaker(LayerMaker * lm)
{
 layerMaker = lm;
}

void LayerSelect::SetSegs(nat32 sc,const svt::Field<nat32> & s)
{
 segCount = sc;
 segs = s;
}

void LayerSelect::SetImages(const svt::Field<bs::ColourRGB> & l,const svt::Field<bs::ColourRGB> & r)
{
 left = l;
 right = r;
}

void LayerSelect::SetMasks(const svt::Field<bit> & lm,const svt::Field<bit> & rm)
{
 leftMask = lm;
 rightMask = rm;
}

void LayerSelect::SetCosts(real32 occ,real32 dis)
{
 occCost = occ;
 disCost = dis;
}

void LayerSelect::SetBailOut(nat32 bo)
{
 checkRuns = bo;
}

bit LayerSelect::Run(time::Progress * prog)
{
 prog->Push();

 // Setup a data structure for the fast rendering of individual segments...
  sed = new Node*[segCount];
  for (nat32 i=0;i<segCount;i++) sed[i] = null<Node*>();

  for (nat32 y=0;y<left.Size(1);y++)
  {
   for (int32 x=left.Size(0)-1;x>=0;x--)
   {
    if (leftMask.Get(x,y)==false) continue;
    Node * nn = new Node;

    nat32 s = segs.Get(x,y);
    nn->next = sed[s];
    sed[s] = nn;

    nn->x = x;
    nn->y = y;
    nn->colour = left.Get(x,y);
    nn->paired = (nn->next!=null<Node*>())&&(nn->next->x==nat32(x+1))&&(nn->next->y==y);
    if (nn->paired)
    {
     nn->incStart = true;
     nn->next->incStart = false;
     if (nn->next->paired) nn->incEnd = false;
                      else nn->incEnd = true;
    }
   }
  }


 // Initialise and build the warp image, with all the segments/layers...
  WarpInc warp(right,segCount);
  warp.SetMask(rightMask);
  warp.Weights(occCost);
  for (nat32 i=0;i<segCount;i++) RenderSegment(warp,i);

 // Create a graph so we can iterate the adjacent segments of any one segment quickly.
  filter::SegGraph segGraph(segs,segCount);

 // Create a discontinuity count variable, which we incrimentally update as we go,
 // fill it with the starting discontinuity...
  nat32 discCount = 0;
  for (nat32 i=0;i<segGraph.Segments();i++)
  {
   for (nat32 j=0;j<segGraph.NeighbourCount(i);j++)
   {
    if ((i<segGraph.Neighbour(i,j))&&(layerMaker->SegToLayer(i)!=layerMaker->SegToLayer(segGraph.Neighbour(i,j))))
    {
     discCount += segGraph.BorderSize(i,j);
    }
   }
  }

 // A tempory lump of memory used below for keeping track of which layers we need to try...
  bit * layF = new bit[layerMaker->Layers()];

 // A tempory lump of memory, this once for tracking the new layer for each segment, we have
 // two, due to tracking the last few and using the last best when no improvment happens for x.
  real32 lowestScore = warp.Score() + discCount*disCost;
  nat32 * bestLay = new nat32[segCount];
  for (nat32 i=0;i<segCount;i++) bestLay[i] = layerMaker->SegToLayer(i);
  bit ret = false;

  nat32 * useLay = new nat32[segCount];
  nat32 bailOut = 0;


 // Iterate until no changes found...
  while (bailOut<checkRuns)
  {
   // For each segment try layer re-alignments, set it to the one which reduces
   // the cost the most...
    for (nat32 i=0;i<segCount;i++)
    {
     // Work out which layers we need to try...
      for (nat32 j=0;j<layerMaker->Layers();j++) layF[j] = false;
      for (nat32 j=0;j<segGraph.NeighbourCount(i);j++)
      {
       layF[layerMaker->SegToLayer(segGraph.Neighbour(i,j))] = true;
      }
      layF[layerMaker->SegToLayer(i)] = false;

     // For each layer check if its an improvment on the last...
      nat32 startLayer = layerMaker->SegToLayer(i);
      nat32 bestLayer = startLayer;
      real32 bestScore = warp.Score() + discCount*disCost;

      for (nat32 j=0;j<layerMaker->Layers();j++)
      {
       if (layF[j])
       {
	// We have a layer to try out, update the warp image as required...
	 warp.Remove(i);
	 discCount += DiscDelta(segGraph,i,j);
	 layerMaker->SegToLayer(i) = j;
	 RenderSegment(warp,i);

	// If the score has improved, record it, otherwise we don't give a dam...
	 real32 score = warp.Score() + discCount*disCost;

	 if (score<bestScore)
         {
	  bestLayer = j;
	  bestScore = score;
         }
       }
      }

     // Store the change for future application and revert to the starting state...
      useLay[i] = bestLayer;
      if (layerMaker->SegToLayer(i)!=startLayer)
      {
       warp.Remove(i);
       discCount += DiscDelta(segGraph,i,startLayer);
       layerMaker->SegToLayer(i) = startLayer;
       RenderSegment(warp,i);
      }
    }
    prog->Report(segCount,segCount);


   // Update the segments with there new layer allocations...
    for (nat32 i=0;i<segCount;i++)
    {
     if (layerMaker->SegToLayer(i)!=useLay[i])
     {
      warp.Remove(i);
      discCount += DiscDelta(segGraph,i,useLay[i]);
      layerMaker->SegToLayer(i) = useLay[i];
      RenderSegment(warp,i);
     }
    }

   // Check if the score is an improvment, if so store it, if not indicate
   // were getting close to bailing out...
    real32 finScore = warp.Score() + discCount*disCost;
    if (lowestScore<=finScore) bailOut++;
    else
    {
     // We do this as rounding differences between addition and subtraction can result
     // in infinite 'improvment'...
      if (math::Equal(lowestScore,finScore)) bailOut++;
                                        else bailOut = 0;

     for (nat32 i=0;i<segCount;i++) bestLay[i] = useLay[i];
     lowestScore = finScore;
     ret = true;
    }
  }


 // Set our output to the best score found...
  for (nat32 i=0;i<segCount;i++) layerMaker->SegToLayer(i) = bestLay[i];


 // Clean up...
  delete[] useLay;
  delete[] layF;

  for (nat32 i=0;i<segCount;i++)
  {
   Node * targ = sed[i];
   while (targ)
   {
    Node * victim = targ;
    targ = targ->next;
    delete victim;
   }
  }
  delete[] sed;

 prog->Pop();
 return ret;
}

void LayerSelect::RenderSegment(WarpInc & warp,nat32 segment)
{
 Node * targ = sed[segment];
 const bs::PlaneABC & plane = layerMaker->Plane(layerMaker->SegToLayer(segment));
 nat32 width = warp.Width();
 while (targ)
 {
  if (targ->paired)
  {
   // Find the start and end points...
    real32 start = targ->x + plane.Z(targ->x,targ->y);
    real32 end = targ->x + 1.0 + plane.Z(targ->x+1,targ->y);

    real32 mult = 1.0/(end-start);
    real32 base = start;

    if (targ->incStart) start = targ->x-0.5 + plane.Z(targ->x-0.5,targ->y);
    if (targ->incEnd) end = targ->x + 1.5 + plane.Z(targ->x+1.5,targ->y);
    if (start>end) {real32 temp = start; start = end; end = temp;}

   // For each integer in the range interpolate a colour and write...
    for (nat32 i=math::Max<int32>(int32(math::RoundUp(start)),0);(real32(i)<end)&&(i<width);i++)
    {
     real32 t = (real32(i)-base)*mult;
     bs::ColourRGB colour;
      colour.r = (1.0-t)*targ->colour.r + t*targ->next->colour.r;
      colour.g = (1.0-t)*targ->colour.g + t*targ->next->colour.g;
      colour.b = (1.0-t)*targ->colour.b + t*targ->next->colour.b;

     warp.Add(i,targ->y,plane.Z(targ->x+t,targ->y),colour,segment);
    }
  }
  targ = targ->next;
 }
}

int32 LayerSelect::DiscDelta(filter::SegGraph & segGraph,nat32 seg,nat32 toLayer)
{
 nat32 fromLayer = layerMaker->SegToLayer(seg);

 int32 ret = 0;
  for (nat32 i=0;i<segGraph.NeighbourCount(seg);i++)
  {
   bit wasEqual = layerMaker->SegToLayer(segGraph.Neighbour(seg,i))==fromLayer;
   bit tobeEqual = layerMaker->SegToLayer(segGraph.Neighbour(seg,i))==toLayer;
   if (wasEqual!=tobeEqual)
   {
    if (wasEqual)
    {
     ret += segGraph.BorderSize(seg,i);
    }
    else
    {
     ret -= segGraph.BorderSize(seg,i);
    }
   }
  }
 return ret;
}

//------------------------------------------------------------------------------
 };
};
