#ifndef EOS_FIT_DISP_NORM_H
#define EOS_FIT_DISP_NORM_H
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


/// \file disp_norm.h
/// Provides an algorithm for fitting normal distributions to disparity maps,
/// specifically setting the variance as the mean is given.

#include "eos/types.h"
#include "eos/time/progress.h"
#include "eos/ds/arrays2d.h"
#include "eos/svt/field.h"
#include "eos/stereo/dsi.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
/// Standard algorithm class for fitting Normal distributions to a disparity map,
/// the disparity map is taken to be the mean however, so this is only actually
/// calculating standard deviations.
/// Fits using a bisquare m-estimator, using iterative reweighting.
/// Reasonably fast, and you can get away with setting the maximum number 
/// of iterations quite low.
class EOS_CLASS DispNorm
{
 public:
  /// &nbsp;
   DispNorm();
   
  /// &nbsp;
   ~DispNorm();


  /// Sets the disparity map and DSC to be used. Must be called before run.
  /// Optional set a multiplier for the dsc costs, to emphasis differences.
   void Set(const svt::Field<real32> & disp,const stereo::DSC & dsc,real32 dscMult = 1.0);

  /// Sets the mask, optional - will set masked areas to a standard deviation of 0.
   void SetMask(const svt::Field<bit> & mask);
   
  /// Sets the disparity search range, algorithm is linear in this so setting high is not that bad.
  /// Defaults to 20. (20*2+1=41).
  /// Also sets the sdCount - this is the number of standard deviations from the
  /// current variance estimate the bisquare k value is set to. Defaults to 2.
   void SetRange(nat32 range,real32 sdCount);
   
  /// Sets the acceptable k range for the bisquare function.
  /// Defaults to 2.5 to 10.0
   void SetClampK(real32 minK,real32 maxK);

  /// Sets the clamping range for the standard deviation, defaults to 0.1 to 10.0.
  /// (min should never reach 0)
   void SetClamp(real32 minSd,real32 maxSd);
   
  /// Sets the maximum number of iterations. Defaults to 1000.
   void SetMaxIters(nat32 maxIters);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the results.
   void Get(svt::Field<real32> & sd);
   
  /// Extracts a result.
   real32 GetSd(nat32 x,nat32 y);


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::fit::DispNorm";}


 private:
  // Input...
   svt::Field<real32> disp;
   svt::Field<bit> mask;
   const stereo::DSC * dsc;
   real32 dscMult;
   nat32 range;
   real32 sdCount;
   real32 minK;
   real32 maxK;
   real32 minSd;
   real32 maxSd;
   nat32 maxIters;
   
  // Output...
   ds::Array2D<real32> out;
};

//------------------------------------------------------------------------------
/// Alternate version of DispNorm - uses Laplace's method/approximation of
/// fitting a Gaussian to a distribution. Takes the disparity values for each
/// pixel, applies a Gaussian blur and then uses central differences to
/// calculate the second differential, from which standard deviation is
/// calculated.
class EOS_CLASS LaplaceDispNorm
{
 public:
  /// &nbsp;
   LaplaceDispNorm();
   
  /// &nbsp;
   ~LaplaceDispNorm();


  /// Sets the disparity map and DSC to be used. Must be called before run.
  /// Optional set a multiplier for the dsc costs, to emphasis differences.
   void Set(const svt::Field<real32> & disp,const stereo::DSC & dsc,real32 dscMult = 1.0);

  /// Sets the mask, optional - will set masked areas to a standard deviation of 0.
   void SetMask(const svt::Field<bit> & mask);
   
  /// Sets the strength of the blur to apply, the range of sd values to clamp
  /// the output to and the multiplier of the bluring sd to get the sampling
  /// range.
  /// Defaults are 7 for blur and as given for the rest.
   void SetParam(real32 sd,real32 minSd = 0.1,real32 maxSd = 16.0,real32 sdMult = 3.0);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the results.
   void Get(svt::Field<real32> & sd);
   
  /// Extracts a result.
   real32 GetSd(nat32 x,nat32 y);


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::fit::LaplaceDispNorm";}


 private:
  // Input...
   svt::Field<real32> disp;
   svt::Field<bit> mask;
   const stereo::DSC * dsc;
   real32 dscMult;
   
   real32 sd; // SD of gaussian blur to apply.
   real32 sdMult; // Multiplier of blur sd to get range of values to calculate.
   real32 minSd; // Caps on output.
   real32 maxSd; // "

  // Output...
   ds::Array2D<real32> out;
};

//------------------------------------------------------------------------------
 };
};
#endif
