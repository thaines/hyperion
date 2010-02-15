#ifndef EOS_STEREO_WARP_H
#define EOS_STEREO_WARP_H
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


/// \file warp.h
/// Provides a particularly advanced forward warping class, with incrimental
/// behaviour for changing disparities of segments and calculating a cost for
/// being different from the image it is warping to.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/bs/colours.h"
#include "eos/mem/preempt.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// A simple method that impliments forward warping to create a warped view, you
/// simply supply an image, a disparity map and a strength, where 1.0 should
/// get you the right image (0.0 the left, but thats pointless) and away it goes.
/// It uses interpolation to produce a relativly clean result, it only interpolates
/// over distances less than the passed in parameter co, so you can decide where
/// it draws the line between a steep surface and a shadow/abrupt change of depth.
EOS_FUNC void ForwardWarp(const svt::Field<bs::ColourRGB> & image,const svt::Field<real32> & disp,svt::Field<bs::ColourRGB> & out,real32 co = 2.5,real32 t = 1.0,const svt::Field<bit> * mask = 0);

//------------------------------------------------------------------------------
/// Does incrimental forward warping, though ironically dosn't do the warping
/// itself. Stores a large number of nodes, where each node represents the
/// writting of a single pixel; each node is a member of a segment, a layer and
/// a pixel. You can add new pixels to segments or delete entire segments.
/// Each pixel also has an associated true colour, a cost is calculated,
/// incrimentally with each change, which can be obtained at any time, so a
/// change can be tested for improving the results without any serious computation.
/// The cost function consists of the following terms:
/// - Sum of absolute differences of actual image, the difference between the true colour and rendered colour. (Excluding areas with no pixels.)
/// - Sum of occlusion weight multiplied by the number of occlusions in both the left and right images.
class EOS_CLASS WarpInc
{
 public:
  /// Sets the size of the image and the 'true colour' data, and initialises
  /// the score. You also give it the number of segments.
   WarpInc(const svt::Field<bs::ColourRGB> & base,nat32 segments);

  /// &nbsp;
   ~WarpInc();


  /// Allows you to set a mask for the right image, this will prevent masked
  /// pixels from being involved in the cost. Optional.
  /// Call before any real work is done.
   void SetMask(const svt::Field<bit> & mask);


  /// Sets the relative weight for the occlusion part of the cost function. Remember that
  /// we use [0..1] range colours, not [0..255], so these weight must be /255.0 to match those
  /// given in most papers.
  /// \param occ Occlusion cost, number of occlusions is multiplied by this. Defaults to 0.1
   void Weights(real32 occ);


  /// Returns the width of the image.
   nat32 Width();

  /// Returns the height of the image.
   nat32 Height();


  /// Renders a pixel to the image, adjusting everything as required.
  /// The caller is responsable for range checking and factoring in the
  /// disparity for the given coordinate. (i.e. the actual warping.)
   void Add(nat32 x,nat32 y,real32 disp,const bs::ColourRGB & colour,nat32 segment);

  /// Removes all pixels in a segment.
   void Remove(nat32 segment);


  /// Returns the score for the class in its current state.
   real32 Score() const;

  /// Returns the score for just the absolute colour difference.
   real32 ScoreDiff() const;

  /// Returns the score for just the occlusion cost.
   real32 ScoreOcc() const;

  /// Returns the score for the class in its current state, except instead of using
  /// the incrimental calculation it calculates it directly. This is far slower to
  /// Score() but has the advantage of simpler code, so its less likelly to be wrong.
  /// This code is provided only for the purpose of debugging.
   real32 RawScore() const;

  /// Outputs the image currently stored within to the given Field.
   void GetImage(svt::Field<bs::ColourRGB> & out);


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::WarpInc";}


 private:
  // The Node structure...
   struct Node
   {
    Node * next; // Circular linked list with dummy node as pixel,
    Node * last; // kept sorted with highest disparity first in the list.

    Node * segNext; // Singerly linked list of nodes that are a member of the same segment.

    Node * head; // Pointer to the dummy node of the pixel, if null then is a dummy node.
    bs::ColourRGB colour; // True colour if in the dummy node.
    bit count; // True to count it, false to ignore this pixel for costs. Relevant for dummy only.
    real32 disp;
   };

  // A memory allocator - used by the code to speed up memory allocations, as there bloody slow otherwise...
   mem::PreAlloc<sizeof(Node),300000> nodeAlloc; // Consumes a bit over 10 meg, about enough for a 640x480 image.

  // The data structure entry points...
   // All the dummy nodes for the linked list at each pixel...
    nat32 width;
    nat32 height;
    Node * pixel;
   // Pointers to first node for each segment...
    nat32 segments;
    Node ** segs;

  // Incrimental score stuff...
   real32 occCost;

   real32 diffSum; // Sum of all differences from true colour.
   nat32 occCount; // Number of occlusions.
};

//------------------------------------------------------------------------------
 };
};
#endif
