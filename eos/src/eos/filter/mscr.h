#ifndef EOS_FILTER_MSCR_H
#define EOS_FILTER_MSCR_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file mscr.h
/// Provides an implimentation of the MSCR algorithm, plus support stuff for 
/// generating keys and matching 'em.

#include "eos/types.h"
#include "eos/math/matrices.h"
#include "eos/bs/colours.h"
#include "eos/bs/geo2d.h"
#include "eos/ds/arrays.h"
#include "eos/ds/stacks.h"
#include "eos/time/progress.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This returns the distance between two pixels - it is used throughout the
/// MSCR algorithm when handling merges. (Its just the chi squared distance.)
EOS_FUNC real32 DistMSCR(bs::ColourRGB & a,bs::ColourRGB & b);

//------------------------------------------------------------------------------
/// Stable region as found by the MSCR class - its ultimate output is an array
/// of these. Includes sufficient details to obtain the original region plus 
/// various stats on the region...
class EOS_CLASS StableRegion
{
 public:
  /// The coordinate of an arbitary pixel in the region - a flood fill from here
  /// allows its precise membership to be found.
   bs::Pos pixel;
   
  /// The pixel difference of the last merge that created the region, used to 
  /// limit the flood fill.
  /// (i.e. if you do a flood fill from pixel on the condition of being less than
  /// this value you get the region represented. Less than is enforced by taking
  /// half way between the dist of the final merge and the next merge.)
   real32 dist;
 
  /// Total number of pixels in the region.
   nat32 area;
 
  /// Centre of region - mean of the pixel coordinates in the region.
   bs::Pnt mean;
   
  /// Covariance matrix of pixel coordinates in region.
   math::Mat<2,2> covar;
};

//------------------------------------------------------------------------------
/// Given an image this calculates all its MSCR regions, including collecting 
/// various stats about them. Includes support for exporting various diagnostic
/// images.
/// Based on the paper 'Maximally Stable Colour Regions for Recognition and Matching'
/// by Per-Erik Forssen.
class EOS_CLASS MSCR
{
 public:
  /// &nbsp;
   MSCR();
   
  /// &nbsp;
   ~MSCR();


  /// Sets the input image - must always be called before Run.
   void Input(const svt::Field<bs::ColourRGB> & input);
  
  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Returns how many regions have been found by the Run method.
   nat32 Size() const {return out.Size();}
   
  /// Accesses a found region.
   const StableRegion & operator[](nat32 i) const {return out[i];}
  

  /// &nbsp;
   static cstrconst TypeString() {return "eos::filter::MSCR";}


 private:
  // Parameters...
   nat32 minSize; // Regions are only accepted if larger than this, defaults to 60.
   real32 areaCap; // Defines when two regions are close enough to be merged. larger/smaller < areaCap. 1.1
   real32 minDim; // Minimum ellipsoid width - if less than this a region is dropped. Default is 1.5.
   

  // Input image...
   svt::Field<bs::ColourRGB> image;
   
  // Output array...
   ds::Array<StableRegion> out;


  // This stores a 'crack', that is the coordinates of two adjacent pixels 
  // and the difference between them.
   struct Crack
   {
    int32 x;
    int32 y;
    real32 dist; // Difference between the two pixels.
    byte dir; // 0=(1,0),1=(0,1),2=(1,1),3=(-1,1)
    // (x,y) is coordinate of first pixel, offset with dir to get second.
    
    bit operator < (const Crack & rhs) const {return dist<rhs.dist;}
   };


  // This stores a region in the merge forest - contains all the information 
  // required to do the merges, extract information afterwards and generally
  // run the algorithm. At the start of the algorithm each Region is a pixel.
   struct Region
   {
    // parent pointer for forest, for merging - null if its the parent.
     Region * parent;
     Region * Parent() // Accessor of above, does path shortening.
     {
      if (parent==null<Region*>()) return this;
      else
      {
       parent = parent->Parent();
       return parent;
      }
     }
     
    // These variables allow tracking of how the region grows with the increase
    // in the pixel difference accepted.
     Region * past;
     real32 dist; // Dist of crack that created this region.
     
    // This is used during the stable region search to store how good a region is.
     real32 distDiff;
     
    // Various statistics of the region - only valid if parent==null or its
    // in a current merge log...
     nat32 size; // Number of pixels in the region.
     real32 eX; // Incrimentatlly calculated expectation of x for all values in region.
     real32 eY; // Above for y.
     real32 eXX; //  Above for x squared.
     real32 eYY; // Above for y squared.
     real32 eXY; // Above for x times y.
   };

   
  // Helper method - this searches a merge log made from Region-s to find
  // stable regions... (baseRegion is used to calculate coordinates.)
   void FindStableRegions(Region * ml,ds::Stack<StableRegion> & output,Region * baseRegion);
   
  // Helper method used by above helper method - given a region it stores it,
  // assuming it thinks its good enough...
   void StoreStableRegion(Region * r,ds::Stack<StableRegion> & output,Region * baseRegion);
};

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
 };
};
#endif
