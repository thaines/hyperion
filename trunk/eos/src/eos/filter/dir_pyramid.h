#ifndef EOS_FILTER_DIR_PYRAMID_H
#define EOS_FILTER_DIR_PYRAMID_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file dir_pyramid.h
/// Provides a class that can create a pyramid of magnitudes and directions, 
/// using the ushall definition.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/svt/var.h"
#include "eos/filter/pyramid.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This class represents and will build a pyramid of magnitudes and orientation
/// for each pixel in a gaussian pyramid. Uses differences across the pixel that
/// do not involve the pixel itself.
class EOS_CLASS DirPyramid
{
 public:  
  /// &nbsp;
   DirPyramid();

  /// &nbsp;
   ~DirPyramid();


  /// Constructs the pyramid, you must call this before doing anything else.
  /// You must only call this method once.
   void Construct(const Pyramid & pyramid);


  /// Returns how many scales there are in each octave.
   nat32 Scales() const;

  /// Returns how many extra scales there are in each octave.
   nat32 Extras() const;

  /// Returns how many octaves there are.
   nat32 Octaves() const;

  /// Allows you to obtain a field to the internal data for a particular 
  /// scale/octave. Outputs magnitude Field's.
   void GetMag(nat32 octave,nat32 scale,svt::Field<real32> & mag) const;

  /// Allows you to obtain a field to the internal data for a particular 
  /// scale/octave. Outputs direction Field's, in radians.
   void GetDir(nat32 octave,nat32 scale,svt::Field<real32> & dir) const;

   
  /// Allows you to sample the magnitude field with linear interpolation.
   real32 SampleMag(nat32 octave,real32 scale,real32 x,real32 y) const;
   
  /// Allows you to sample the orientation field with linear interpolation.
   real32 SampleRot(nat32 octave,real32 scale,real32 x,real32 y) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::filter::DirPyramid";}


 private:	
  // Each octave is stored in a var, where we just use the tokens as numbers to
  // directly index each scale, with level str::Token(1) being the level with 
  // the lowest smoothing levels.
   nat32 octaves; // Size of below arrays.
   nat32 scales;
   nat32 extras;
   svt::Var ** magOctave;
   svt::Var ** dirOctave;
};

//------------------------------------------------------------------------------
 };
};
#endif
