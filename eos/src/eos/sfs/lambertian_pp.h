#ifndef EOS_SFS_LAMBERTIAN_PP_H
#define EOS_SFS_LAMBERTIAN_PP_H
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


/// \file lambertian_pp.h
/// Fits a lambertian model to every pixel in an image given a surface
/// orientation map and irradiance.

#include "eos/types.h"

#include "eos/time/progress.h"
#include "eos/ds/arrays2d.h"
#include "eos/bs/geo3d.h"
#include "eos/svt/field.h"
#include "eos/data/randoms.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
/// Fits a lambertian model to every pixel in an irradiance map. Uses RANSAC in
/// a window arround each pixel, to maximise the models compatability with its
/// neighbours. The window is weighted with a Gaussian. Propagates models
/// between pixels post-RANSAC, to improve chance of a good fit in all cases.
/// The result of this process should be good for segmentation.
class EOS_CLASS LambertianPP
{
 public:
  /// &nbsp;
   LambertianPP(); 
   
  /// &nbsp;
   ~LambertianPP();


  /// Sets the irradiance map.
   void Set(const svt::Field<real32> & irr);

  /// Sets the surface orientation map.
   void Set(const svt::Field<bs::Normal> & orient);
   
  /// Sets the window sampling parameters.
  /// \param radius Window size is (radius*2+1) in each dimension. Defaults to 5.
  /// \param radSD The number of standard deviations out the radius is, defaults to 2.0
   void SetWin(nat32 radius,real32 radSD);

  /// Sets the RANSAC parameters.
  /// \param prob Probability of success. Defaults to 0.99.
  /// \param cutoff Cutoff in irradiance difference for a pixel being considered an outlier. Defaults to 2.0.
  /// \param maxSamples Maximum number of samples before it gives up. Defaults to 1000.
   void SetSel(real32 prob,real32 cutoff,nat32 maxSamples);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());
   
   
  /// Extracts the model assigned to each pixel, as a normal such that the 
  /// normal dot the surface orientation will precisly get the irradiance.
  /// (That should also be the case for as many neighbours as possible.)
   void Get(nat32 x,nat32 y,bs::Normal &  out);
   
  /// Extracts the percentage, factoring in the weighting, of pixels that are
  /// inliers for the model assigned to a given pixel. [0..1].
  /// Can be used to weight the model assigned to a pixel.
   real32 Weight(nat32 x,nat32 y);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::sfs::LambertianPP";}


 private:
  // Input...
   svt::Field<real32> irr;
   svt::Field<bs::Normal> orient;

   int32 radius;
   real32 radSD;

   real32 prob;
   real32 cutoff;
   nat32 maxSamples;

  // Output...
   struct Model
   {
    bs::Normal model;
    real32 weight;
   };
   ds::Array2D<Model> res;


  // Blah...
   data::Random rand;

   
  // Suport structures...
   // Used to select items from the window with the correct probability...
    struct Sel
    {
     real32 weight;
     real32 culum; // So far, excluding this one.
     int32 u;
     int32 v;
    
     bit operator < (const Sel & rhs) const {return culum < rhs.culum;}
    };
   
  // Suport methods...
   // Calculates the sum of the weights for a given model at a given pixel.
   // Have to provide the weight mask.
    real32 Weight(const ds::Array2D<real32> & mask,int32 x,int32 y,const bs::Normal & model) const;
};

//------------------------------------------------------------------------------
 };
};
#endif
