#ifndef EOS_MYA_LAYER_MERGE_H
#define EOS_MYA_LAYER_MERGE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file layer_merge.h
/// Provides an algorithm to be run on a Layers object, merges layers when a 
/// given scoring object produces better results for the two layers merged over
/// the two layers seperate.

#include "eos/types.h"
#include "eos/ds/graphs.h"
#include "eos/mya/surfaces.h"
#include "eos/mya/ied.h"
#include "eos/mya/layers.h"
#include "eos/mya/layer_score.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
/// A layers object operation functor. Given a layers object and a cost object
/// on construction every time this object is run it merges any layers in best
/// improvment first order until there are no possible merges with a better 
/// improvment than a given bias value.
class EOS_CLASS LayerMerge
{
 public:
  /// &nbsp;
   LayerMerge(const svt::Field<nat32> & segs,Layers & layers,LayerScore & layerScore);
   
  /// &nbsp;
   ~LayerMerge();


  /// Sets the bias, when comparing costs it does all merge operations where the improvment
  /// is less than the given bias. Defaults to 0.0. Setting it to greater than 0.0
  /// is just asking for trouble.
   void SetBias(real32 bias);
  
  /// Applys a merge operation on the layer object from construction.
   void operator () (time::Progress * prog = null<time::Progress*>()) const;


  // &nbsp;
   static inline cstrconst TypeString() {return "eos::mya::LayerMerge";}

 private:
  svt::Field<nat32> segs;
  Layers & layers;
  LayerScore & layerScore;
  
  real32 bias;
  
  // This class represents a potential merge option and its cost, these are
  // stored in a heap so the algorithm can be efficiently greedy.
   struct Node
   {
    bit operator < (const Node & rhs) const {return cost<rhs.cost;}
    
    nat32 seg1;
    nat32 seg2;
    real32 cost;
    nat32 seg1ver; // Version numbers, when a segment head is involved in a merge its version is incrimented, to indicate the invalidity of yet to be proccessed merges without having to actually remove such merges from the heap.
    nat32 seg2ver; // "
   };
   
  // A derivative of a graph vertex used by the algorithm...
   class Vertex : public ds::Vertex
   {
    public:
     Vertex(ds::Graph & graph):ds::Vertex(graph) {}
    
     nat32 layer; // The head segment for the layer in representation.
   };
};

//------------------------------------------------------------------------------
 };
};
#endif
