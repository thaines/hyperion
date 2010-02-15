#ifndef EOS_FILTER_DOG_PYRAMID_H
#define EOS_FILTER_DOG_PYRAMID_H
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


/// \file dog_pyramid.h
/// Provides a class that can create a pyramid of differences of gaussians.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/svt/var.h"
#include "eos/ds/lists.h"
#include "eos/math/vectors.h"
#include "eos/math/matrices.h"
#include "eos/filter/pyramid.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This class represents and will build a pyramid of differences of gaussian,
/// it also provides methods for finding scale invariant features and 
/// interpolating there positions and a curvature measure of them. Based on the
/// description given by Lowe for the SIFT paper.
class EOS_CLASS DogPyramid
{
 public:  
  /// &nbsp;
   DogPyramid();

  /// &nbsp;
   ~DogPyramid();
  
   
  /// Constructs the pyramid, you must call this before doing anything else.
  /// the Extras() method of the given pyramid must return at least 3 otherwise
  /// it will go rather hairy. You must only call this method once.
   void Construct(const Pyramid & pyramid);
  
  
  /// Returns how many scales there are in each octave. Note that there is 
  /// allways an extra two scales, with index value being the size returned
  /// and size returned+1, they equate with the first two scales of the next
  /// octave.
   nat32 Scales() const;
   
  /// Returns 2, to be consistant with the other pyramids.
   nat32 Extras() const {return 2;}
  
  /// Returns how many octaves there are.
   nat32 Octaves() const;
  
  /// Allows you to obtain a field to the internal data for a particular 
  /// scale/octave. scale is [0..Scales()+1], because of the extra 2 entrys.
   void Get(nat32 octave,nat32 scale,svt::Field<real32> & out) const;
  
  
  /// Class used to output positions in the dog, consisting of x, y, and octave/scale,
  /// with x and y in original image coordinates. (i.e. divide by octave+1 to get 
  /// coordinates that work with the output of GetDog.)
   struct EOS_CLASS Pos
   {
    nat32 x; ///< Uses pyramid octave coordinates, multiply by math::Pow(2,octave) to get original.
    nat32 y; ///< Uses pyramid octave coordinates, multiply by math::Pow(2,octave) to get original.
    
    nat32 oct; ///< The octave, [0,Octaves()-1)] in which the point was found.
    nat32 scale; ///< The scale within the octave, can't be the top or bottom scales as they will appear in other octaves.
    
    real32 xOff; ///< [-0.5,0.5] An offset for the x coordinate, filled in by the refine method.
    real32 yOff; ///< [-0.5,0.5] An offset for the y coordinate, filled in by the refine method.
    real32 sOff; ///< [-0.5,0.5] An offset for the scale, filled in by the refine method.
    
    real32 value; ///< The dog value of the position, to save looking it up, is adjusted by refinement.
   };
   
    
  /// Extracts a list of all minimas and maximas in scale space throughout the
  /// pyramid. Appends all found extrema to the given list. Will not produce 
  /// points on the border, as such pixels can't be trusted.
   void GetExtrema(ds::List<Pos> & out) const;
  
  /// Given a pos it fills in its offset fields and updates its value by interpolation
  /// to provide a better position. Returns true on success, false on failure. There
  /// are many possible failure scenarios, mostly indicating the point is unsuitable
  /// for use anyway. This is not strictly neccesary, but it does improve the final
  /// result.
   bit RefineExtrema(Pos & pos) const;
   
  /// Returns the ratio of principal curvatures, note that if the curvatures have 
  /// different signs a negative value will be returned, you will want to check for
  /// this as it indicates an invalid point. Closelly related to a harris corner
  /// detector, applied to the point in question in dog space.
   real32 CurveRatio(const Pos & pos) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::filter::DogPyramid";}
  
  
 private:	
  // Each octave is stored in a var, where we just use the tokens as numbers to
  // directly index each scale, with level str::Token(1) being the level with 
  // the lowest smoothing levels.
   nat32 octaves; // Size of below array.
   nat32 scales; // Scales per octave, remember to +2 to include the extra scale.
   svt::Var ** octave;


  // Helper method...
   // This calculates the (x,y,s) derivative at a given point, don't give it
   // points on the border, it dosn't like it.
    void Deriv(const Pos & pos,math::Vect<3> & out) const;
   // This calcualtes the hessian at a given point.
    void Hessian(const Pos & pos,math::Mat<3> & out) const;
};

//------------------------------------------------------------------------------
 };
};
#endif
