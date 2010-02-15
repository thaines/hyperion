#ifndef EOS_FIT_DISP_FISH_H
#define EOS_FIT_DISP_FISH_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \namespace eos::math
/// Provides tools for 'fitting' to data sets.

/// \file disp_fish.h
/// Provides an algorithm for fitting Fisher distributions to disparity maps, 
/// given the DSC.

#include "eos/types.h"
#include "eos/time/progress.h"
#include "eos/bs/geo3d.h"
#include "eos/cam/files.h"
#include "eos/ds/arrays2d.h"
#include "eos/svt/field.h"
#include "eos/stereo/dsi.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
/// A standard algorithm class that is given a disparity map and a DSC as well 
/// as parameters - it will then fit a Fisher distribution to each pixel which 
/// will be output as a vector indicating both direction and concentration.
/// This is not a fast algorithm.
class EOS_CLASS DispFish
{
 public:
  /// &nbsp;
   DispFish();
   
  /// &nbsp;
   ~DispFish();


  /// Sets the disparity map and DSC to be used. Must be called before run.
   void Set(const svt::Field<real32> & disp,const stereo::DSC & dsc);

  /// Sets the parameters to convert to depth - a cam::CameraPair
   void SetPair(const cam::CameraPair & pair);

  /// Sets the search range for calculating concentration - this is the slow bit.
  /// Note that the scaling is nuts here, requiring the handling of (range*2+1)^3 values,
  /// i.e. the algorithm is O(width*height*(range*2+1)^3) - a very big number.
  /// Default is 3
   void SetRange(nat32 range);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());
   
   
  /// Extracts the results.
   void Get(svt::Field<bs::Vert> & fish);
   
  /// Extracts a result.
   bs::Vert & GetFish(nat32 x,nat32 y);


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::fit::DispFish";}


 private:
  // Input...
   svt::Field<real32> disp;
   const stereo::DSC * dsc;
   cam::CameraPair pair;
   nat32 range;
   
  // Output...
   ds::Array2D<bs::Vert> out;
   
  // Runtime...
   // Structure for storing information for each disparity considered for each pixel...
    struct Pixel
    {
     bs::Vert pos;
     real32 cost;
     real32 weight;
     
     static inline cstrconst TypeString() {return "eos::fit::DispFish::Pixel";}
    };
    
   // Structure for storing a r-contribution and weight, used during L-estimate
   // of concentration...
    struct Rcont
    {
     real32 r;
     real32 weight;
     
     bit operator < (const Rcont & rhs) const {return this->r<rhs.r;}
    };
};

//------------------------------------------------------------------------------
 };
};
#endif
