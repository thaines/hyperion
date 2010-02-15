#ifndef EOS_MYA_LAYER_GROW_H
#define EOS_MYA_LAYER_GROW_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file layer_grow.h
/// Provides an algorithm to be run on a Layers object, re-assigns segments to 
/// neighbouring layers when this improves a given scoring objects result.

#include "eos/types.h"
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
/// on construction every time this object is run it attempts to reduce the cost
/// of the assignment by growing/shrinking layers by changing the assignment of
/// segments to layers at the layer borders. For every border segment it finds
/// the best assignment, it then makes the assignments for every segment and
/// repeats. Iterates till no change, or a cap is reached.
class EOS_CLASS LayerGrow
{
 public:
  /// &nbsp;
   LayerGrow(const svt::Field<nat32> & segs,Layers & layers,LayerScore & layerScore);
   
  /// &nbsp;
   ~LayerGrow();


  /// Applys a grow/shrink operation on the layer object from construction.
   void operator () (time::Progress * prog = null<time::Progress*>()) const;


  // &nbsp;
   static inline cstrconst TypeString() {return "eos::mya::LayerGrow";}

 private:
  svt::Field<nat32> segs;
  Layers & layers;
  LayerScore & layerScore;
};

//------------------------------------------------------------------------------
 };
};
#endif
