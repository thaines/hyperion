#ifndef EOS_FILTER_SEG_K_MEAN_GRID_H
#define EOS_FILTER_SEG_K_MEAN_GRID_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

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


/// \file seg_k_mean_grid.h
/// Provides a segmentation algorithm designed to produce lots of similar sized 
/// segments.

#include "eos/types.h"
#include "eos/bs/colours.h"
#include "eos/ds/arrays.h"
#include "eos/ds/arrays2d.h"
#include "eos/time/progress.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// Segmentation algorithm, based on the one used in a past stereopsis paper I 
/// can no longer find.
/// Uses k-means, initialised with the image divided into a fine grid - the 
/// k-means then distorts the grid to the image. This inevitably leads to an 
/// oversegmentation, but it also limits the size of segments to a relativly
/// tight range, making it useful for certain kinds of algorithm.
/// The small segments also limit the potential affect of the inevitable errors.
class EOS_CLASS MeanGridSeg
{
 public:
  /// &nbsp;
   MeanGridSeg();

  /// &nbsp;
   ~MeanGridSeg();


  /// Provides the input image, must be called before Run.
   void SetImage(const svt::Field<bs::ColourLuv> & in);
   
  /// Sets the grid size, defaults to 12 pixels (in each dimension.) - this is 
  /// used to initialise the k means algorithm.
   void SetSize(nat32 dim);
   
  /// Sets the minimum segment size - segments smaller than this will bleed all
  /// there pixels to surrounding segments. Defaults to 32.
   void SetMinSeg(nat32 size);
   
  /// Sets the relative costs of the 2 distances involved - colour and spatial distance.
  /// Defaults are 2 for colMult and 1 for spatialMult.
   void SetDist(real32 colMult,real32 spatialMult);
   
  /// Sets the maximum number of iterations - defaults to 1000.
   void SetMaxIters(nat32 iters);


  /// &nbsp;
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Returns the number of segments found.  
   nat32 Segments() const;

  /// Extracts the resulting segmentation. Must be the same size as the input image.
   void GetSegments(svt::Field<nat32> & out) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::filter::MeanGridSeg";}


 private:
  // Parameters...
   nat32 dim;
   nat32 minSize;   
   real32 colMult;
   real32 spatialMult;
   nat32 maxIters;
   
  // Input...
   svt::Field<bs::ColourLuv> image;
   
  // Output...
   nat32 segments;
   ds::Array2D<nat32> out;
 
  // Runtime...
   struct Mean
   {
    nat32 samples; // Number of samples involved.
    real32 x,y;
    real32 l,u,v;
   };
   
   void CalcMeans(const ds::Array2D<nat32> & seg,ds::Array<Mean> & mean);
   nat32 UpdatePixels(const ds::Array2D<nat32> & oldSeg, ds::Array2D<nat32> & newSeg,
                      const ds::Array<Mean> & mean); // Returns the number of changes.
   
   real32 DistSqr(const Mean & m,nat32 x,nat32 y) const
   {
    if (m.samples<minSize) return math::Infinity<real32>();
   
    real32 ret = spatialMult * (math::Sqr(m.x-real32(x)) + math::Sqr(m.y-real32(y)));
    
    const bs::ColourLuv & c = image.Get(x,y);
    ret += colMult * (math::Sqr(c.l-m.l) + math::Sqr(c.u-m.u) + math::Sqr(c.v-m.v));
    
    return ret;
   }
};

//------------------------------------------------------------------------------
 };
};
#endif
