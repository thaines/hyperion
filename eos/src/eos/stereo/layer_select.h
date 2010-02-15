#ifndef EOS_STEREO_LAYER_SELECT_H
#define EOS_STEREO_LAYER_SELECT_H
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


/// \file layer_select.h
/// Given a segmentation where each segment is clasified as belonging to a layer
/// this trys out other possible layer assignments to find an 'optimal' solution
/// in terms of matching to the other view.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/stereo/layer_maker.h"
#include "eos/stereo/warp.h"
#include "eos/filter/seg_graph.h"
#include "eos/time/progress.h"
#include "eos/bs/colours.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This class is essentially an implimentation of Section 5 from 'A layered
/// stereo matching algorithm using image segmentation and global visibility
/// constraints' by Bleyer & Gelautz. Given a LayerMaker that has been run
/// through to completion and the other (right) image from a stereo pair this
/// uses a greedy algorithm to re-assign segments to layers to reduce a
/// cost associated with warping the 'left' image to the 'right' image. After
/// this a re-fitting of the layers is ushally recomended then a repeat of this
/// algorithm etc until no change happens. It indicates no change in its return
/// value from running.
class EOS_CLASS LayerSelect
{
 public:
  /// &nbsp;
    LayerSelect();

  /// &nbsp;
   ~LayerSelect();


  /// Sets the LayerMaker to use.
   void SetMaker(LayerMaker * layerMaker);

  /// Sets the segmentation information to use.
   void SetSegs(nat32 segCount,const svt::Field<nat32> & segs);

  /// Sets the left and right images. We are trying to determine the correct data
  /// for the left, so it maps correctly to the right.
   void SetImages(const svt::Field<bs::ColourRGB> & left,const svt::Field<bs::ColourRGB> & right);

  /// Sets masks for the left and right images, masked pixels are ignored for cost calculation.
   void SetMasks(const svt::Field<bit> & left,const svt::Field<bit> & right);

  /// Sets the costs, occ is the cost of occlusion, in either the left or right image,
  /// dis is the cost of a discontinuity in layer assignment, per shared edge between
  /// pixels.
   void SetCosts(real32 occ,real32 dis);

  /// Sets the number of runs in the greedy improvment loop to go without improvment
  /// to the score before bailing out. Defaults to 12.
   void SetBailOut(nat32 bo);


  /// Runs the algorithm, it edits directly the LayerMaker passed in, returns
  /// true if changes were made, false if no improvment could be found. If
  /// changes were made you ushally re-fit the layer maker and then call this
  /// again.
   bit Run(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::LayerSelect";}


 private:
  LayerMaker * layerMaker;
  nat32 segCount;
  svt::Field<nat32> segs;
  svt::Field<bs::ColourRGB> left;
  svt::Field<bs::ColourRGB> right;
  svt::Field<bit> leftMask;
  svt::Field<bit> rightMask;
  real32 occCost;
  real32 disCost;
  nat32 checkRuns;

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

  // Renders the given segment...
   void RenderSegment(WarpInc & warp,nat32 segment);

  // Given various bits of information calculates the change in discontinuity
  // count given a segment changing its layer to another...
   int32 DiscDelta(filter::SegGraph & segGraph,nat32 seg,nat32 toLayer);
};

//------------------------------------------------------------------------------
 };
};
#endif
