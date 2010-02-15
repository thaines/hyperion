#ifndef EOS_FIT_DISP_NORM_FISH_H
#define EOS_FIT_DISP_NORM_FISH_H
//------------------------------------------------------------------------------
// Copyright 2009 Tom Haines

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


/// \file disp_norm_fish.h
/// Provides the ability to fit a Fisher distribution to a needle map that has
/// been augmented with Gaussian distributions.

#include "eos/types.h"
#include "eos/time/progress.h"
#include "eos/svt/field.h"
#include "eos/ds/arrays2d.h"
#include "eos/bs/geo3d.h"
#include "eos/cam/files.h"
#include "eos/math/stats_dir.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
class EOS_CLASS DispNormFish
{
 public:
  /// &nbsp;
   DispNormFish();
  
  /// &nbsp;
   ~DispNormFish();


  /// Sets the disparity map and associated standard deviations - must be called
  /// before Run(...).
   void Set(const svt::Field<real32> & disp,const svt::Field<real32> & sd);

  /// Sets the mask, optional - will set masked areas to a concentration of 0.
   void SetMask(const svt::Field<bit> & mask);

  /// Sets the parameters to convert to depth - a cam::CameraPair
   void SetPair(const cam::CameraPair & pair);

  /// Sets the probability weight to match - trys to make the region of the input
  /// distribution of the given size have the same angular range as the same
  /// region of the output Fisher distribution.
  /// Also sets the sd multiplier - simply applied to the standard deviation to
  /// tweak the outputs confidence.
  /// Defaults to 0.1 and 1.0
   void SetRegion(real32 prob,real32 mult = 1.0);
   
  /// Sets the range of concentration parameters it will consider -
  /// defaults to 0.1 and 10.0
   void SetRange(real32 minK,real32 maxK);
   
  
  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the results.
   void Get(svt::Field<bs::Vert> & fish);

  /// Extracts the results.
   void Get(svt::Field<math::Fisher> & fish);


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::fit::DispNormFish";}


 private:
  // Input & parameter...
   svt::Field<real32> disp;
   svt::Field<real32> sd;
   svt::Field<bit> mask;
   cam::CameraPair pair;
   real32 prob;
   real32 mult;
   real32 minK;
   real32 maxK;
    
  // Output...
   ds::Array2D<bs::Vert> out;
};

//------------------------------------------------------------------------------
 };
};
#endif
