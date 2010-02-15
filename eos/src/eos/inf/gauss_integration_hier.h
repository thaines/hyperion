#ifndef EOS_INF_GAUSS_INTEGRATION_HIER_H
#define EOS_INF_GAUSS_INTEGRATION_HIER_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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


/// \file gauss_integration_hier.h
/// Provides a tool for integrating 2D grids of differences where (some) value
/// information is also avaliable and all values suffer from noise.
/// A funky smoothing algorithm. A hierachical version - faster convergance.

#include "eos/types.h"
#include "eos/time/progress.h"
#include "eos/math/gaussian_mix.h"
#include "eos/ds/arrays2d.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
/// This uses belief propagation with Gaussians to 'integrate' 2D grids of
/// numbers.
/// You provide an expected value for each grid entry and a relationship
/// from each entry to all other entrys in its 4-neighbourhood.
/// All values are optional, though obviously you need a certain amount of info
/// to get something reasonable out. You can also fix expected values in the
/// grid rather than having an expectation on them.
/// The relationship going from node aa to node bb is of the form aa*m + z = bb,
/// with a suplied standard deviation; you provide m and z as well as the
/// standard deviation.
/// In general the relationships in both directions are provided, they do not
/// need to match, it will automatically produce some sort of 'average'.
/// Expected values are given as normal distributions, as mean and standard
/// deviation.
/// Given all this information values are determined for each entry on the grid,
/// as well as standard deviations to indicate confidence.
/// This is done with belief propagation using a checkerboard update pattern and a
/// given number of message passing iterations.
///
/// This is a hierachical version of the original IntegrateBP - should always be
/// used in preference of.
///
/// NOT IMPLIMENTED - DO NOT USE
class EOS_CLASS HierIntegrateBP
{
 public:
  /// &nbsp;
   HierIntegrateBP(nat32 width = 0,nat32 height = 0);

  /// &nbsp;
   ~HierIntegrateBP();


  /// This resets the grid and sets its size. All values except for the
  /// iteration count return to the default.
   void Reset(nat32 width,nat32 height);

  /// Blah.
   nat32 Width() const;

  /// Blah blah.
   nat32 Height() const;


  /// Sets the expected value for a given coordinate, you provide the mean and
  /// the inverse standard deviation.
  /// The inverse allows you to pass in 0 to indicate infinity, i.e. any
  /// value equally probable.
  /// (The grid defaults to all values equally probable, i.e. sd of infinity.)
   void SetVal(nat32 x,nat32 y,real32 mean,real32 invSd);

  /// This sets a given coordinate to be locked at a particular value,
  /// essentially a standard deviation of 0, which can't be represented with
  /// inverse standard deviation.
   void SetLocked(nat32 x,nat32 y,real32 val);

  /// Sets the expected relationship between two adjacent coordinates.
  /// You provide multiplier(m), offset(z) and inverse standard deviation(invSd).
  /// This represents that exp(dir(x,y)) = exp(x,y)*m + z.
  /// A sd of 0 is not mathematically possible, but infinity to indicate no
  /// information being passed is.
  /// dir can be 0..4, 0 is +ve x, 1 is +ve y, 2 is -ve x, 3 is -ve y.
  /// The default for all values is a standard deviation of infinity,
  /// i.e. no relationship.
   void SetRel(nat32 x,nat32 y,nat32 dir,real32 m,real32 z,real32 invSd);

  /// Sets how many iterations it does per level, defaults to 8, and maximum
  /// number of levels, which defaults to 32.
   void SetIters(nat32 iters,nat32 levels = 32);


  /// Runs the algorithm, reports progress.
  /// Can be called repetedly with you editting stuff between runs.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Returns true if the entry is defined, false if it is undefined.
   bit Defined(nat32 x,nat32 y) const;

  /// Returns the most likelly value for a given pixel.
  /// Do not call for undefined pixels.
   real32 Expectation(nat32 x,nat32 y) const;

  /// Returns the standard deviation for a given pixel.
  /// Do not call for undefined pixels.
   real32 StandardDeviation(nat32 x,nat32 y) const;


  /// Fills the given field with the output of Defined().
   void GetDefined(svt::Field<bit> & out) const;

  /// Fills the given field with the output of Expectation().
   void GetExpectation(svt::Field<real32> & out) const;

  /// Fills the given field with the output of StandardDeviation().
   void GetDeviation(svt::Field<real32> & out) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::inf::HierIntegrateBP";}


 private:
  // Input...
   nat32 maxLevels;
   nat32 iters;

   struct Pixel
   {
    bit lock; // True if locked, in which case ignore the standard deviation of the predicted value.
    real32 mean; // Predicted mean for pixel, unless locked in which case actual value for pixel.
    real32 invSd; // Inverse of predicted standard deviation for pixel if not locked.

    struct Rel
    {
     real32 m; // Multiplicative component.
     real32 z; // Addative component.
     real32 invSd; // If <0 no message is passed, used to indicate boundary conditions etc.
    } rel[4]; // Relationship with neighbouring nodes.
   };
   ds::ArrayDel< ds::Array<Pixel> > in; // [y][x]. Declared like this so it is allocatable for large inputs.


  // Runtime...
   struct Node
   {
    Pixel pix;
    math::Gauss1D exp; // A duplicate of stuff in pix - don't want to construct it each time.
    math::Gauss1D msg[4]; // Comming into the node, so exp*msg[0]*msg[1]*msg[2]*msg[3] is the output.
   };


  // Output...
   ds::Array2D<math::Gauss1D> out;
};

//------------------------------------------------------------------------------
 };
};
#endif
