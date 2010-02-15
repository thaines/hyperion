//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/mya/layer_merge.h"

#include "eos/ds/arrays.h"
#include "eos/ds/priority_queues.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
LayerMerge::LayerMerge(const svt::Field<nat32> & s,Layers & l,LayerScore & ls)
:segs(s),layers(l),layerScore(ls),bias(0.0)
{}

LayerMerge::~LayerMerge()
{}

void LayerMerge::SetBias(real32 b)
{
 bias = b;
}

void LayerMerge::operator () (time::Progress * prog) const
{
 prog->Push();
 prog->Report(0,3);

 // Create the array of head version numbers, so versions of segments can be
 // tracked and old merges on the heap ignored...
  ds::Array<nat32> headVer(layers.SegmentCount());
  for (nat32 i=0;i<headVer.Size();i++) headVer[i] = 0;

 // Build a graph structure, to track the current state so the new merges 
 // after applying a merge can be generated efficiently. An array mapping 
 // from segment to graph node is provided, and each graph vertex has a 
 // relevent layer head number associated with it...
  ds::Graph graph;
  ds::Array<Vertex*> segToVert(layers.SegmentCount());
  // Make vertices...
   for (nat32 i=0;i<layers.SegmentCount();i++)
   {
    if (layers.SegToLayer(i)==i)
    {
     segToVert[i] = new Vertex(graph);
     segToVert[i]->layer = i;
    }
   }
   
  // Fill in remaining segToVert links...
   for (nat32 i=0;i<layers.SegmentCount();i++)
   {
    segToVert[i] = segToVert[layers.SegToLayer(i)];
   } 
   
  // Make links between vertices...
   nat32 edgeCount = 0; // Just so the heap can be reasonably initialised.
   for (nat32 y=0;y<segs.Size(1);y++)
   {
    for (nat32 x=0;x<segs.Size(0);x++)
    {
     // Back...
      if (x!=0)
      {
       nat32 here = segs.Get(x,y);
       nat32 there = segs.Get(x-1,y);
       if (here!=there)
       {
        Vertex * hv = segToVert[here];
        Vertex * tv = segToVert[there];
        if (hv!=tv)
        {
         if (hv->Connection(tv)==null<ds::Edge*>())
         {
          new ds::Edge(graph,hv,tv);
         }
        }
       }
      }
      
     // Up...
      if (y!=0)
      {
       nat32 here = segs.Get(x,y);
       nat32 there = segs.Get(x,y-1);
       if (here!=there)
       {
        Vertex * hv = segToVert[here];
        Vertex * tv = segToVert[there];
        if (hv!=tv)
        {
         if (hv->Connection(tv)==null<ds::Edge*>())
         {
          new ds::Edge(graph,hv,tv);
         }
        }
       }      
      }
    }
   }


 // Build a heap with all the initial merge operations...
  prog->Report(1,3);
  prog->Push();
  nat32 progPos = 0;
  ds::PriorityQueue<Node> heap(edgeCount*2);
  ds::List<ds::Vertex*>::Cursor vTarg = graph.VertexList();
  while (!vTarg.Bad())
  {
   prog->Report(progPos++,graph.Vertices());
   ds::List<ds::Edge*>::Cursor eTarg = (*vTarg)->EdgeList();   
   while (!eTarg.Bad())
   {
    Vertex * here = static_cast<Vertex*>(*vTarg);
    Vertex * there = static_cast<Vertex*>((*eTarg)->From());
    if (there==here) there = static_cast<Vertex*>((*eTarg)->To());
    
    if (here<there) // So we only add each pair once, to compensate for getting each edge twice.
    {
     Node node;
      node.seg1 = here->layer;
      node.seg2 = there->layer;
      node.cost = layerScore.IfMergeLayers(node.seg1,node.seg2);
      node.seg1ver = headVer[node.seg1];
      node.seg2ver = headVer[node.seg2];   
     if (node.cost<bias) heap.Add(node);
    }
    
    ++eTarg;
   }
   ++vTarg;
  }
  prog->Pop();
  

 // Keep poping the best merge off the heap, applying it, and adding all the
 // new merges created by it...
  prog->Report(2,3);
  prog->Push();
  nat32 progSize = heap.Size();
  while (heap.Size()!=0)
  {
   // Pop the next merge and check if its valid...
    prog->Report(progSize-heap.Size(),progSize);
    Node node = heap.Peek();
    heap.Rem();
    if ((headVer[node.seg1]==node.seg1ver)&&(headVer[node.seg2]==node.seg2ver))
    {
     // Apply the merge...
      headVer[node.seg1] += 1;
      headVer[node.seg2] += 1;
      
      layers.MergeLayers(node.seg1,node.seg2);
      layerScore.OnMergeLayers(node.seg1,node.seg2);      
      
      nat32 mergeHead = layers.SegToLayer(node.seg1);
      headVer[mergeHead] += 1;
      Vertex * toDie = segToVert[node.seg2];
      if (toDie==segToVert[mergeHead]) toDie = segToVert[node.seg1];
      
      for (nat32 i=0;i<segToVert.Size();i++)
      {
       if (segToVert[i]==toDie) segToVert[i] = segToVert[mergeHead];
      }
      segToVert[node.seg1]->Merge(toDie);
      segToVert[node.seg1]->layer = mergeHead;
      
      
     // Add the new merges created to the heap...
      ds::List<ds::Edge*>::Cursor targ = segToVert[mergeHead]->EdgeList();
      while (!targ.Bad())
      {
       Vertex * here = segToVert[mergeHead];
       Vertex * there = static_cast<Vertex*>((*targ)->From());
       if (there==here) there = static_cast<Vertex*>((*targ)->To());       

       Node node;
        node.seg1 = here->layer;
        node.seg2 = there->layer;
        node.cost = layerScore.IfMergeLayers(node.seg1,node.seg2);
        node.seg1ver = headVer[node.seg1];
        node.seg2ver = headVer[node.seg2];
       if (node.cost<bias) {heap.Add(node); ++progSize;}

       ++targ;      
      }
    }
  }
  prog->Pop();


 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
