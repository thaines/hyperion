#ifndef EOS_FILTER_SIFT_H
#define EOS_FILTER_SIFT_H

#ifdef EOS_INC_PATENTED
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


/// \file sift.h
/// Contains an implimentation of David Lowe's SIFT algorithm.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/svt/var.h"
#include "eos/filter/pyramid.h"
#include "eos/time/progress.h"
#include "eos/ds/arrays.h"
#include "eos/bs/colours.h"
#include "eos/math/vectors.h"

namespace eos
{
 namespace filter
 {
//-----------------------------------------------------------------------------
/// This class provides a partial implimentation of the SIFT algorithm, as 
/// documented by 'Distinctive Image Features from Scale-Invariant Keypoints'
/// by David Lowe from IJCV 2004. Impliments just the keypoint discovery, pruning
/// and orientation assignment, not the descriptor, which is covered by the
/// SiftFeature class. Sections 3 to 5 of the paper in other words. Whilst many
/// parameters could be exposed for this algorithm none have as optimal values
/// have been given by the paper. Note that the base of the pyramid contains an
/// image of double size, so you have to concider this when managing the Keypoint
/// x and y values.
/// 
/// Note that there is a patent on the sift algorithm as a whole, so the legality
/// of using this class is questionable outside of research/personal use.
/// A license should be obtained in such circumstances.
class EOS_CLASS SiftKeypoint
{
 public:
  /// &nbsp;
   SiftKeypoint();
   
  /// &nbsp;
   ~SiftKeypoint();
   
 
  /// Each time this is called with an image it rebuilds the keypoint array
  /// for the new data.
   void Run(const svt::Field<real32> & image,time::Progress * prog = null<time::Progress*>());
 
   
  /// Structure that represents any given keypoint.
   struct Keypoint
   {
    nat32 octave; ///< &nbsp;
    nat32 scale; ///< &nbsp;
    
    nat32 x; ///< Uses pyramid octave coordinates, multiply by math::Pow(2,octave)/2 to get original.
    nat32 y; ///< Uses pyramid octave coordinates, multiply by math::Pow(2,octave)/2 to get original.

    real32 xOff; ///< [-0.5,0.5] - The refinement of the x position.
    real32 yOff; ///< [-0.5,0.5] - The refinement of the y position.
    real32 sOff; ///< [-0.5,0.5] - The refinement of the scale.
    
    real32 rot; ///< The orientation of the keypoint in radians.
   }; 
   
   
  /// Returns how many keypoints were found.
   nat32 Keypoints() const;
   
  /// Accessor to get access to a keypoint.
   const Keypoint & operator [] (nat32 i) const;
  

  /// Provides access to the pyramid created in calculating the keypoints.
   const Pyramid & GetPyramid() const {return pyramid;}
   
   
  /// Renders out to an image the size of the base image handed in arrows
  /// representing each keypoint, indicating scale and orientation.
  /// You get to choose the arrow colour.
   void Render(svt::Field<bs::ColourRGB> & image,const bs::ColourRGB & pcol = bs::ColourRGB(1.0,0.0,1.0),const bs::ColourRGB & lcol = bs::ColourRGB(1.0,1.0,0.0));


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::filter::SiftKeypoint";}

 
 private:
  Pyramid pyramid;
  ds::Array<Keypoint> data;
  
  // Some internal constants...
   static const real32 initialSmooth = 1.5;
   static const real32 minContrast = 0.03;
   static const real32 maxCurveRatio = 10.0;
   static const nat32 rotBins = 36; // Bins in the rotation histogram.
   static const real32 sdScale = 1.5; // Multiplier of the scale to get the sd for the rotation weighting gaussian.
   static const real32 sdMult = 2.0; // What factor of the gaussian sd to go out to for deciding the window to search.
   static const real32 peekMult = 0.8; // Expresses how close any maxima has to be to the maximum rotation response to be selected.
};

//-----------------------------------------------------------------------------
/// This class augments the SiftKeypoint class with calculation of the feature
/// vectors on being given a SiftKeypoint object to get its data from. Builds
/// feature vectors identical to the description in the 'Distinctive Image
/// Features from Scale-Invariant Keypoints' by David G Lowe '04 paper, with
/// 128 entry keys.
/// 
/// Note that there is a patent on the sift algorithm as a whole, so the legality
/// of using this class is questionable outside of research/personal use.
/// A license should be obtained in such circumstances.
class EOS_CLASS SiftFeature
{
 private:
  static const nat32 dimRes = 16;
  static const nat32 dimSamp = 4;
  static const nat32 rotSamp = 8;  
  
  static const real32 maxFeat = 0.2;
  
 public:
  static const nat32 fvSize = dimSamp * dimSamp * rotSamp;
 
  /// &nbsp;  
   SiftFeature();
   
  /// &nbsp;
   ~SiftFeature();


  /// Each time this is called with a SiftKeypoint it rebuilds the features to match
  /// with its array of keypoints.
   void Run(const SiftKeypoint & kps,time::Progress * prog = null<time::Progress*>());
   
  
  /// Provides access to the feature vectors, the size of this array is dictated
  /// by the number of keypoints with which it shares indexes in the SiftKeypoint
  /// object it was Run(...) with.
   const math::Vect<fvSize> & operator[] (nat32 kp) const;

   
  /// Returns how many rotation samples there are.
   nat32 Rots() const {return rotSamp;}
   
  /// Returns how many dimension samples there are. 
   nat32 Dims() const {return dimSamp;}
   
  /// A helper method, given a Vect and the relevent position it returns a
  /// reference to the relevent entry in the vector. Note that the keypoint
  /// rotation axis goes along the +ve x axis in the coordinate system.
   static real32 & Entry(math::Vect<fvSize> & vec,nat32 x,nat32 y,nat32 rot)
   {return vec[(y*dimSamp + x)*rotSamp + rot];}
   
   
  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::filter::SiftFeature";}


 private:
  ds::Array< math::Vect<fvSize> > data;	
};

//------------------------------------------------------------------------------
 };
};
#endif#endif

