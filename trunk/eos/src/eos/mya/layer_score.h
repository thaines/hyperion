#ifndef EOS_MYA_LAYER_SCORE_H
#define EOS_MYA_LAYER_SCORE_H
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


/// \file layer_score.h
/// Provides the ability to score segment combinations, via various metrics,
/// for use when applying various possible editting operations onto the layers 
/// object.

#include "eos/types.h"
#include "eos/mya/surfaces.h"
#include "eos/mya/ied.h"
#include "eos/mya/layers.h"
#include "eos/ds/sort_lists.h"
#include "eos/stereo/warp.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
/// Definition of a scoring object, this never returns an actual final score, it
/// simply returns how the score would change for any given operation, so you 
/// can decide which operation would be best to apply for the Layers object in 
/// its current state.
class EOS_CLASS LayerScore : public Deletable
{
 public:
  /// &nbsp;
   LayerScore(Layers & layers);
   
  /// &nbsp;
   ~LayerScore();
 
 
  /// Returns the change in the score that would occur if the
  /// operation was actualy applied. A positive return value 
  /// indicates the score getting worse, a negative the score 
  /// getting better.
   virtual real32 IfSetSegLayer(nat32 seg,nat32 layer) const = 0;

  /// Returns the change in the score that would occur if the
  /// operation was actualy applied. A positive return value 
  /// indicates the score getting worse, a negative the score 
  /// getting better.
   virtual real32 IfMergeLayers(nat32 lay1,nat32 lay2) const = 0;

  /// Returns the change in the score that would occur if the
  /// operation was actualy applied. A positive return value 
  /// indicates the score getting worse, a negative the score 
  /// getting better.
   virtual real32 IfSeperate(nat32 seg) const = 0;


  /// Called when the equivalent is called for the layer object, after any call to Rebuild().
   virtual void OnSetSeglayer(nat32 seg,nat32 layer);
   
  /// Called when the equivalent is called for the layer object, after any call to Rebuild().
   virtual void OnMergeLayers(nat32 lay1,nat32 lay2);
   
  /// Called when the equivalent is called for the layer object, after any call to Rebuild().
   virtual void OnSeperate(nat32 seg);
   
  /// Called when Rebuild is called on the layer object, after the call to Rebuild().
   virtual void OnRebuild();
 
  
  /// &nbsp;
   virtual cstrconst TypeString() const = 0;


 protected:
  Layers & layers;
};

//------------------------------------------------------------------------------
/// A layer configuration scoring object, scores layers by how well they fit the
/// underlying data, this is simply an echoing of the Layers object fitting costs
/// employed when deciding which layer to use. This simply equates to the number
/// of outliers in the entire image weighted by the surface types for each layer.
/// Internally caches the scores for each particular layer configuration, so it
/// will only calculate each required configuration once.
class EOS_CLASS OutlierScore : public LayerScore
{
 public:
  /// &nbsp;
   OutlierScore(Layers & layers);
   
  /// &nbsp;
   ~OutlierScore();
 
 
  /// &nbsp;
   real32 IfSetSegLayer(nat32 seg,nat32 layer) const;

  /// &nbsp;
   real32 IfMergeLayers(nat32 lay1,nat32 lay2) const;

  /// &nbsp;
   real32 IfSeperate(nat32 seg) const;


  /// &nbsp;
   cstrconst TypeString() const;
   
 private:
  // Own foi for internal use, to save repeated creation for each member that uses it.  
   mutable ds::Array<bit> foi;
};

//------------------------------------------------------------------------------
/// Scores an image by warping it with the calculated disparity to the other 
/// image in a stereo pair, and summing both an occlusion and colour difference
/// cost which can be wieghted at will.
class EOS_CLASS WarpScore : public LayerScore
{
 public:
  /// &nbsp;
   WarpScore(Layers & layers,nat32 segCount,const svt::Field<nat32> & segs,const svt::Field<bs::ColourRGB> & left,const svt::Field<bs::ColourRGB> & right);
   
  /// &nbsp;
   ~WarpScore();
 
 
  /// This sets the relative occlusion cost compared to the absolute colour 
  /// difference cost, note that colour is in the [0..1] range so give the 
  /// occlusion cost accordingly. Defaults to 20/255.
   void SetOcclusionCost(real32 cost);
   
  /// This sets the multiplyign factor for converting from depth to disparity, 
  /// defaults to 1.
   void SetDisparityMult(real32 mult);

 
  /// &nbsp;
   real32 IfSetSegLayer(nat32 seg,nat32 layer) const;

  /// &nbsp;
   real32 IfMergeLayers(nat32 lay1,nat32 lay2) const;

  /// &nbsp;
   real32 IfSeperate(nat32 seg) const;


  /// &nbsp;
   void OnSetSeglayer(nat32 seg,nat32 layer);

  /// &nbsp;
   void OnMergeLayers(nat32 lay1,nat32 lay2);

  /// &nbsp;
   void OnSeperate(nat32 seg);
 
  /// &nbsp;
   void OnRebuild();


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  mutable stereo::WarpInc warp;
  
  nat32 segCount;
  svt::Field<nat32> segs;
  svt::Field<bs::ColourRGB> left;
  svt::Field<bs::ColourRGB> right;
  
  real32 mult;
  
  
  // A data structure used to optimise rendering segments, we simply arrange a
  // linked list of all pixels in each segment, as sorted rows, so we can
  // quickly iterate all of them when re-rendering a segment...
   struct Node
   {
    Node * next;
    nat32 x,y; // Its image coordinates.
    bs::ColourRGB colour; // Its colour.
    bit paired; // Set to true if the next node is at x+1 position, i.e. we should be doing a span here. Saves work in the inner loop.
    bit incStart; // We are the start of a span - extend by .5
    bit incEnd; // We are the end of a span - extend by .5
   };

   Node ** sed; // Array of segCount size, points to first in each list.


  // Method to add entire segments...
   void AddSeg(nat32 seg,Surface * surface) const;
   
  // Internal data, to save its re-allocation each operation...
   mutable ds::Array<bit> foiA;
   mutable ds::Array<bit> foiB;
   mutable ds::Array<bit> foiC;
};

//------------------------------------------------------------------------------
/// A score encapsulator, this encapsulates another scoring object and adds to 
/// its costs a change-of-surface cost, discouraging dicontinuities.


//------------------------------------------------------------------------------
 };
};
#endif
