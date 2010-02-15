#ifndef EOS_FILTER_PYRAMID_H
#define EOS_FILTER_PYRAMID_H
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


/// \file pyramid.h
/// Provides a class that can create a pyramid of gaussians.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/svt/var.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This constructs a pyramid of gaussians, using octaves to indicate a halfing
/// of the image size and scales to indicate levels within each octave. Each 
/// scale has a gaussian blur applied such that the blur level doubles for each
/// octave. It also has scales beyond the transfer point to the next octave so
/// they can be used by further algorithms to extend boundarys etc.
class EOS_CLASS Pyramid
{
 public:
  /// &nbsp;
   Pyramid();
   
  /// &nbsp;
   ~Pyramid();

   
  /// Sets the number of scales per octave, and the number of extra scales to
  /// go beyond the transfer point. Note that the gaussian standard deviation
  /// multiplier will be set so that Pow(sd,scales)==2.0. Scales defaults to 3,
  /// so sd defaults to 2^(1/3); extra defaults to 3, the required extras for
  /// a DogPyramid.
   void SetScales(nat32 scales,nat32 extra);
  
  /// Sets the maximum number of octaves and the smallest image dimension,
  /// will stop generation on reaching either of these points. Default to
  /// 32 and 16 respectivly. (The 32 makes maxOctaves irrelevent, due to 
  /// the size limits of nat32's.)
   void SetStop(nat32 maxOctaves,nat32 smallestDim);
  
   
  /// Constructs the pyramid, you must call this before doing anything else
  /// other than setting parameters. Do not change parameters after calling
  /// this, you can recall it to rebuild.
   void Construct(const svt::Field<real32> & image);


  /// Returns how many scales there are in each octave.
   nat32 Scales() const;

  /// Returns how many extra scales there are, in positions scales,scales+1 etc.
   nat32 Extras() const;

  /// Returns how many octaves there are.
   nat32 Octaves() const;
   
  /// Returns the width of a given octave.
   nat32 OctaveWidth(nat32 i) const {return octave[i]->Size(0);}
  
  /// Returns the height of a given octave.
   nat32 OctaveHeight(nat32 i) const {return octave[i]->Size(1);}


  /// Allows you to obtain a field to the internal data for a particular 
  /// scale/octave. scale is [0..Scales()+Extras()-1].
   void Get(nat32 octave,nat32 scale,svt::Field<real32> & out) const;


  /// A helper method, given the octave and scale this returns the actual gaussian
  /// blur value this equates to. Note that this is not the fastest of methods, as its
  /// quite complex mathematically.
   real32 Sd(nat32 octave,real32 scale) const
   {
    return math::Pow<real32>(2.0,octave) * math::Pow<real32>(math::Pow(2.0,1.0/real32(scales)),scale);
   }


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::filter::Pyramid";}


 private:	
  // Each octave is stored in a var, where we just use the tokens as numbers to
  // directly index each scale, with level str::Token(1) being the level with 
  // the lowest smoothing levels.
   nat32 octaves; // Size of below array.
   nat32 scales; // Scales per octave.
   nat32 extras; // Extra scales above and beyond the transfer point.
   svt::Var ** octave;

   real32 baseBlur;
   nat32 maxOctaves;
   nat32 smallestDim;
};

//------------------------------------------------------------------------------
 };
};
#endif
