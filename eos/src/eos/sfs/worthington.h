#ifndef EOS_SFS_WORTHINGTON_H
#define EOS_SFS_WORTHINGTON_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \namespace eos::sfs
/// Provides algorithms related to light and shape, most specifically 
/// shape from shading algorithms. Also covers albedo estimation, light models 
/// and other related areas however.

/// \file worthington.h
/// An implimentation of worthington & Hancocks sfs method.

#include "eos/mya/needles.h"
#include "eos/time/progress.h"
#include "eos/filter/edge_confidence.h"
#include "eos/bs/colours.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
/// An implimentation of 'New Constraints on Data-Closeness and Needle Map
/// Consistancy for Shape-from-Shading' by Worthington & Hancock. A algorithm
/// class in my ushal pattern, run once as no reset method is provided.
/// Impliments the DD6 variation provided in the paper, the one with not one but
/// two painful bits of maths.
class EOS_CLASS Worthington
{
 public:
  /// &nbsp;
   Worthington();

  /// &nbsp;
   ~Worthington();


  /// Sets the image to use.
   void SetImage(const svt::Field<real32> & image);

  /// Sets an albedo map to use.
   void SetAlbedo(const svt::Field<real32> & albedo);
   
  /// Sets a mask, it only calculates normals in areas that are set to true,
  /// note that it will also mark further areas as masked, these can then be 
  /// extracted post calculation.
   void SetMask(const svt::Field<bit> & mask);

  /// Sets the light source direction, this comes as a normal that
  /// points towards the light at its infinite position.
   void SetLight(bs::Normal & norm);
   
  /// Sets the number of iterations done.
   void SetIters(nat32 iters = 200);


  /// Produces results, outputting an indication of
  /// progress. (Albit a 1-step ahead variety.)
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the calculated needle map.
   void GetNeedle(svt::Field<bs::Normal> & out) const;

  /// Extracts the mask, areas where this is set to false have no valid normal.
   void GetMask(svt::Field<bit> & out) const;
   
  /// Generates a colour image that represents the needle map. Converts the x,y,z
  /// components of the normal into the r,g,b components of colour, respectivly.
  /// x and y are concidered [-1,1] ranged whilst z is onlt done with [0,1] range.
   void GetNeedleColour(svt::Field<bs::ColourRGB> & out) const;
   
  /// Returns a new, so you must latter delete, svt::Var with an rgb image in.
  /// This will be considerably larger than the input image with an arrow for
  /// each pixel to represent its surface orientation.
   svt::Var * GetArrow() const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  nat32 iterCount; // Number of iterations the algorithm does +1.
  static const real32 sigmaZero = 10.0; // Base sigma used by the algorithm.
  static const nat32 arrowSize = 9; // Longest length of an arrow for arrow map generation - squares are arrowSize*2+1 in size.
 
  // Inputs...
   svt::Field<real32> image;
   svt::Field<real32> albedo;
   svt::Field<bit> givenMask;
   bs::Normal toLight;
 
  // Outputs... 
   svt::Var * var;
   svt::Field<bs::Normal> needle;
   svt::Field<bit> mask;
};

//------------------------------------------------------------------------------
 };
};
#endif
