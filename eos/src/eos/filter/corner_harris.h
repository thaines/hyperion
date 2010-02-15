#ifndef EOS_FILTER_CORNER_HARRIS_H
#define EOS_FILTER_CORNER_HARRIS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file corner_harris.h
/// Provides the harris corner detector, enough said.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/time/progress.h"
#include "eos/ds/arrays.h"
#include "eos/ds/arrays2d.h"
#include "eos/bs/geo2d.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This class represents a corner, as output by corner detectors.
/// Consists of a location and strength.
class EOS_CLASS Corner
{
 public:
  /// Wrong way arround - this is for using this in a heap where we want the 
  /// highest first.
   bit operator < (const Corner & rhs) const {return strength>rhs.strength;}
 
  bs::Pnt loc;
  real32 strength;
};

//------------------------------------------------------------------------------
/// A Harris corner detector, uses the standard parameters, all you provide is
/// a greyscale image and a limit on how many corners to detect, it then edits
/// a passed array to contain all these corners.
/// \param img A greyscale image to extract corners from.
/// \param maxSize Maximum number of features to find, a limit on how big the output array can get.
/// \param out The ouput array, set to contain the corners found, sorted so the strongest corner comes first. Resized to teh number of corners found.
/// \param prog Progress bar, as this is not that fast an operation.
EOS_FUNC void CornerHarris(const svt::Field<real32> & img,
                           nat32 maxSize,
                           ds::Array<Corner> & out,
                           time::Progress * prog = null<time::Progress*>());

//------------------------------------------------------------------------------
/// This generates a 'corner snapper image', this is a sub-pixel snapping
/// coordinate for every pixel in an image, you can then use this as a lookup
/// table to snap user selected coordinates to. Uses the harris corner detector
/// to find its corners for snapping.
class EOS_CLASS HarrisSnap
{
 public:
  /// &nbsp;
   HarrisSnap();
   
  /// &nbsp;
   ~HarrisSnap();
   
  
  /// Sets the irradiance field to generate a snapping grid for.
   void Set(const svt::Field<real32> & img);
   
  /// Sets the falloff rate for strength from a corner, used to decide between
  /// strong corners and less strong corners by all corners giving out a field 
  /// with this falloff and the strongest winning.
  /// Defaults to 0.75
   void Set(real32 falloff);
  
   
  /// Run rabbit, run.
   void Run(time::Progress * prog = null<time::Progress*>());
   
   
  /// For any given pixel this returns the snapping coordinate. Works for
  /// ranges outside the image, but will allways return an in-image 
  /// coordinate.
   void Get(int32 x,int32 y,bs::Pnt & out);
  
  
  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::filter::HarrisSnap";}


 private:    
  // In...
   real32 falloff;
   svt::Field<real32> img;

  // Out...  
   ds::Array2D<bs::Pnt> snap;
};

//------------------------------------------------------------------------------
 };
};
#endif
