#ifndef EOS_FILTER_GRAD_WALK_H
#define EOS_FILTER_GRAD_WALK_H
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


/// \file grad_walk.h
/// Provides a method of calculating the gradiant of an image that uses random
/// walks. (i.e. isn't that blasted Sobel operator.)

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/time/progress.h"
#include "eos/ds/arrays2d.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This generates fields of dy and dx for a given luminence map using random
/// walks. (Well, sort of dy and dx; theres no defined surface, and scale
/// depends on the parameters.)
/// Ignoring the fact that the Sobel operator and its ilk are crap the primary
/// motivation for this is that in noisy areas where gradiant is ill defined 
/// anyway the gradiants will be noisy, and cancel out, either in averaging or
/// in doing lots of walks.
/// Runtime of this algoirthm is O(pixel_count * walks * length), so can get slow very quick.
///
/// \param l The luminence map to calculate for.
/// \param dx The x component of the gradiant output.
/// \param dy The y component of the gradiant output.
/// \param walks Number of walks to do for each pixel. The resuling gradiant is an average of the walks.
/// \param length Length of each walk. The gradiant for a walk is the vector from the pixel to the end.
/// \param exp The next pixel in a walk is selected from its 4 neighbours, weighted by brightness to the power of this value.
/// \param extend If false it considers pixels outside the image to be of value 0, and never selected, if true then the pixels are extended and it can walk outside the image.
/// \param prog Optional progress bar.
EOS_FUNC void GradWalk(const svt::Field<real32> & l,svt::Field<real32> & dx,svt::Field<real32> & dy,
                       nat32 walks,nat32 length,real32 exp,bit extend,
                       time::Progress * prog = null<time::Progress*>());

/// This impliments the same algorithm as GradWalk. The difference is that it 
/// takes the limit as walks goes to infinity, meaning it doesn't suffer from
/// noise. extend is also not provided to this version, it is always considered
/// true.
/// Runtime of this algorithm is O(pixel_count * length * length), making it 
/// theoretically faster as well as better. Inner loop is a lot slower however,
/// so that depends on how many walks you compare it to.
/// Has an extra parameter, stopChance, which is the chance of each walk
/// stopping after each step. Having this set to something other than 0.0 will
/// aleviate any horizon affects of the hard walk length.
EOS_FUNC void GradWalkPerfect(const svt::Field<real32> & l,svt::Field<real32> & dx,svt::Field<real32> & dy,
                              nat32 length,real32 exp,real32 stopChance,
                              time::Progress * prog = null<time::Progress*>());

//------------------------------------------------------------------------------
/// Object version of the GradWalkPerfect function for when you only want it for
/// select pixels in an image - you give it the image and then query individual
/// pixels to get an answer.
class EOS_CLASS GradWalkSelect
{
 public:
  /// &nbsp;
   GradWalkSelect();
   
  /// &nbsp;
   ~GradWalkSelect();
   
  
  /// Sets the input.
   void SetInput(const svt::Field<real32> & l);
   
  /// Sets the parameters.
   void SetParas(nat32 length = 8,real32 exp = 6.0);

  /// Given a coordinate outputs a gradiant direction, that points towards the light.
   void Query(nat32 x,nat32 y,real32 & dx,real32 & dy); 


  /// &nbsp;
   cstrconst TypeString() const;


 private:  
  // Input...
   svt::Field<real32> l;
  
  // Parameters...
   nat32 length;
   real32 exp;
   
  // Runtime...
   ds::Array2D<real32> bufA; // Buffers to store propagation values in - this class
   ds::Array2D<real32> bufB; // exists to avoid there repeated creation/destruction.  
};

//------------------------------------------------------------------------------
 };
};
#endif
