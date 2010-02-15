//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/ds/layered_graphs.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
LayeredGraphCode::LayeredGraphCode()
{
 // Setup the 'all' lists...
  allNodes.MakeEmpty();
  allLayers.MakeEmpty();
  allEdges.MakeEmpty();
  allSupers.MakeEmpty();

 // Setup the 'current' lists...
  currentNodes.MakeEmpty();
  currentLayers.MakeEmpty();
  currentEdges.MakeEmpty();
  currentSupers.MakeEmpty();
  
 // Version number - any value will do...
  version = 0;
}

LayeredGraphCode::~LayeredGraphCode()
{
 // Iterate each 'all' list, terminate...
  NodeLink * targNode = allNodes.next;
  while (targNode->ptr)
  {
   Node * victim = targNode->ptr;
   targNode = targNode->next;
   mem::Free(victim);
  }
  
  LayerLink * targLayer = allLayers.next;
  while (targLayer->ptr)
  {
   Layer * victim = targLayer->ptr;
   targLayer = targLayer->next;
   mem::Free(victim);
  }
  
  EdgeLink * targEdge = allEdges.next;
  while (targEdge->ptr)
  {
   Edge * victim = targEdge->ptr;
   targEdge = targEdge->next;
   mem::Free(victim);
  }
  
  SuperLink * targSuper = allSupers.next;
  while (targSuper->ptr)
  {
   Super * victim = targSuper->ptr;
   targSuper = targSuper->next;
   mem::Free(victim);
  }    
}

nat32 LayeredGraphCode::AddLayer(const Size & size)
{
 nat32 ret = layer.Size();
 
 // Iterate the layers as they currently stand and null pointers for those that
 // point to themselves...
  for (nat32 i=0;i<layer.Size();i++)
  {
   if (layer[i].layers.next==&layer[i].layers) layer[i].layers.next = null<LayerLink*>();
   if (layer[i].edges.next==&layer[i].edges) layer[i].edges.next = null<EdgeLink*>();
  }

 // Add layer to layer list array...
  layer.Size(layer.Size()+1);
  layer[ret].layers.MakeEmpty();
  layer[ret].edges.MakeEmpty();


 // Pass through previous layers and correct pointers - array re-allocation 
 // broke them...
  for (nat32 i=0;i<ret;i++)
  {
   if (layer[i].layers.next==null<LayerLink*>()) layer[i].layers.MakeEmpty();
   else
   {
    layer[i].layers.next->last = &layer[i].layers;
    layer[i].layers.last->next = &layer[i].layers;
   }
   
   if (layer[i].edges.next==null<EdgeLink*>()) layer[i].edges.MakeEmpty();
   else
   {
    layer[i].edges.next->last = &layer[i].edges;
    layer[i].edges.last->next = &layer[i].edges;
   }   
  }


 // Iterate all nodes and add the extra layer to each...
  NodeLink * targNode = currentNodes.next;
  while (targNode->ptr)
  {
   Layer * nl = (Layer*)(void*)mem::Malloc<byte>(sizeof(Layer) + size.layer);
   nl->index = ret;
   nl->successor = null<Layer*>();
   nl->all.ptr = nl;
   nl->all.AddEnd(allLayers);
   nl->current.ptr = nl;
   nl->current.AddEnd(currentLayers);
   nl->layer.ptr = nl;
   nl->layer.AddEnd(layer[ret].layers);
   nl->node = targNode->ptr;
   nl->layers.ptr = nl;
   nl->layers.AddEnd(targNode->ptr->layer);
   nl->edges.MakeEmpty();
   nl->version = version;

   targNode = targNode->next;
  }
 
 return ret;
}

LayeredGraphCode::Node * LayeredGraphCode::AddNode(const Size & size)
{
 // Create the node itself...
  Node * nn = (Node*)(void*)mem::Malloc<byte>(sizeof(Node) + size.node);
  nn->successor = null<Node*>();
  nn->all.ptr = nn;
  nn->all.AddEnd(allNodes);
  nn->current.ptr = nn;
  nn->current.AddEnd(currentNodes);
  nn->layer.MakeEmpty();


 // Create the nodes Super...
  Super * ns = (Super*)(void*)mem::Malloc<byte>(sizeof(Super) + size.super);
  ns->successor = null<Super*>();
  ns->all.ptr = ns;
  ns->all.AddEnd(allSupers);
  ns->current.ptr = ns;
  ns->current.AddEnd(currentSupers);
  ns->children.MakeEmpty();

  nn->super = ns;
  nn->siblings.ptr = nn;
  nn->siblings.AddEnd(ns->children);


 // Create the Layer nodes for each layer... 
  for (nat32 i=0;i<layer.Size();i++)
  {
   Layer * nl = (Layer*)(void*)mem::Malloc<byte>(sizeof(Layer) + size.layer);
   nl->index = i;
   nl->successor = null<Layer*>();
   nl->all.ptr = nl;
   nl->all.AddEnd(allLayers);
   nl->current.ptr = nl;
   nl->current.AddEnd(currentLayers);
   nl->layer.ptr = nl;
   nl->layer.AddEnd(layer[i].layers);
   nl->node = nn;
   nl->layers.ptr = nl;
   nl->layers.AddEnd(nn->layer);
   nl->edges.MakeEmpty();
   nl->version = version;
  }


 return nn;
}

LayeredGraphCode::Edge * LayeredGraphCode::AddEdge(const Size & size,Layer * a,Layer * b)
{
 log::Assert(a->index==b->index);
 
 while (a->successor) a = a->successor;
 while (b->successor) b = b->successor;
 if (a==b) return null<Edge*>();

 Edge * ne = (Edge*)(void*)mem::Malloc<byte>(sizeof(Edge) + size.edge);
 ne->successor = null<Edge*>();
 ne->all.ptr = ne;
 ne->all.AddEnd(allEdges);
 ne->current.ptr = ne;
 ne->current.AddEnd(currentEdges);
 ne->layer.ptr = ne;
 ne->layer.AddEnd(layer[a->index].edges);
 ne->a = a;
 ne->b = b;
 ne->linkA.ptr = ne;
 ne->linkA.AddEnd(a->edges);
 ne->linkB.ptr = ne;
 ne->linkB.AddEnd(b->edges);
 
 return ne;
}

LayeredGraphCode::Edge * LayeredGraphCode::AddEdge(const Size & size,nat32 l,Node * a,Node * b)
{
 LayerLink * la = a->layer.next;
 LayerLink * lb = b->layer.next;
 for (nat32 i=0;i<l;i++)
 {
  la = la->next;
  lb = lb->next;
 }
 
 log::Assert((la->ptr->index==l)&&(lb->ptr->index==l));
 return AddEdge(size,la->ptr,lb->ptr);
}

LayeredGraphCode::Node * LayeredGraphCode::MergeNodes(const Size & size,Node * a,Node * b)
{
 // Create the merged node, fiddle with its predecessors...
  Node * nn = (Node*)(void*)mem::Malloc<byte>(sizeof(Node) + size.node);
  nn->successor = null<Node*>();
  nn->all.ptr = nn;
  nn->all.AddEnd(allNodes);
  nn->current.ptr = nn;
  nn->current.AddEnd(currentNodes);
  nn->layer.MakeEmpty();
  nn->siblings.ptr = nn;
  
  a->successor = nn;
  b->successor = nn;
  a->current.Remove();
  b->current.Remove();
  a->siblings.Remove();
  b->siblings.Remove();


 // Check if a new super needs to be created, and create if so...
 // Fiddle the other supers as needed.
  if (a->super==b->super)
  {
   nn->super = a->super;
   nn->siblings.AddEnd(nn->super->children);
  }
  else
  {
   Super * ns = (Super*)(void*)mem::Malloc<byte>(sizeof(Super) + size.super);
   ns->successor = null<Super*>();
   ns->all.ptr = ns;
   ns->all.AddEnd(allSupers);
   ns->current.ptr = ns;
   ns->current.AddEnd(currentSupers);

   ns->children.ptr = null<Node*>();
   a->super->children.next->last = b->super->children.last;
   b->super->children.last->next = a->super->children.next;
   a->super->children.next = &ns->children;
   b->super->children.last = &ns->children;
   ns->children.next = &b->super->children;
   ns->children.last = &a->super->children;
   
   a->super->children.Remove();
   b->super->children.Remove();

   a->super->successor = ns;
   b->super->successor = ns;
   a->super->current.Remove();
   b->super->current.Remove();

   nn->super = ns;
   nn->siblings.AddEnd(ns->children);
  }

 
 // Create new layers, except when the layers are allready merged in which case recycle...
 // When creating new layers the edges will need to be regiged.
 {
  LayerLink * lap = a->layer.next;
  LayerLink * lbp = b->layer.next;
  for (nat32 i=0;i<layer.Size();i++)
  {
   // Get to the heads...
    Layer * headA = lap->ptr;
    Layer * headB = lbp->ptr;
    while (headA->successor) headA = headA->successor;
    while (headB->successor) headB = headB->successor;

   // If the heads are the same they are allready merged - simply transfer over, 
   // otherwise we need to create a new layer and depreciate the previous...
    if (headA==headB)
    {
     // Reuse the current head...
      headA->node = nn;
      headA->layers.Remove();
      headA->layers.AddEnd(nn->layer);      
    }
    else
    {
     // Make a new head...
      Layer * nl = (Layer*)(void*)mem::Malloc<byte>(sizeof(Layer) + size.layer);
      nl->index = headA->index;
      nl->successor = null<Layer*>();
      nl->all.ptr = nl;
      nl->all.AddEnd(allLayers);
      nl->current.ptr = nl;
      nl->current.AddEnd(currentLayers);
      nl->layer.ptr = nl;
      nl->layer.AddEnd(layer[nl->index].layers);
      nl->node = nn;
      nl->layers.ptr = nl;
      nl->layers.AddEnd(nn->layer);
      nl->edges.MakeEmpty();
      nl->version = version;

     // Screw with the previous heads...
      headA->successor = nl;
      headB->successor = nl;
      headA->current.Remove();
      headB->current.Remove();
      headA->layer.Remove();
      headB->layer.Remove();
      
     // Copy across and merge the two edge sets (Hard part is making sure to avoid duplicates)...
      MergeEdges(headA,headB,nl);
    }
   
   lap = lap->next;
   lbp = lbp->next;
  }
 }

 return nn;
}

LayeredGraphCode::Layer * LayeredGraphCode::MergeLayers(const Size & size,Layer * a,Layer * b)
{
 // Move to current, exit and return null if the same...
  while (a->successor) a = a->successor;
  while (b->successor) b = b->successor;
  if (a==b) return a;

 // Create new layer...
  Layer * nl = (Layer*)(void*)mem::Malloc<byte>(sizeof(Layer) + size.layer);
  nl->index = a->index;
  nl->successor = null<Layer*>();
  nl->all.ptr = nl;
  nl->all.AddEnd(allLayers);  
  nl->current.ptr = nl;
  nl->current.AddEnd(currentLayers);
  nl->layer.ptr = nl;
  nl->layer.AddEnd(layer[nl->index].layers);
  nl->node = null<Node*>();
  nl->layers.ptr = null<Layer*>();
  nl->layers.next = null<LayerLink*>();
  nl->layers.last = null<LayerLink*>();
  nl->edges.MakeEmpty();
  nl->version = version;
 
 // Fiddle with predecesors as needed...
  a->successor = nl;
  b->successor = nl;
  a->current.Remove();
  b->current.Remove();
  a->layer.Remove();
  b->layer.Remove();
 
 // Copy over and merge edges...
  MergeEdges(a,b,nl);

 return nl;
}

LayeredGraphCode::Layer * LayeredGraphCode::MergeLayers(const Size & size,nat32 l,Node * a,Node * b)
{
 LayerLink * la = a->layer.next;
 LayerLink * lb = b->layer.next;
 for (nat32 i=0;i<l;i++)
 {
  la = la->next;
  lb = lb->next;
 }
 
 log::Assert((la->ptr->index==l)&&(lb->ptr->index==l));
 return MergeLayers(size,la->ptr,lb->ptr);
}

bit LayeredGraphCode::Merged(Node * a,Node * b) const
{
 // Iterate all Layers - return false the moment a head is unshared...
  LayerLink * targA = a->layer.next;
  LayerLink * targB = b->layer.next;
  for (nat32 i=0;i<layer.Size();i++)
  {
   Layer * ta = targA->ptr;
   Layer * tb = targB->ptr;

   while (ta->successor) ta = ta->successor;
   while (tb->successor) tb = tb->successor;
   
   if (ta!=tb) return false;
   
   targA = targA->next;
   targB = targB->next;
  }
 
 return true;
}

void LayeredGraphCode::MergeEdges(Layer * sourceA,Layer * sourceB,Layer * out)
{
 // We use a number versioning system assigned to each layer - the number is 
 // incrimented for each use of MergeEdges so its unique each time. When an edge
 // to a given layer is made the other layer has its number updated - this means
 // we don't add it twice. At the start of the process we set the out source
 // version numbers, so edges to self are avoided as part of this logic.
  // Setup...
   version += 1;
   sourceA->version = version;
   sourceB->version = version;
   sourceA->succ = null<Edge*>();
   sourceB->succ = null<Edge*>();

  // sourceA...
   EdgeLink * targ = sourceA->edges.next;
   log::Assert(targ,"Bad Targ Ptr");
   while (targ->ptr)
   {
    Edge * te = targ->ptr;
    targ = targ->next;
    log::Assert(targ,"Bad Targ Ptr");

    // Identify other Layer...
     Layer * other = (te->a==sourceA)?te->b:te->a;

    // If other Layers version is different we can transfer over the edge...
    // (We do this be reusing the memory, if it isn't different then we
    //  have to depreciate the edge.)
     if (other->version!=version)
     {
      other->version = version;

      if (te->a==sourceA)
      {
       // Edit the a side...
        te->a = out;
        te->linkA.Remove();
        te->linkA.AddEnd(out->edges);
      }
      else
      {
       // Edit the b side...
        te->b = out;
        te->linkB.Remove();
        te->linkB.AddEnd(out->edges);
      }
     }
     else
     {
      // Depreciate the edge, if its not an edge to self then we need to find 
      // its successor...
       te->successor = other->succ;

       te->current.Remove();
       te->layer.Remove();
       te->linkA.Remove();
       te->linkB.Remove();
     }
   }
  
  // sourceB...
   targ = sourceB->edges.next;
   log::Assert(targ,"Bad Targ Ptr");
   while (targ->ptr)
   {
    Edge * te = targ->ptr;
    targ = targ->next;
    log::Assert(targ,"Bad Targ Ptr");

    // Identify other Layer...
     Layer * other = (te->a==sourceB)?te->b:te->a;

    // If other Layers version is different we can transfer over the edge...
    // (We do this be reusing the memory, if it isn't different then we
    //  have to depreciate the edge.)
     if (other->version!=version)
     {
      other->version = version;

      if (te->a==sourceB)
      {
       // Edit the a side...
        te->a = out;
        te->linkA.Remove();
        te->linkA.AddEnd(out->edges);
      }
      else
      {
       // Edit the b side...
        te->b = out;
        te->linkB.Remove();
        te->linkB.AddEnd(out->edges);
      }
     }
     else
     {
      // Depreciate the edge, if its not an edge to self then we need to find 
      // its successor...
       te->successor = other->succ;

       te->current.Remove();
       te->layer.Remove();
       te->linkA.Remove();
       te->linkB.Remove();
     }
   }
}

//------------------------------------------------------------------------------
 };
};
