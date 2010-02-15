#ifndef EOS_DS_LAYERED_GRAPHS_H
#define EOS_DS_LAYERED_GRAPHS_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file layered_graphs.h
/// Provides a graph structure where a shared set of nodes exist between
/// multiple graphs. Various merging and iterating operations are provided.
/// This primarilly exists to suport greedy optimisation by merging a graph
/// structure, where there are multiple things that may be equal between nodes.

#include "eos/types.h"
#include "eos/ds/arrays.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// Code for the layered graph flat template...
// (Its quite nasty.)
// Notes:
// - All data storage malloced.
// - Other than 'all' lists all other lists only contain the 'current' model.
// - The previous model is never deleted however, so you can always work from old 
//   pointers to current state representatives.
// - If iterating a list from which your current item is removed the handle will
//   indicate that its bad if asked.
class EOS_CLASS LayeredGraphCode
{
 protected:
  // This helper structure defines the sizes of the data appened to the 4 storage areas...
   struct Size
   {
    Size(nat32 n,nat32 l,nat32 e,nat32 s)
    :node(n),layer(l),edge(e),super(s)
    {}
    
    nat32 node;
    nat32 layer;
    nat32 edge;
    nat32 super;
   };


  // Pre-declerations...
   struct Node;
   struct Layer;
   struct Edge;
   struct Super;
   
  // Linked list pairs...
   struct NodeLink
   {
    Node * ptr;
    NodeLink * next;
    NodeLink * last;
    
    void MakeEmpty()
    {
     ptr = null<Node*>();
     next = this;
     last = this;
    }
    
    void AddEnd(NodeLink & of)
    {
     next = &of;
     last = of.last;
     next->last = this;
     last->next = this;
    }
    
    void Remove()
    {
     next->last = last;
     last->next = next;
     next = null<NodeLink*>();
     last = null<NodeLink*>();
    }
   };
   
   struct LayerLink
   {
    Layer * ptr;
    LayerLink * next;
    LayerLink * last;
    
    void MakeEmpty()
    {
     ptr = null<Layer*>();
     next = this;
     last = this;
    }
    
    void AddEnd(LayerLink & of)
    {
     next = &of;
     last = of.last;
     next->last = this;
     last->next = this;
    }
    
    void Remove()
    {
     next->last = last;
     last->next = next;
     next = null<LayerLink*>();
     last = null<LayerLink*>();
    }
   };
   
   struct EdgeLink
   {
    Edge * ptr;
    EdgeLink * next;
    EdgeLink * last;
    
    void MakeEmpty()
    {
     ptr = null<Edge*>();
     next = this;
     last = this;
    }
    
    void AddEnd(EdgeLink & of)
    {
     next = &of;
     last = of.last;
     next->last = this;
     last->next = this;
    }
    
    void Remove()
    {
     next->last = last;
     last->next = next;
     next = null<EdgeLink*>();
     last = null<EdgeLink*>();
    }
   };
   
   struct SuperLink
   {
    Super * ptr;
    SuperLink * next;
    SuperLink * last;
    
    void MakeEmpty()
    {
     ptr = null<Super*>();
     next = this;
     last = this;
    }
    
    void AddEnd(SuperLink & of)
    {
     next = &of;
     last = of.last;
     next->last = this;
     last->next = this;
    }
    
    void Remove()
    {
     next->last = last;
     last->next = next;
     next = null<SuperLink*>();
     last = null<SuperLink*>();
    }
   };


  // Structure to store a Node...
   struct Node
   {
    // Standard data set - all linked list that contains everything,
    // current linked list which contains non-superceded nodes, and successor
    // which points to the successor Node when it has been replaced...
     Node * successor;
     NodeLink all;
     NodeLink current; // Will be nulled (next) if not current.

    // For linked list of its Layer nodes - these are kept in layer order...
     LayerLink layer;
     
    // Pointer to the Super set which it is a member of...
     Super * super;
     
    // Linked list of all nodes that are children of a given Super.
     NodeLink siblings;
    
    // Gets pointer to associated data...
     void * Data() {return this+1;}
   };

  // Structure to store a Layer...
   struct Layer
   {
    nat32 index; // Its layer number.

    // Standard data set - all linked list that contains everything,
    // current linked list which contains non-superceded nodes, and successor
    // which points to the successor Node when it has been replaced...
     Layer * successor;
     LayerLink all;
     LayerLink current; // Will be nulled (next) if not current.

    // Linked list of the layer it is in...
     LayerLink layer;

    // Pointer to its ownning node (If it has one.)...
     Node * node;

    // Linked list it forms with its node (If it has one.)...
     LayerLink layers;

    // Linked list of edges...
     EdgeLink edges;
     
    // Version number, used during layer merges to stop edge duplication...
    // (The below variables are only valid at operation time.)
     nat32 version;
     Edge * succ; // If version is valid then this will be the edge that set it - for setting the successor.

    // Gets pointer to associated data...
     void * Data() {return this+1;}
   };
   
  // Structure to store an edge...
   struct Edge
   {
    // Standard data set - all linked list that contains everything,
    // current linked list which contains non-superceded nodes, and successor
    // which points to the successor Node when it has been replaced...
     Edge * successor;
     EdgeLink all;
     EdgeLink current; // Will be nulled (next) if not current.

    // Linked list for the layer its in...
     EdgeLink layer;

    // Pointers to its connecting layers...
     Layer * a;
     Layer * b;

    // Linked lists for its two connecting Layer's...
     EdgeLink linkA;
     EdgeLink linkB;

    // Gets pointer to associated data...
     void * Data() {return this+1;}
   };

  // Structure to store a Super...
   struct Super
   {
    // Standard data set - all linked list that contains everything,
    // current linked list which contains non-superceded nodes, and successor
    // which points to the successor Node when it has been replaced...
     Super * successor;
     SuperLink all;
     SuperLink current; // Will be nulled (next) if not current.
     
    // Linked list of nodes that form part of the super. Nulled if successor.
     NodeLink children;

    // Gets pointer to associated data...
     void * Data() {return this+1;}
   };


  // Storage...
   // Storage of everything, for deletion...
    NodeLink allNodes;
    LayerLink allLayers;
    EdgeLink allEdges;
    SuperLink allSupers;

   // Storage of only the current state, without any depreciated stuff...
    NodeLink currentNodes;
    LayerLink currentLayers;
    EdgeLink currentEdges;
    SuperLink currentSupers;

   // Per-layer storage...
    struct PerLayer
    {
     LayerLink layers;
     EdgeLink edges;
    };
    ds::Array<PerLayer> layer;
    
   // Version number, used for edge merging...
   // (There is an obvious wrap around bug here, but its chances of happenning
   //  are small enough for me to ignore them.)
    nat32 version;


  // Methods (All passed input should be current)...
    LayeredGraphCode();
   ~LayeredGraphCode();
  
   nat32 AddLayer(const Size & size);
   Node * AddNode(const Size & size);
   Edge * AddEdge(const Size & size,Layer * a,Layer * b);
   Edge * AddEdge(const Size & size,nat32 layer,Node * a,Node * b);
   
   Node * MergeNodes(const Size & size,Node * a,Node * b);
   Layer * MergeLayers(const Size & size,Layer * a,Layer * b);
   Layer * MergeLayers(const Size & size,nat32 layer,Node * a,Node * b);
   
   bit Merged(Node * a,Node * b) const;


  // Helper method - this merges edges from two Layers into a single Layer.
  // Done twice in above methods, hence seperated to here...
  // (out must contain an empty edge list.)
   void MergeEdges(Layer * sourceA,Layer * sourceB,Layer * out);
};

//------------------------------------------------------------------------------
/// A graph, with the same set of nodes shared between multiple layers where
/// each layer has its own edges and can have its own node merging operations
/// applied. Has very speciifc design features:
/// - You can store data at nodes, per layer for each node, for each super node 
/// and for each edge.
/// - A super node is defined as a node from the graph constructed if you take
/// the nodes and apply the merge operations from all layers to it.
/// - No handles to data ever go bad. Additionally, except for edge data, all
/// other data gets new storage each time it changes due to a merge, so you can
/// re-calculate the new data post-merge from the original data that still exists.
/// Given a handle to something you always find its newest version. (Except for
/// edges, which can reach a dead end if there two nodes are merged. You can 
/// find that out also.)
/// - Nothing is deleted; this gets bigger with merges, not smaller.
/// - You will usually want to typedef a particular version of this template, to make handles reasonable to declare.
/// - All operations are efficient in implimentation, admitedly at the expense of memory consumption.
///
/// Has 4 templated parameters. Note that the types are not initialised or
/// deinitialised - they must be safe as arbitary blocks of memory.
/// - NT - Object stored with a node.
/// - LT - Object stored with a node for each layer. When merging layers another one appears.
/// - ET - Object stored with an edge. When things are merged such that edges merge one edge is kept, with its data, whilst another is depreciated, such that you can find its succesor.
/// - ST - Objects stored at a super node.
template <typename NT = Nothing,typename LT = Nothing,typename ET = Nothing,typename ST = Nothing>
class EOS_CLASS LayeredGraph : public LayeredGraphCode
{
 public:
 // Predeclerations...
  class NodeHand;
  class LayerHand;
  class EdgeHand;
  class SuperHand;

  class NodeIter;
  class LayerIter;
  class EdgeIter;
  class SuperIter;



 // Handle types...
  /// Handle to a node.
   class EOS_CLASS NodeHand
   {
    public:
     /// Will be Bad after this.
      NodeHand():ptr(null<Node*>()) {}

     /// &nbsp;
      NodeHand(const NodeHand & rhs):ptr(rhs.ptr) {}

     // Not documented as the user should not be able to use this.
      NodeHand(Node * p):ptr(p) {}

     /// &nbsp;
      ~NodeHand() {}

     /// &nbsp;
      NodeHand & operator = (const NodeHand & rhs) {ptr = rhs.ptr; return *this;}

     /// Returns true if its bad, i.e. it has not had a real handle assigned to it.
      bit Bad() const {return ptr==null<Node*>();}


     /// &nbsp;
      NT & operator* () {return *(NT*)ptr->Data();}

     /// &nbsp;
      const NT & operator* () const {return *(NT*)ptr->Data();}

     /// &nbsp;
      NT * operator->() {return (NT*)ptr->Data();}

     /// &nbsp;
      const NT * operator->() const {return (NT*)ptr->Data();}


     /// Returns true if the handle is to a current node.
      bit Current() const {return ptr->successor==null<Node*>();}

     /// Returns a handle to the current NodeHand derived from this one.
     /// (If it is the current it clones the current handle.)
      NodeHand GetCurrent() const
      {
       Node * targ = ptr;
       while (targ->successor) targ = targ->successor;
       return NodeHand(targ);
      }

     /// Converts this handle into a handle to the current one derived from this one.
     /// If current does nothing.
      void ToCurrent() const
      {
       while (ptr->successor) ptr = ptr->successor;
      }


     /// Returns the Super node which contains this node.
      SuperHand GetSuper() const
      {
       Super * s = ptr->super;
       while (s->successor) s = s->successor;
       return SuperHand(s);
      }

     /// Returns an iterator with which you can, in order, iterate the LayerHand-s.
      LayerIter LayerFront() const {return LayerIter(ptr->layer.next);}

     /// Returns an iterator with which you can, in order, iterate the LayerHand-s.
      LayerIter LayerBack() const {return LayerIter(ptr->layer.last);}


    private:
     friend class LayeredGraph;
     Node * ptr;
   };


  /// Handle to a per-layer node.
   class EOS_CLASS LayerHand
   {
    public:
     /// Will be Bad after this.
      LayerHand():ptr(null<Layer*>()) {}

     /// &nbsp;
      LayerHand(const LayerHand & rhs):ptr(rhs.ptr) {}

     // Not documented as the user should not be able to use this.
      LayerHand(Layer * p):ptr(p) {}

     /// &nbsp;
      ~LayerHand() {}

     /// &nbsp;
      LayerHand & operator = (const LayerHand & rhs) {ptr = rhs.ptr; return *this;}

     /// Returns true if its bad, i.e. it has not had a real handle assigned to it.
      bit Bad() const {return ptr==null<Layer*>();}


     /// &nbsp;
      LT & operator* () {return *(LT*)ptr->Data();}

     /// &nbsp;
      const LT & operator* () const {return *(LT*)ptr->Data();}

     /// &nbsp;
      LT * operator->() {return (LT*)ptr->Data();}

     /// &nbsp;
      const LT * operator->() const {return (LT*)ptr->Data();}


     /// Returns true if the handle is to a current node.
      bit Current() const {return ptr->successor==null<Layer*>();}

     /// Returns a handle to the current LayerHand derived from this one.
     /// (If it is the current it clones the current handle.)
      LayerHand GetCurrent() const
      {
       Layer * targ = ptr;
       while (targ->successor) targ = targ->successor;
       return LayerHand(targ);
      }

     /// Converts this handle into a handle to the current one derived from this one.
     /// If current does nothing.
      void ToCurrent() const
      {
       while (ptr->successor) ptr = ptr->successor;
      }
      
      
     /// Returns its layer index.
      nat32 Index() const {return ptr->index;}
      
     /// Returns the owning node, if it has one.
     /// Will return a bad if it has no owning node.
     /// Thie method is a touch dodgy in its specification/meaning - probably best avoided.
      NodeHand GetNode() const {return NodeHand(ptr->node);}
      
     /// Returns an iterator so you can iterate all its edges.
      EdgeIter EdgeFront() const {return EdgeIter(ptr->edges.next);}

     /// Returns an iterator so you can iterate all its edges.
      EdgeIter EdgeBack() const {return EdgeIter(ptr->edges.last);}


    private:
     friend class LayeredGraph;
     Layer * ptr;
   };


  /// Handle to an edge.
   class EOS_CLASS EdgeHand
   {
    public:
     /// Will be Bad after this.
      EdgeHand():ptr(null<Edge*>()) {}

     /// &nbsp;
      EdgeHand(const EdgeHand & rhs):ptr(rhs.ptr) {}

     // Not documented as the user should not be able to use this.
      EdgeHand(Edge * p):ptr(p) {}

     /// &nbsp;
      ~EdgeHand() {}

     /// &nbsp;
      EdgeHand & operator = (const EdgeHand & rhs) {ptr = rhs.ptr; return *this;}

     /// Returns true if its bad, i.e. it has not had a real handle assigned to it.
      bit Bad() const {return ptr==null<Edge*>();}


     /// &nbsp;
      ET & operator* () {return *(ET*)ptr->Data();}

     /// &nbsp;
      const ET & operator* () const {return *(ET*)ptr->Data();}

     /// &nbsp;
      ET * operator->() {return (ET*)ptr->Data();}

     /// &nbsp;
      const ET * operator->() const {return (ET*)ptr->Data();}


     /// Returns true if the handle is to a current node.
      bit Current() const {return ptr->successor==null<Edge*>();}

     /// Returns a handle to the current EdgeHand derived from this one.
     /// (If it is the current it clones the current handle.)
     /// Note that there can be no current, in such case the returnee will be Bad().
      EdgeHand GetCurrent() const
      {
       Edge * targ = ptr;
       while (targ->successor) targ = targ->successor;
       if (targ->current.next==null<EdgeLink*>()) targ = null<Edge*>();
       return EdgeHand(targ);
      }

     /// Converts this handle into a handle to the current one derived from this one.
     /// If current does nothing.
     /// Note that there can be no current, in such case this will make this Bad().     
      void ToCurrent() const
      {
       while (ptr->successor) ptr = ptr->successor;
       if (ptr->current.next==null<EdgeLink*>()) ptr = null<Edge*>();
      }


     /// Returns one of the LayerHands this edge is connected too.
      LayerHand A() const {return LayerHand(ptr->a);}

     /// Returns the other LayerHand this edge is connected too.
      LayerHand B() const {return LayerHand(ptr->b);}


    private:
     friend class LayeredGraph;
     Edge * ptr;
   };


  /// Handle to a super node.
   class EOS_CLASS SuperHand
   {
    public:
     /// Will be Bad after this.
      SuperHand():ptr(null<Super*>()) {}

     /// &nbsp;
      SuperHand(const SuperHand & rhs):ptr(rhs.ptr) {}

     // Not documented as the user should not be able to use this.
      SuperHand(Super * p):ptr(p) {}

     /// &nbsp;
      ~SuperHand() {}

     /// &nbsp;
      SuperHand & operator = (const SuperHand & rhs) {ptr = rhs.ptr; return *this;}

     /// Returns true if its bad, i.e. it has not had a real handle assigned to it.
      bit Bad() const {return ptr==null<Super*>();}


     /// &nbsp;
      ST & operator* () {return *(ST*)ptr->Data();}

     /// &nbsp;
      const ST & operator* () const {return *(ST*)ptr->Data();}

     /// &nbsp;
      ST * operator->() {return (ST*)ptr->Data();}

     /// &nbsp;
      const ST * operator->() const {return (ST*)ptr->Data();}


     /// Returns true if the handle is to a current node.
      bit Current() const {return ptr->successor==null<Super*>();}

     /// Returns a handle to the current SuperHand derived from this one.
     /// (If it is the current it clones the current handle.)
      SuperHand GetCurrent() const
      {
       Super * targ = ptr;
       while (targ->successor) targ = targ->successor;
       return SuperHand(targ);
      }

     /// Converts this handle into a handle to the current one derived from this one.
     /// If current does nothing.
      void ToCurrent() const
      {
       while (ptr->successor) ptr = ptr->successor;
      }


     /// Returns an iterator of the NodeHand objects that are merged to make 
     /// this SuperHand.
      NodeIter NodeFront() const {return NodeIter(ptr->children.next);}

     /// Returns an iterator of the NodeHand objects that are merged to make 
     /// this SuperHand.
      NodeIter NodeBack() const {return NodeIter(ptr->children.last);}


    private:
     friend class LayeredGraph;
     Super * ptr;
   };



 // Iterator types...
  /// Iterator of node handles.
   class EOS_CLASS NodeIter
   {
    public:
     /// Will be Bad after this.
      NodeIter():ptr(null<NodeLink*>()) {}

     /// &nbsp;
      NodeIter(const NodeIter & rhs):ptr(rhs.ptr) {}

     // Not documented as the user should not be able to use this.
      NodeIter(NodeLink * p):ptr(p) {}

     /// &nbsp;
      ~NodeIter() {}

     /// &nbsp;
      NodeIter & operator = (const NodeIter & rhs) {ptr = rhs.ptr; return *this;}

     /// Returns true if its bad, i.e. it is not capable of iterating and you shouldn't ask for the handle.
      bit Bad() const {return (ptr==null<NodeLink*>())||(ptr->ptr==null<Node*>());}
      
      
     /// Returns the handle it currently points at, will be Bad() if it is Bad().
     /// (Will always be current.)
      NodeHand Targ() const
      {
       if (ptr)
       {
        Node * targ = ptr->ptr;
        while (targ->successor) targ = targ->successor;
        return NodeHand(targ);
       }
       else return NodeHand(null<Node*>());
      }
      
     /// To next.
      NodeIter & operator++() {ptr = ptr->next; return *this;}

     /// To previous.
      NodeIter & operator--() {ptr = ptr->last; return *this;}


    private:
     NodeLink * ptr;
   };


  /// Iterator of layer handles.
   class EOS_CLASS LayerIter
   {
    public:
     /// Will be Bad after this.
      LayerIter():ptr(null<LayerLink*>()) {}

     /// &nbsp;
      LayerIter(const LayerIter & rhs):ptr(rhs.ptr) {}

     // Not documented as the user should not be able to use this.
      LayerIter(LayerLink * p):ptr(p) {}

     /// &nbsp;
      ~LayerIter() {}

     /// &nbsp;
      LayerIter & operator = (const LayerIter & rhs) {ptr = rhs.ptr; return *this;}

     /// Returns true if its bad, i.e. it is not capable of iterating and you shouldn't ask for the handle.
      bit Bad() const {return (ptr==null<LayerLink*>())||(ptr->ptr==null<Layer*>());}


     /// Returns the handle it currently points at, will be Bad() if it is Bad().
     /// (Will always be current.)
      LayerHand Targ() const
      {
       if (ptr)
       {
        Layer * targ = ptr->ptr;
        while (targ->successor) targ = targ->successor;
        return LayerHand(targ);
       }
       else return LayerHand(null<Layer*>());
      }
      
     /// To next.
      LayerIter & operator++() {ptr = ptr->next; return *this;}

     /// To previous.
      LayerIter & operator--() {ptr = ptr->last; return *this;}


    private:
     LayerLink * ptr;
   };


  /// Iterator of edge handles.
   class EOS_CLASS EdgeIter
   {
    public:
     /// Will be Bad after this.
      EdgeIter():ptr(null<EdgeLink*>()) {}
      
     /// &nbsp;
      EdgeIter(const EdgeIter & rhs):ptr(rhs.ptr) {}      

     // Not documented as the user should not be able to use this.
      EdgeIter(EdgeLink * p):ptr(p) {}

     /// &nbsp;
      ~EdgeIter() {}

     /// &nbsp;
      EdgeIter & operator = (const EdgeIter & rhs) {ptr = rhs.ptr; return *this;}

     /// Returns true if its bad, i.e. it is not capable of iterating and you shouldn't ask for the handle.
      bit Bad() const {return (ptr==null<EdgeLink*>())||(ptr->ptr==null<Edge*>());}


     /// Returns the handle it currently points at, will be Bad() if it is Bad().
     /// (Will always be current, can return  Bad() hand if it has no current.)
      EdgeHand Targ() const
      {
       if (ptr)
       {
        Edge * targ = ptr->ptr;
        while (targ->successor) targ = targ->successor;
        if (targ->current.next==null<EdgeLink*>()) return EdgeHand(null<Edge*>());
        return EdgeHand(targ);
       }
       else return EdgeHand(null<Edge*>());
      }
      
     /// To next.
      EdgeIter & operator++() {ptr = ptr->next; return *this;}

     /// To previous.
      EdgeIter & operator--() {ptr = ptr->last; return *this;}


    private:
     EdgeLink * ptr;
   };


  /// Iterator of super handles.
   class EOS_CLASS SuperIter
   {
    public:
     /// Will be Bad after this.
      SuperIter():ptr(null<SuperLink*>()) {}
      
     /// &nbsp;
      SuperIter(const SuperIter & rhs):ptr(rhs.ptr) {}

     // Not documented as the user should not be able to use this.
      SuperIter(SuperLink * p):ptr(p) {}

     /// &nbsp;
      ~SuperIter() {}

     /// &nbsp;
      SuperIter & operator = (const SuperIter & rhs) {ptr = rhs.ptr; return *this;}

     /// Returns true if its bad, i.e. it is not capable of iterating and you shouldn't ask for the handle.
      bit Bad() const {return (ptr==null<SuperLink*>())||(ptr->ptr==null<Super*>());}


     /// Returns the handle it currently points at, will be Bad() if it is Bad().
     /// (Will always be current.)
      SuperHand Targ() const
      {
       if (ptr)
       {
        Super * targ = ptr->ptr;
        while (targ->successor) targ = targ->successor;
        return SuperHand(targ);
       }
       else return SuperHand(null<Super*>());
      }
      
     /// To next.
      SuperIter & operator++() {ptr = ptr->next; return *this;}

     /// To previous.
      SuperIter & operator--() {ptr = ptr->last; return *this;}


    private:
     SuperLink * ptr;
   };


   
 // Methods...   
  /// &nbsp;
   LayeredGraph() {}
   
  /// &nbsp;
   ~LayeredGraph() {}
   
   
  /// Returns how many layers exist.
   nat32 Layers() const {return layer.Size();}
   
  /// Adds a new layer, returning its index number.
   nat32 AddLayer() {return LayeredGraphCode::AddLayer(Size(sizeof(NT),sizeof(LT),sizeof(ET),sizeof(ST)));}
   
   
  /// Adds a node, returning its handle.
   NodeHand AddNode() {return NodeHand(LayeredGraphCode::AddNode(Size(sizeof(NT),sizeof(LT),sizeof(ET),sizeof(ST))));}

  /// Adds an edge between two layer nodes. The layer nodes must be of the same index.
  /// Returns its handle, if given two identical/merged LayerHadn nodes does 
  /// nothing and returns a bad EdgeHand.
   EdgeHand AddEdge(LayerHand a,LayerHand b)
   {
    return EdgeHand(LayeredGraphCode::AddEdge(Size(sizeof(NT),sizeof(LT),sizeof(ET),sizeof(ST)),a.ptr,b.ptr));
   }

  /// Adds an edge between two nodes on a given layer.
  /// Has to iterate to get to the relevant layer, so can be quite slow for higher layer indices.
  /// Does nothing if the two nodes have been merged on the given layer, and returns a bad handle.
   EdgeHand AddEdge(nat32 l,NodeHand a,NodeHand b)
   {
    return EdgeHand(LayeredGraphCode::AddEdge(Size(sizeof(NT),sizeof(LT),sizeof(ET),sizeof(ST)),l,a.ptr,b.ptr));
   }


  /// Returns true if the two given nodes have been merged on all layers.
   bit Merged(NodeHand a,NodeHand b) const {return LayeredGraphCode::Merged(a.ptr,b.ptr);}
   
  /// Merges two nodes, so that one node now represents them and they are merged
  /// on all layers.
   NodeHand MergeNodes(NodeHand a,NodeHand b)
   {
    return NodeHand(LayeredGraphCode::MergeNodes(Size(sizeof(NT),sizeof(LT),sizeof(ET),sizeof(ST)),a.ptr,b.ptr));
   }
   
  /// Merges two layers, returning the handle of the replacement layer.
  /// If they are allready merged does nothing and returns the merged layer.
   LayerHand MergeLayers(LayerHand a,LayerHand b)
   {
    return LayerHand(LayeredGraphCode::MergeLayers(Size(sizeof(NT),sizeof(LT),sizeof(ET),sizeof(ST)),a.ptr,b.ptr));
   }

  /// Merges two layers, returning the handle of the replacement layer.
  /// If they are allready merged does nothing and returns the merged layer.
   LayerHand MergeLayers(nat32 l,NodeHand a,NodeHand b)
   {
    return LayerHand(LayeredGraphCode::MergeLayers(Size(sizeof(NT),sizeof(LT),sizeof(ET),sizeof(ST)),l,a.ptr,b.ptr));
   }


  /// Returns an iterator for all current nodes.
   NodeIter NodeFront() const {return NodeIter(currentNodes.next);}

  /// Returns an iterator for all current nodes.
   NodeIter NodeBack() const {return NodeIter(currentNodes.last);}
  
  /// Returns an iterator for all current layer nodes.
   LayerIter LayerFront() const {return LayerIter(currentLayers.next);}

  /// Returns an iterator for all current layer nodes.
   LayerIter LayerBack() const {return LayerIter(currentLayers.last);}
   
  /// Returns an iterator for all current edges.
   EdgeIter EdgeFront() const {return EdgeIter(currentEdges.next);}

  /// Returns an iterator for all current edges.
   EdgeIter EdgeBack() const {return EdgeIter(currentEdges.last);}
   
  /// Returns an iterator for all current super nodes.
   SuperIter SuperFront() const {return SuperIter(currentSupers.next);}

  /// Returns an iterator for all current super nodes.
   SuperIter SuperBack() const {return SuperIter(currentSupers.last);}


  /// Returns an iterator for all layer nodes on a given layer.
   LayerIter LayerFront(nat32 l) const {return LayerIter(layer[l].layers.next);}

  /// Returns an iterator for all layer nodes on a given layer.
   LayerIter LayerBack(nat32 l) const {return LayerIter(layer[l].layers.last);}
   
  /// Returns an iterator for all edges on a given layer.
   EdgeIter EdgeFront(nat32 l) const {return EdgeIter(layer[l].edges.next);}

  /// Returns an iterator for all edges on a given layer.
   EdgeIter EdgeBack(nat32 l) const {return EdgeIter(layer[l].edges.last);}
};

//------------------------------------------------------------------------------
 };
};
#endif
