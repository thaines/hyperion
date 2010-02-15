#ifndef EOS_MYA_LAYER_GROW_H
#define EOS_MYA_LAYER_GROW_H
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
