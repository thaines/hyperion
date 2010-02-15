#ifndef EOS_STEREO_SAD_H
#define EOS_STEREO_SAD_H
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


/// \file sad.h
/// Provides an implimentation of Sum of Absolute Differences.

/// \namespace eos::stereo
/// Contains a collection of stereo and closely related algorithms.

#include "eos/types.h"
#include "eos/mem/alloc.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/time/progress.h"
#include "eos/ds/arrays.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This class is given two sets of 2D fields, with identical heights, and 
/// minimum and maximum disparity values. It then calculates a volume, 
/// width x height x disparity range and writes into that volume sums of 
/// absolute values as appropriate. It is incrimental in nature, so no advantage
/// is to be gained by masks of values to not bother with etc.
/// Implimented as a use once class, where you call methods to give it the 
/// relevent values, then Run() to produce the result. Due to the large size of
/// the output of this class you provide it with a field for it to put its 
/// output into.
class EOS_CLASS Sad
{
 public:
  /// &nbsp;
   Sad();

  /// &nbsp;
   ~Sad();


  /// This adds a pair of 2D fields to the calculation, an entry for each
  /// channel. This must be called at least once before Run.
  /// All fields must be the same size, but between the left and right 
  /// set the width can be different.
   void AddField(const svt::Field<real32> & left,const svt::Field<real32> & right);

  /// Sets the radius of the window, the actual window being radius*2 + 1 
  /// in size, defaults to 1 if this method is not called.
   void SetRadius(nat32 radius);

  /// Sets the range of disparity values to search through. They default to
  /// [-30..30] if this method is not called. (Hence a volume of depth 61)
   void SetRange(int32 minDisp,int32 maxDisp);

  /// Returns the volume depth required for the currently set range.
   nat32 Depth();

  /// This sets the absolute difference used whenever the value is out of
  /// bounds, will ushally be set to a large value, defaults to 1e2.
   void SetMaxDiff(real32 maxDiff);


  /// Sets the output volume into which the results will be written, must be 3D,
  /// with the same sizes as the left fields for the first 2 dimensions and Depth()
  /// as the size for the final dimension. You call this before Run(), which then
  /// writes the results directly into the suplied structure, to save on memory.
  /// We only output the left image, as its easy enough to get the right image
  /// from it.
   void SetOutput(svt::Field<real32> & out);


  /// This takes the given inputs and calculates and stores the output, provides a
  /// progress bar capability as it can take some time.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::Sad";}


 private:
  nat32 radius;
  int32 minDisp;
  int32 maxDisp;
  real32 maxDiff;

  ds::Array< Pair< svt::Field<real32>, svt::Field<real32> > > in;
  svt::Field<real32> out;

  // Internal stuff...
   // Returns the absolute difference for the given coordinate, handles out of range values.
   // (For x and y only, not d)
    real32 AD(int32 x,int32 y,int32 d)
    {
     if ((y<0)||(y>=int32(in[0].first.Size(1)))) return maxDiff;
     if ((x<0)||(x>=int32(in[0].first.Size(0)))) return maxDiff;
     int32 x2 = x+d;
     if ((x2<0)||(x2>=int32(in[0].second.Size(0)))) return maxDiff;
     
     real32 ret = 0.0;
      for (nat32 i=0;i<in.Size();i++)
      {
       ret += math::Abs(in[i].first.Get(x,y) - in[i].second.Get(x2,y));
      }
     return ret;
    }
};

//------------------------------------------------------------------------------
 };
};
#endif
